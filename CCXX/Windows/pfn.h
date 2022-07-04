#pragma once
#include <windows.h>
#include "ntos.h"

////////////// advapi32.dll ////////////// 

typedef BOOL (WINAPI *pfnOpenProcessToken)(
	_In_ HANDLE ProcessHandle,
	_In_ DWORD DesiredAccess,
	_Outptr_ PHANDLE TokenHandle
);

typedef BOOL (WINAPI *pfnGetTokenInformation)(
	_In_ HANDLE TokenHandle,
	_In_ TOKEN_INFORMATION_CLASS TokenInformationClass,
	_Out_writes_bytes_to_opt_(TokenInformationLength, *ReturnLength) LPVOID TokenInformation,
	_In_ DWORD TokenInformationLength,
	_Out_ PDWORD ReturnLength
);

typedef BOOL (WINAPI *pfnCreateWellKnownSid)(
	_In_ WELL_KNOWN_SID_TYPE WellKnownSidType,
	_In_opt_ PSID DomainSid,
	_Out_writes_bytes_to_opt_(*cbSid, *cbSid) PSID pSid,
	_Inout_ DWORD* cbSid
	);



////////////// kernel32.dll ////////////// 

typedef HANDLE (WINAPI *pfnCreateEventW)(
	_In_opt_ LPSECURITY_ATTRIBUTES lpEventAttributes,
	_In_ BOOL bManualReset,
	_In_ BOOL bInitialState,
	_In_opt_ LPCWSTR lpName
);

typedef DWORD (WINAPI *pfnWaitForSingleObject)(
	_In_ HANDLE hHandle,
	_In_ DWORD dwMilliseconds
);

typedef BOOL (APIENTRY *pfnWaitForDebugEvent)(
	_Out_ LPDEBUG_EVENT lpDebugEvent,
	_In_ DWORD dwMilliseconds
);

typedef BOOL (APIENTRY *pfnContinueDebugEvent)(
	_In_ DWORD dwProcessId,
	_In_ DWORD dwThreadId,
	_In_ DWORD dwContinueStatus
);

typedef BOOL (WINAPI *pfnCreateProcessW)(
	_In_opt_ LPCWSTR lpApplicationName,
	_Inout_opt_ LPWSTR lpCommandLine,
	_In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes,
	_In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,
	_In_ BOOL bInheritHandles,
	_In_ DWORD dwCreationFlags,
	_In_opt_ LPVOID lpEnvironment,
	_In_opt_ LPCWSTR lpCurrentDirectory,
	_In_ LPSTARTUPINFOW lpStartupInfo,
	_Out_ LPPROCESS_INFORMATION lpProcessInformation
);

typedef BOOL(WINAPI *pfnInitializeProcThreadAttributeList)(
	_Out_writes_bytes_to_opt_(*lpSize, *lpSize) LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList,
	_In_ DWORD dwAttributeCount,
	_Reserved_ DWORD dwFlags,
	_When_(lpAttributeList == nullptr, _Out_) _When_(lpAttributeList != nullptr, _Inout_) PSIZE_T lpSize
);

typedef BOOL (WINAPI *pfnUpdateProcThreadAttribute)(
	_Inout_ LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList,
	_In_ DWORD dwFlags,
	_In_ DWORD_PTR Attribute,
	_In_reads_bytes_opt_(cbSize) PVOID lpValue,
	_In_ SIZE_T cbSize,
	_Out_writes_bytes_opt_(cbSize) PVOID lpPreviousValue,
	_In_opt_ PSIZE_T lpReturnSize
);

typedef VOID (WINAPI *pfnDeleteProcThreadAttributeList)(
	_Inout_ LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList
);

typedef DWORD(WINAPI *pfnGetModuleFileNameW)(
	_In_opt_ HMODULE hModule,
	_Out_writes_to_(nSize, ((return < nSize) ? (return +1) : nSize)) LPWSTR lpFilename,
	_In_ DWORD nSize
);



////////////// ntdll.dll ////////////// 



typedef VOID (NTAPI *pfnRtlInitUnicodeString)(
	_Out_ PUNICODE_STRING DestinationString,
	_In_opt_ PCWSTR SourceString);

