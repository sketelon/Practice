/*
目录：
1. 初始化字符串（申请空间）
2. 模拟 Sleep 函数
3. 创建线程
4. 文件操作，操作完记住调用 ZwClose()
5. 由进程名获得 PID
6. 根据 PID 获取用户名
7. 由进程 id 获得该进程的某模块的基地址
8. 获取驱动所在路径

*/




/*
	1. 初始化字符串（申请空间）
*/
// for warning C4996: 'ExAllocatePool': was declared deprecated
#pragma warning(disable:4996)
#define BUFFER_SIZE 1024
VOID allocate_string1(PUNICODE_STRING pUnicodeString, LPCWSTR lpStrings)
{

	pUnicodeString->Length = (USHORT)(wcslen(lpStrings) * sizeof(WCHAR));
	pUnicodeString->MaximumLength = BUFFER_SIZE;

	ASSERT(pUnicodeString->MaximumLength >= pUnicodeString->Length);

	pUnicodeString->Buffer = ExAllocatePool(PagedPool, BUFFER_SIZE);
	ASSERT(pUnicodeString->Buffer != NULL);

	RtlCopyMemory(pUnicodeString->Buffer, lpStrings, pUnicodeString->Length);
}

VOID allocate_string2(PUNICODE_STRING pDst, PUNICODE_STRING pSrc)
{
	pDst->Length = pSrc->Length;
	pDst->MaximumLength = BUFFER_SIZE;
	ASSERT(pDst->MaximumLength >= pSrc->Length);

	pDst->Buffer = ExAllocatePool(PagedPool, BUFFER_SIZE);
	ASSERT(pDst->Buffer != NULL);

	RtlCopyMemory(pDst->Buffer, pSrc->Buffer, pSrc->Length);
}

VOID free_string(PUNICODE_STRING pUnicodeString)
{
	if (MmIsAddressValid(pUnicodeString->Buffer))
	{
		ExFreePool(pUnicodeString->Buffer);
		pUnicodeString->Buffer = NULL;
		pUnicodeString->Length = pUnicodeString->MaximumLength = 0;
	}

}


/*
	2. 模拟 Sleep 函数
*/
#define DELAY_ONE_MICROSECOND 	(-10)
#define DELAY_ONE_MILLISECOND	(DELAY_ONE_MICROSECOND*1000)
VOID sleep(ULONG uMilliseconds)
{
	LARGE_INTEGER interval;
	interval.QuadPart = DELAY_ONE_MILLISECOND;
	interval.QuadPart *= uMilliseconds;
	KeDelayExecutionThread(KernelMode, FALSE, &interval);
}

/*
	3. 创建线程
*/
NTSTATUS create_thread(PKSTART_ROUTINE pStartRoutine, PVOID pStartContext, PETHREAD *Thread)
{
	NTSTATUS status;
	HANDLE hThreadHandle;
	status = PsCreateSystemThread(
		&hThreadHandle, THREAD_ALL_ACCESS, 
		NULL, NULL, NULL,
		pStartRoutine, pStartContext);
	if (!NT_SUCCESS(status)) {
		return status;
	}

	status = ObReferenceObjectByHandle(
		hThreadHandle, THREAD_ALL_ACCESS, NULL, 
		KernelMode, Thread, NULL);

	ZwClose(hThreadHandle);
	return status;
}

VOID stop_thread(PETHREAD Thread)
{
	KeWaitForSingleObject(Thread, Executive, KernelMode, FALSE, NULL);
	ObDereferenceObject(Thread);
}

/*
	4. 文件操作，操作完记住调用 ZwClose()
*/

NTSTATUS create_file(PHANDLE pFileHandle, PUNICODE_STRING pObjectName)
{
	OBJECT_ATTRIBUTES		objectAttributes;
	IO_STATUS_BLOCK			iostatus = { 0 };

	InitializeObjectAttributes(&objectAttributes,
		pObjectName,
		OBJ_CASE_INSENSITIVE,
		NULL, NULL);

	return ZwCreateFile(pFileHandle,
						FILE_GENERIC_READ | FILE_GENERIC_WRITE,  // FILE_GENERIC_WRITE
						&objectAttributes, &iostatus, 
						0, FILE_ATTRIBUTE_NORMAL, // FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_TEMPORARY
						0,	// FILE_SHARE_READ
						FILE_OPEN_IF, // FILE_CREATE | FILE_OPEN
						FILE_SYNCHRONOUS_IO_NONALERT,
						NULL,0);
}

NTSTATUS read_file(HANDLE FileHandle, PVOID pBuffer, ULONG uLength, PLARGE_INTEGER pByteOffset)
{
	IO_STATUS_BLOCK			iostatus = { 0 };
	return ZwReadFile(FileHandle,
		NULL, NULL, NULL, &iostatus, pBuffer, uLength, pByteOffset, 0);
}

