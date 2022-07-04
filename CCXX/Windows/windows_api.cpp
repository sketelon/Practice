/*
# 目录：
## My Windows API
1. 创建进程
2. 创建文件
3. 写文件
4. 读文件
5. 设备操作
## Windows API Extension
1. 根据进程名获得进程ID
2. 宽字节与多字节转换
3. 获取进程所在用户名
4. 获取系统信息
5. 是否属于管理员权限
*/



//
// My Windows API
//


/*
	1. 创建进程
*/
BOOL create_process(LPCTSTR lpCommandLine, LPPROCESS_INFORMATION lpProcessInformation)
{
	STARTUPINFO si = { 0 };

	TCHAR szCommandLine[MAX_PATH] = { 0 };
	lstrcpy(szCommandLine, lpCommandLine);
	return CreateProcess(NULL, szCommandLine, NULL, NULL, FALSE,
		0, // CREATE_SUSPEND | CREATE_NO_WINDOW | CREATE_NEW_CONSOLE
		NULL, NULL, &si, lpProcessInformation);
}


/*
	2. 创建文件
*/
HANDLE create_file(LPCTSTR lpFileName)
{
	return CreateFile(lpFileName, GENERIC_READ | GENERIC_WRITE,
		0, // FILE_SHARE_READ | FILE_SHARE_WRITE
		NULL,
		OPEN_ALWAYS, // OPEN_EXISTING & CREATE_NEW & CREATE_ALWAYS & TRUNCATE_EXISTING
		FILE_ATTRIBUTE_NORMAL, // FILE_ATTRIBUTE_HIDDEN
		NULL);
}


/*
	4. 读文件
*/
BOOL read_file(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead)
{
    DWORD NumberOfBytesRead = 0;
    return ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, &NumberOfBytesRead, NULL);
}


/*
	3. 写文件
*/
BOOL write_file(HANDLE hFile, LPCVOID lpBuffer, DWORD dwNumberOfBytesToWrite)
{
    // [out, optional] lpNumberOfBytesWritten: Windows 7:  This parameter can not be NULL.
    DWORD NumberOfBytesWritten = 0;
    return WriteFile(hFile, lpBuffer, dwNumberOfBytesToWrite, &NumberOfBytesWritten, NULL);
}

/*
	5. 设备操作
*/
BOOL device_io_control(HANDLE hDevice, 
                       DWORD dwIoControlCode, 
                       LPVOID lpInBuffer,
                       DWORD nInBufferSize,
                       LPVOID lpOutBuffer,
                       DWORD nOutBufferSize)
{
    DWORD BytesReturned = 0;
    // #define IRP_MJ_DEVICE_CONTROL           0x0e
    return DeviceIoControl(hDevice, dwIoControlCode, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize, &BytesReturned, NULL);
}


//
// Windows API Extension
//


/*
	1. 根据进程名获得进程ID
*/

#include <tlhelp32.h>
DWORD get_process_id_by_name(LPTSTR lpProcessName, DWORD &dwPid)
{
	BOOL bRet = ERROR_NOT_FOUND;
	PROCESSENTRY32 pe32 = { 0 };

	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return bRet;
	}

	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32First(hProcessSnap, &pe32))
	{
		goto EXIT;
	}

	do
	{
		if (lstrcmpi(lpProcessName, pe32.szExeFile) == 0)
		{
			dwPid = pe32.th32ProcessID;
			bRet = ERROR_SUCCESS;
			break;
		}

	} while (Process32Next(hProcessSnap, &pe32));

EXIT:
	CloseHandle(hProcessSnap);
	return bRet;
}

/*
	2. 宽字节与多字节转换
*/

#define HEAP_FREE(lpMem) if (lpMem != NULL) {\
							HeapFree(GetProcessHeap(), 0, lpMem);\
							lpMem = NULL;}

BOOL W2A(LPCWCH lpWideCharStr, LPSTR &lpMultiByteStr)
{
	BOOL bRet = FALSE;
	size_t cchLength = 0;

	cchLength = lstrlenW(lpWideCharStr);

	lpMultiByteStr = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (cchLength + 1) * sizeof(CHAR));
	if (NULL == lpMultiByteStr)
	{
		return bRet;
	}

	// if the string is null, allocate one character sapce and return TRUE
	if (0 == cchLength)
	{
		bRet = TRUE;
		return bRet;
	}

	if (0 == WideCharToMultiByte(CP_OEMCP, 0, lpWideCharStr, cchLength,
		lpMultiByteStr, cchLength, NULL, NULL))
	{
		HEAP_FREE(lpMultiByteStr);
		return bRet;
	}
	bRet = TRUE;
	return bRet;
}