typedef NTSTATUS (NTAPI *pfnNtOpenKey)(
	_Out_ PHANDLE KeyHandle,
	_In_ ACCESS_MASK DesiredAccess,
	_In_ POBJECT_ATTRIBUTES ObjectAttributes);

typedef NTSTATUS (NTAPI *pfnNtQueryValueKey)(
	_In_ HANDLE KeyHandle,
	_In_ PUNICODE_STRING ValueName,
	_In_ KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
	_Out_writes_bytes_opt_(Length) PVOID KeyValueInformation,
	_In_ ULONG Length,
	_Out_ PULONG ResultLength);

typedef NTSTATUS (NTAPI *pfnNtClose)(
	_In_ _Post_ptr_invalid_ HANDLE Handle);

typedef NTSTATUS (NTAPI *pfnNtQueryInformationProcess)(
	_In_ HANDLE ProcessHandle,
	_In_ PROCESSINFOCLASS ProcessInformationClass,
	_Out_writes_bytes_(ProcessInformationLength) PVOID ProcessInformation,
	_In_ ULONG ProcessInformationLength,
	_Out_opt_ PULONG ReturnLength);

typedef NTSTATUS (NTAPI *pfnNtRemoveProcessDebug)(
	_In_ HANDLE ProcessHandle,
	_In_ HANDLE DebugObjectHandle);

typedef VOID (NTAPI *pfnDbgUiSetThreadDebugObject)(
	_In_ HANDLE DebugObject);

typedef NTSTATUS (NTAPI *pfnNtDuplicateObject)(
	_In_ HANDLE SourceProcessHandle,
	_In_ HANDLE SourceHandle,
	_In_opt_ HANDLE TargetProcessHandle,
	_Out_ PHANDLE TargetHandle,
	_In_ ACCESS_MASK DesiredAccess,
	_In_ ULONG HandleAttributes,
	_In_ ULONG Options);



////////////// rpcrt4.dll ////////////////

typedef RPC_STATUS (RPC_ENTRY *pfnRpcStringBindingComposeW)(
	_In_opt_ RPC_WSTR ObjUuid,
	_In_opt_ RPC_WSTR ProtSeq,
	_In_opt_ RPC_WSTR NetworkAddr,
	_In_opt_ RPC_WSTR Endpoint,
	_In_opt_ RPC_WSTR Options,
	_Outptr_opt_ RPC_WSTR __RPC_FAR * StringBinding
);

typedef RPC_STATUS(RPC_ENTRY *pfnRpcBindingFromStringBindingW)(
	_In_ RPC_WSTR StringBinding,
	_Out_ RPC_BINDING_HANDLE __RPC_FAR * Binding
);

typedef RPC_STATUS(RPC_ENTRY *pfnRpcStringFreeW)(
	_Inout_ RPC_WSTR __RPC_FAR * String
);

typedef RPC_STATUS(RPC_ENTRY *pfnRpcBindingSetAuthInfoExW)(
	_In_ RPC_BINDING_HANDLE Binding,
	_In_opt_ RPC_WSTR ServerPrincName,
	_In_ unsigned long AuthnLevel,
	_In_ unsigned long AuthnSvc,
	_In_opt_ RPC_AUTH_IDENTITY_HANDLE AuthIdentity,
	_In_ unsigned long AuthzSvc,
	_In_opt_ RPC_SECURITY_QOS * SecurityQOS
);

typedef RPC_STATUS(RPC_ENTRY *pfnRpcBindingFree)(
	_Inout_ RPC_BINDING_HANDLE __RPC_FAR * Binding
);

typedef RPC_STATUS(RPC_ENTRY *pfnRpcAsyncInitializeHandle)(
	_Out_writes_bytes_(Size) PRPC_ASYNC_STATE pAsync,
	_In_ unsigned int     Size
);

typedef CLIENT_CALL_RETURN (RPC_VAR_ENTRY *pfnNdrAsyncClientCall)(
	PMIDL_STUB_DESC         pStubDescriptor,
	PFORMAT_STRING          pFormat,
	...
);

typedef void (RPC_ENTRY *pfnRpcRaiseException)(
	_In_ RPC_STATUS exception
);

typedef RPC_STATUS (RPC_ENTRY *pfnRpcAsyncCompleteCall)(
	_Inout_ PRPC_ASYNC_STATE pAsync,
	_Out_opt_ void *Reply
);