/*
	停止服务
*/

#include <windows.h>

#pragma comment(lib, "advapi32.lib")


#define SVCNAME L"eventlog"


BOOL __stdcall StopDependentServices(SC_HANDLE schSCManager, SC_HANDLE schService);
BOOL __stdcall DoStopSvc();

BOOL __stdcall DoStopSvc()
{
	BOOL bRet = FALSE;
	SERVICE_STATUS_PROCESS ssp = { 0 };
	DWORD dwStartTime = GetTickCount();
	DWORD dwBytesNeeded = 0;
	DWORD dwTimeout = 30000; // 30-second time-out
	DWORD dwWaitTime = 0;
	SC_HANDLE schSCManager = NULL;
	SC_HANDLE schService = NULL;

	// Get a handle to the SCM database. 

	schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // ServicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (NULL == schSCManager)
	{
		return bRet;
	}

	// Get a handle to the service.

	schService = OpenService(
		schSCManager,			// SCM database 
		SVCNAME,				// name of service 
		SERVICE_ALL_ACCESS);

	if (schService == NULL)
	{
		if (ERROR_SERVICE_DOES_NOT_EXIST == GetLastError())
		{
			bRet = TRUE;
		}
		CloseServiceHandle(schSCManager);
		return bRet;
	}

	// Make sure the service is not already stopped.

	if (!QueryServiceStatusEx(
		schService,
		SC_STATUS_PROCESS_INFO,
		(LPBYTE)&ssp,
		sizeof(SERVICE_STATUS_PROCESS),
		&dwBytesNeeded))
	{
		goto stop_cleanup;
	}

	if (ssp.dwCurrentState == SERVICE_STOPPED)
	{
		bRet = TRUE;
		goto stop_cleanup;
	}

	// If a stop is pending, wait for it.

	while (ssp.dwCurrentState == SERVICE_STOP_PENDING)
	{

		// Do not wait longer than the wait hint. A good interval is 
		// one-tenth of the wait hint but not less than 1 second  
		// and not more than 10 seconds. 

		dwWaitTime = ssp.dwWaitHint / 10;

		if (dwWaitTime < 1000)
			dwWaitTime = 1000;
		else if (dwWaitTime > 10000)
			dwWaitTime = 10000;

		Sleep(dwWaitTime);

		if (!QueryServiceStatusEx(
			schService,
			SC_STATUS_PROCESS_INFO,
			(LPBYTE)&ssp,
			sizeof(SERVICE_STATUS_PROCESS),
			&dwBytesNeeded))
		{
			goto stop_cleanup;
		}

		if (ssp.dwCurrentState == SERVICE_STOPPED)
		{
			bRet = TRUE;
			goto stop_cleanup;
		}

		if (GetTickCount() - dwStartTime > dwTimeout)
		{
			goto stop_cleanup;
		}
	}

	// If the service is running, dependencies must be stopped first.

	StopDependentServices(schSCManager, schService);

	// Send a stop code to the service.

	if (!ControlService(
		schService,
		SERVICE_CONTROL_STOP,
		(LPSERVICE_STATUS)&ssp))
	{
		goto stop_cleanup;
	}

	// Wait for the service to stop.

	while (ssp.dwCurrentState != SERVICE_STOPPED)
	{

		Sleep(1000);

		if (!QueryServiceStatusEx(
			schService,
			SC_STATUS_PROCESS_INFO,
			(LPBYTE)&ssp,
			sizeof(SERVICE_STATUS_PROCESS),
			&dwBytesNeeded))
		{
			goto stop_cleanup;
		}

		if (ssp.dwCurrentState == SERVICE_STOPPED)
		{
			bRet = TRUE;
			break;
		}


		if (GetTickCount() - dwStartTime > dwTimeout)
		{
			goto stop_cleanup;
		}
	}

stop_cleanup:
	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
	return bRet;
}


BOOL __stdcall StopDependentServices(SC_HANDLE schSCManager, SC_HANDLE schService)
{

	DWORD i = 0;
	DWORD dwBytesNeeded = 0;
	DWORD dwCount = 0;

	LPENUM_SERVICE_STATUS   lpDependencies = NULL;
	ENUM_SERVICE_STATUS     ess = { 0 };
	SC_HANDLE               hDepService = NULL;
	SERVICE_STATUS_PROCESS  ssp = { 0 };

	DWORD dwStartTime = GetTickCount();
	DWORD dwTimeout = 30000; // 30-second time-out

	// Pass a zero-length buffer to get the required buffer size.
	if (EnumDependentServices(schService, SERVICE_ACTIVE,
		lpDependencies, 0, &dwBytesNeeded, &dwCount))
	{
		// If the Enum call succeeds, then there are no dependent
		// services, so do nothing.
		return TRUE;
	}
	else
	{
		if (GetLastError() != ERROR_MORE_DATA)
		{
			return FALSE; // Unexpected error
		}


		// Allocate a buffer for the dependencies.
		lpDependencies = (LPENUM_SERVICE_STATUS)HeapAlloc(
			GetProcessHeap(), HEAP_ZERO_MEMORY, dwBytesNeeded);

		if (!lpDependencies)
		{
			return FALSE;
		}


		__try {
			// Enumerate the dependencies.
			if (!EnumDependentServices(schService, SERVICE_ACTIVE,
				lpDependencies, dwBytesNeeded, &dwBytesNeeded,
				&dwCount))
				return FALSE;

			for (i = 0; i < dwCount; i++)
			{
				ess = *(lpDependencies + i);
				// Open the service.
				hDepService = OpenService(schSCManager,
					ess.lpServiceName,
					SERVICE_STOP | SERVICE_QUERY_STATUS);
				if (!hDepService)
				{
					return FALSE;
				}


				__try {
					// Send a stop code.
					if (!ControlService(hDepService,
						SERVICE_CONTROL_STOP,
						(LPSERVICE_STATUS)&ssp))
					{
						return FALSE;
					}


					// Wait for the service to stop.
					while (ssp.dwCurrentState != SERVICE_STOPPED)
					{
						// under windows7, the value of dwWaitHint is 30000ms...when window10 is 0ms
						// so I change it to 1000ms

						Sleep(1000);
						if (!QueryServiceStatusEx(
							hDepService,
							SC_STATUS_PROCESS_INFO,
							(LPBYTE)&ssp,
							sizeof(SERVICE_STATUS_PROCESS),
							&dwBytesNeeded))
						{
							return FALSE;
						}

						if (ssp.dwCurrentState == SERVICE_STOPPED)
						{
							break;
						}


						if (GetTickCount() - dwStartTime > dwTimeout)
						{
							return FALSE;
						}

					}
				}
				__finally
				{
					// Always release the service handle.
					CloseServiceHandle(hDepService);
				}
			}
		}
		__finally
		{
			// Always free the enumeration buffer.
			HeapFree(GetProcessHeap(), 0, lpDependencies);
		}
	}
	return TRUE;
}