NTSTATUS write_file(HANDLE FileHandle, PVOID pBuffer, ULONG uLength, PLARGE_INTEGER pByteOffset)
{
	IO_STATUS_BLOCK			iostatus = { 0 };
	return ZwWriteFile(FileHandle,
		NULL, NULL, NULL, &iostatus, pBuffer, uLength, pByteOffset, 0);
}

/*
	5. 由进程名获得 PID
*/
typedef struct _SYSTEM_THREADS {
	LARGE_INTEGER  KernelTime;
	LARGE_INTEGER  UserTime;
	LARGE_INTEGER  CreateTime;
	ULONG          WaitTime;
	PVOID          StartAddress;
	CLIENT_ID      ClientId;
	KPRIORITY      Priority;
	KPRIORITY      BasePriority;
	ULONG          ContextSwitchCount;
	LONG           State;
	LONG           WaitReason;
} SYSTEM_THREADS, * PSYSTEM_THREADS;

typedef struct _SYSTEM_PROCESSES {
	ULONG            NextEntryDelta;
	ULONG            ThreadCount;
	ULONG            Reserved1[6];
	LARGE_INTEGER    CreateTime;
	LARGE_INTEGER    UserTime;
	LARGE_INTEGER    KernelTime;
	UNICODE_STRING   ProcessName;
	KPRIORITY        BasePriority;
	HANDLE           ProcessId;
	HANDLE           InheritedFromProcessId;
	ULONG            HandleCount;
	ULONG            Reserved2[2];
	VM_COUNTERS      VmCounters;
	IO_COUNTERS      IoCounters;
	SYSTEM_THREADS   Threads[1];
} SYSTEM_PROCESSES, * PSYSTEM_PROCESSES;

NTSTATUS NTAPI ZwQuerySystemInformation(ULONG SystemInformationClass,
	PVOID SystemInformation,
	ULONG SystemInformationLength,
	PULONG ReturnLength);

#define POOL_TAG 'enoN'
#define SystemProcessInformation 5
#include <wdf.h>
NTSTATUS get_pid_by_processname(PHANDLE pPid, PUNICODE_STRING pProcess)
{

	NTSTATUS status = STATUS_UNSUCCESSFUL;
	ULONG uBufferSize = 0;
	PVOID pBuffer = NULL;

	if (KeGetCurrentIrql() != PASSIVE_LEVEL) {
		return status;
	}

	if (ZwQuerySystemInformation(SystemProcessInformation, NULL, 0, &uBufferSize) != STATUS_INFO_LENGTH_MISMATCH) {
		return;
	}

	pBuffer = ExAllocatePoolWithTag(PagedPool, uBufferSize, POOL_TAG);
	if (pBuffer == NULL) {
		return;
	}

	status = ZwQuerySystemInformation(SystemProcessInformation, pBuffer, uBufferSize, &uBufferSize);
	if (!NT_SUCCESS(status)) {
		goto _EXIT;
	}

	PSYSTEM_PROCESSES processEntry = pBuffer;

	do {

		if (RtlCompareUnicodeString(&processEntry->ProcessName, pProcess, TRUE) == 0)
		{
			*pPid = processEntry->ProcessId;
			status = STATUS_SUCCESS;
			break;
		}
		processEntry = (PSYSTEM_PROCESSES)((BYTE*)processEntry + processEntry->NextEntryDelta);
	} while (processEntry->NextEntryDelta);


_EXIT:
	if (pBuffer != NULL) {
		ExFreePoolWithTag(pBuffer, POOL_TAG);
		pBuffer = NULL;
	}

	return status;
}

/*
	6. 根据 PID 获取用户名
*/

NTSTATUS get_username_by_pid(HANDLE hProcessId, PUNICODE_STRING pUserName)
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    PEPROCESS pProcess = NULL;
    PACCESS_TOKEN pToken = NULL;
    LUID luid;
    PSecurityUserData userInformation = NULL;

    if (KeGetCurrentIrql() != PASSIVE_LEVEL) {
        return status;
    }

    status = PsLookupProcessByProcessId(hProcessId, &pProcess);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    pToken = PsReferencePrimaryToken(pProcess);

    status = SeQueryAuthenticationIdToken(pToken, &luid);
    if (!NT_SUCCESS(status)) {
        goto _EXIT;
    }

    status = GetSecurityUserInfo(&luid, UNDERSTANDS_LONG_NAMES, &userInformation);
    if (!NT_SUCCESS(status)) {
        goto _EXIT;
    }
    
    RtlCopyUnicodeString(pUserName, &userInformation->UserName);


_EXIT:
    if (pProcess != NULL) {
        ObDereferenceObject(pProcess);
    }
    if (pToken != NULL) {
        ObDereferenceObject(pToken);
    }
    if (userInformation) {
        LsaFreeReturnBuffer(userInformation);
    }
    return status;
}


