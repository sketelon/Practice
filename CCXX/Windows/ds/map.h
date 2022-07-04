#include <windows.h>
#include <map>
#include <intsafe.h>

using namespace std;

template <class _Kt, class _Vt>
class CMyMap
{
public:
	CMyMap() { InitializeCriticalSection(&m_csLock); }
	virtual ~CMyMap() { DeleteCriticalSection(&m_csLock); }

	VOID add_item(_Kt key, _Vt value)
	{
		EnterCriticalSection(&m_csLock);
		m_map[key] = value;
		LeaveCriticalSection(&m_csLock);
		return VOID();
	}

	BOOL delete_item(_Kt key)
	{
		BOOL bRet = FALSE;
		EnterCriticalSection(&m_csLock);
		if (m_map.find(key) != m_map.end()) {
			// delete m_map[key];
			// m_map[key] = NULL;
			m_map.erase(key);
			bRet = TRUE;
		}
		LeaveCriticalSection(&m_csLock);
		return bRet;
	}

	BOOL get_item(_Kt key, _Vt& value)
	{
		BOOL bRet = FALSE;
		EnterCriticalSection(&m_csLock);
		typename map<_Kt, _Vt>::iterator iter = m_map.find(key);
		if (iter != m_map.end()) {
			value = (*iter).second;
			bRet = TRUE;
		}
		LeaveCriticalSection(&m_csLock);

		return bRet;
	}

	// !!!
	WORD generate_key()
	{

		WORD i = 1;
		EnterCriticalSection(&m_csLock);
		for (; i <= USHORT_MAX; i++)
		{
			if (m_map.find(i) == m_map.end())
			{
				break;
			}
		}
		LeaveCriticalSection(&m_csLock);
		return i;
	}



private:
	map<_Kt, _Vt> m_map;
	CRITICAL_SECTION m_csLock;
};