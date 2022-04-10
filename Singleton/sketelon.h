#ifndef SKETELON_H_
#define SKETELON_H_


#include <windows.h>
#include <strsafe.h>
#include <tchar.h>

#define PLATFORM_WINDOWS



#ifdef PLATFORM_WINDOWS

void ErrorMsg(LPTSTR lpszFunction, DWORD dwMode);


#define ERROR_BOX(fun) ErrorMsg(fun, 0);
#define ERROR_CMD(fun) ErrorMsg(fun, 1);

enum MSG_MODE
{
    MsgBox = 0,
    CmdLine
};

void ErrorMsg(LPTSTR lpszFunction, DWORD dwMode)
{
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0, NULL);

    // Display the error message and exit the process

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
    StringCchPrintf((LPTSTR)lpDisplayBuf,
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("<%s> failed with error %d: %s"),
        lpszFunction, dw, lpMsgBuf);

    switch (dwMode)
    {
    case MsgBox:
        MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);
        break;
    case CmdLine:
        _tprintf(TEXT("[-] %s \r\n"), (LPCTSTR)lpDisplayBuf);
        break;
    }

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
}

#define CREATE_SINGLE(T) T::Instance()
#define RELEASE_SINGLE(T) T::Instance(true)

#define  SINGLE_INSTANCE(T)\
static T* Instance(bool bRelease = false)\
{\
    static T* m_pApp = NULL;\
    if (!bRelease)\
    {\
        if (m_pApp == NULL)\
        {\
            m_pApp = new T();\
        }\
    }\
    else\
    {\
        if (m_pApp != NULL)\
        {\
            delete m_pApp;\
            m_pApp = NULL;\
        }\
    }\
    return m_pApp;\
}\



#define SINGLETON_ANNOUNCEMENT(T)\
private:\
    T() {};\
    ~T() {};\
    SINGLE_INSTANCE(T)\
public:\
    static T* GetInstance()\
    {\
        return Instance();\
    }\
    static T* ReleaseInstance()\
    {\
        return Instance(true);\
    }\
public:\


#define GET_SINGLETON_CLASS(T)      T::GetInstance()
#define RELEASE_SINGLETON_CLASS(T)  T::ReleaseInstance()
/*******************************************************************************************
--------------------------------------------------------------------------------------------
*******************************************************************************************/

#endif
#endif