/*
	7. 由进程 id 获得该进程的某模块的基地址
*/
VOID* get_module_base_by_process_id(HANDLE ProcessId, PUNICODE_STRING pModuleName)
{
    PMYEPROCESS pEprocess = (PMYEPROCESS)PsGetCurrentProcess();
    PMYEPROCESS pEprocessFirst = pEprocess;
    VOID* BaseAddr = NULL;

    do {
        pEprocess = CONTAINING_RECORD(pEprocess->ActiveProcessLinks.Flink, MYEPROCESS, ActiveProcessLinks);

        if (!MmIsAddressValid(pEprocess))
        {
            continue;
        }

        //PsIsProtectedProcess(eprocess);

        if (pEprocess->UniqueProcessId == ProcessId)
        {

            KAPC_STATE  ks;

            KeStackAttachProcess(pEprocess, &ks);
            PMYPEB pPeb = pEprocess->Peb;
            struct _PEB_LDR_DATA* pPebLdrData = pPeb->Ldr;
            PLIST_ENTRY pListEntryNode = &pPebLdrData->InMemoryOrderModuleList;

            for (PLIST_ENTRY pIterNode = pListEntryNode->Flink;
                pIterNode != pListEntryNode;
                pIterNode = pIterNode->Flink)
            {


                PLDR_DATA_TABLE_ENTRY pLdrDataEntry = (PLDR_DATA_TABLE_ENTRY)CONTAINING_RECORD(pIterNode, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);

                if (RtlEqualUnicodeString(pModuleName, &(pLdrDataEntry->BaseDllName), TRUE))
                {
                    KdPrint((L"DllName = %wZ, BaseAddr = 0x%x", pLdrDataEntry->BaseDllName, pLdrDataEntry->DllBase));
                    BaseAddr = pLdrDataEntry->DllBase;
                    break;
                }

            }
            KeUnstackDetachProcess(&ks);
            break;
        }

    } while (pEprocess != pEprocessFirst);

    return BaseAddr;
}


/*
	8. 获取驱动所在路径
*/

typedef union
{
	WCHAR Name[sizeof(ULARGE_INTEGER) / sizeof(WCHAR)];
	ULARGE_INTEGER Alignment;
} ALIGNEDNAME;

//
// DOS Device Prefix \??\
//

ALIGNEDNAME ObpDosDevicesShortNamePrefix = { { L'\\', L'?', L'?', L'\\' } };
UNICODE_STRING ObpDosDevicesShortName = {
  sizeof(ObpDosDevicesShortNamePrefix), // Length
  sizeof(ObpDosDevicesShortNamePrefix), // MaximumLength
  (PWSTR)&ObpDosDevicesShortNamePrefix  // Buffer
};

NTSTATUS get_directory_path_of_driver(_In_ PUNICODE_STRING pRegistryPath, 
									  _Inout_ PUNICODE_STRING pDirectoryPath)
{
	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING ValueName = RTL_CONSTANT_STRING(L"ImagePath");
	OBJECT_ATTRIBUTES ObjectAttributes;
	InitializeObjectAttributes(&ObjectAttributes,
		pRegistryPath,
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
		NULL,
		NULL);

	HANDLE KeyHandle;
	status = ZwOpenKey(&KeyHandle,
		KEY_READ,
		&ObjectAttributes);
	if (!NT_SUCCESS(status)) {
		return status;
	}

	UCHAR KeyValueInformationBuffer[sizeof(KEY_VALUE_FULL_INFORMATION) + sizeof(WCHAR) * 128];
	PKEY_VALUE_FULL_INFORMATION KeyValueInformation = (PKEY_VALUE_FULL_INFORMATION)KeyValueInformationBuffer;

	ULONG ResultLength;
	status = ZwQueryValueKey(KeyHandle,
		&ValueName,
		KeyValueFullInformation,
		KeyValueInformation,
		sizeof(KeyValueInformationBuffer),
		&ResultLength);

	ZwClose(KeyHandle);

	if (!NT_SUCCESS(status) ||
		KeyValueInformation->Type != REG_EXPAND_SZ ||
		KeyValueInformation->DataLength < sizeof(ObpDosDevicesShortNamePrefix))
	{
		return status;
	}

	PWCHAR ImagePathValue = (PWCHAR)((PUCHAR)KeyValueInformation + KeyValueInformation->DataOffset);
	ULONG  ImagePathValueLength = KeyValueInformation->DataLength;

	if (*(PULONGLONG)(ImagePathValue) == ObpDosDevicesShortNamePrefix.Alignment.QuadPart)
	{
		ImagePathValue += ObpDosDevicesShortName.Length / sizeof(WCHAR);
		ImagePathValueLength -= ObpDosDevicesShortName.Length;
	}

	PWCHAR LastBackslash = wcsrchr(ImagePathValue, L'\\');

	if (!LastBackslash)
	{
		return STATUS_DATA_ERROR;
	}

	*LastBackslash = UNICODE_NULL;

	RtlInitUnicodeString(pDirectoryPath, ImagePathValue);

	return status;

}