BOOL A2W(LPSTR lpMultiByteStr, LPWSTR &lpWideCharStr)
{
	BOOL bRet = FALSE;
	size_t cchLength = 0;

	cchLength = lstrlenA(lpMultiByteStr);

	lpWideCharStr = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (cchLength + 1) * sizeof(WCHAR));
	if (NULL == lpWideCharStr)
	{
		return bRet;
	}

	// if the string is null, allocate one character sapce and return TRUE
	if (0 == cchLength)
	{
		bRet = TRUE;
		return bRet;
	}

	if (0 == MultiByteToWideChar(CP_ACP, 0, lpMultiByteStr, cchLength,
		lpWideCharStr, cchLength))
	{

		HEAP_FREE(lpWideCharStr);
		return bRet;
	}

	bRet = TRUE;
	return bRet;
}


/*
	3. 获取进程所在用户名
*/
DWORD get_process_user_name(LPTSTR pUserName)
{
	DWORD dwResult = EXIT_FAILURE;
	HANDLE hToken = NULL;
	DWORD dwTokenLength = 0;
	TCHAR szAccountName[MAX_PATH] = { 0 };
	TCHAR szDomainName[MAX_PATH] = { 0 };
	PTOKEN_USER pTokenInformation = NULL;
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, GetCurrentProcessId());
	if (NULL == hProcess) {
		goto _EXIT;
	}

	if (!OpenProcessToken(hProcess, TOKEN_ALL_ACCESS, &hToken)){
		goto _EXIT;
	}

	GetTokenInformation(hToken, TokenUser, NULL, 0L, &dwTokenLength);
	if (dwTokenLength > 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
	{
		pTokenInformation = (PTOKEN_USER)::GlobalAlloc(GPTR, dwTokenLength);
		if (GetTokenInformation(hToken, TokenUser, pTokenInformation, dwTokenLength, &dwTokenLength))
		{
			SID_NAME_USE eUse = SidTypeUnknown;
			DWORD dwAccountSize = 0L;
			DWORD dwDomainSize = 0L;

			PSID  pSid = pTokenInformation->User.Sid;

			LookupAccountSid(NULL, pSid, NULL, &dwAccountSize, NULL, &dwDomainSize, &eUse);
			if (dwAccountSize > 0 && dwDomainSize > 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
				if (LookupAccountSid(NULL, pSid, szAccountName, &dwAccountSize, szDomainName, &dwDomainSize, &eUse))
				{
					_tcscpy(pUserName, szAccountName);
					dwResult = EXIT_SUCCESS;
				}
			}
		}
	}

_EXIT:
	if (NULL != hProcess) {
		CloseHandle(hProcess);
		hProcess = NULL;
	}

	if (NULL != pTokenInformation) {
		::GlobalFree(pTokenInformation);
		pTokenInformation = NULL;
	}

	if (NULL != hToken) {
		CloseHandle(hToken);
		hToken = NULL;
	}
	return dwResult;
}




/*
	4. 获取系统信息
*/
DWORD get_system_info(LPOSVERSIONINFOEXW lpOSVersionInfo)
{
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
	typedef LONG(_stdcall *pfnRtlGetVersion)(PRTL_OSVERSIONINFOEXW lpVersionInformation);
	pfnRtlGetVersion pRtlGetVersion;

	DWORD dwResult = EXIT_FAILURE;
	NTSTATUS status;
	char szRtlGetVersion[] = { 'R', 't', 'l', 'G', 'e', 't', 'V', 'e', 'r', 's', 'i', 'o', 'n', 0x0 };
	TCHAR szNtdll[] = { _T('n'), _T('t'), _T('d'), _T('l'), _T('l'), _T('.'), _T('d'), _T('l'), _T('l'), 0x00, 0x00 };

	HMODULE hNtdll = GetModuleHandle(/*_T("ntdll.dll")*/szNtdll);
	if (NULL == hNtdll) {
		return dwResult;
	}

	pRtlGetVersion = (pfnRtlGetVersion)GetProcAddress(hNtdll, szRtlGetVersion);
	if (pRtlGetVersion == NULL) {
		return dwResult;
	}

	lpOSVersionInfo->dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);
	status = pRtlGetVersion(lpOSVersionInfo);
	if (NT_SUCCESS(status)) {
		dwResult = EXIT_SUCCESS;
	}

	return dwResult;
}

/*
	5. 获取系统信息
*/

BOOL CPassUAC::IsAdmin()
{
	BOOL bRet = FALSE;
	HANDLE hProcessToken = NULL;
	TOKEN_ELEVATION tokenElevation = { 0 };
	DWORD dwReturnLength = 0;

	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hProcessToken))
	{
		if (GetTokenInformation(hProcessToken, TokenElevation,
			&tokenElevation, sizeof(tokenElevation), &dwReturnLength))
		{
			if (dwReturnLength == sizeof(tokenElevation) &&
				(tokenElevation.TokenIsElevated)) {
				bRet = TRUE;
			}
		}

		CloseHandle(hProcessToken);

	}
	return bRet;
}