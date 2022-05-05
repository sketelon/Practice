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
	3. 写文件
*/
BOOL read_file(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead)
{
    DWORD NumberOfBytesRead = 0;
    return ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, &NumberOfBytesRead, NULL);
}

/*
	4. 读文件
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
	根据进程名获得进程ID
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
	宽字节与多字节转换
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
