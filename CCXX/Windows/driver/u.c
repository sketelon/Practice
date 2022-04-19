

/*
	初始化字符串（申请空间）
*/
// for warning C4996: 'ExAllocatePool': was declared deprecated
#pragma warning(disable:4996)
#define BUFFER_SIZE 1024
VOID allocate_string(PUNICODE_STRING pUnicodeString, LPCWSTR lpStrings)
{

	pUnicodeString->Length = (USHORT)(wcslen(lpStrings) * sizeof(WCHAR));
	pUnicodeString->MaximumLength = BUFFER_SIZE;

	ASSERT(pUnicodeString->MaximumLength >= pUnicodeString->Length);

	pUnicodeString->Buffer = ExAllocatePool(PagedPool, BUFFER_SIZE);
	RtlCopyMemory(pUnicodeString->Buffer, lpStrings, pUnicodeString->Length);
}

VOID free_string(PUNICODE_STRING pUnicodeString)
{
	ExFreePool(pUnicodeString->Buffer);
	pUnicodeString->Buffer = NULL;
	pUnicodeString->Length = pUnicodeString->MaximumLength = 0;
}


/*
	模拟 Sleep 函数
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
	创建线程
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
	文件操作，操作完记住调用 ZwClose()
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
	由进程 id 获得该进程的某模块的基地址
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