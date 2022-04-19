#include <windows.h>


/*
	字符串长度
*/
int string_length(LPCTSTR lpString)
{
	int i = 0;
	while (*lpString != '\0')
	{
		i++;
		lpString++;
	}

	return i;
}

/*
	路径操作
*/
BOOL path_remove_file_spec(LPTSTR pszPath)
{
	BOOL bRet = FALSE;

	SIZE_T nLength = string_length(pszPath);

	do
	{
		nLength--;
		if (0 == nLength)
		{
			return bRet;
		}
	} while (pszPath[nLength] != '\\');

	_MAX_DRIVE - 1 == nLength ? pszPath[nLength + 1] = '\0' : pszPath[nLength] = '\0';


	bRet = TRUE;
	return bRet;
}

/*
	CopyMemory
*/
inline void copy_memory(LPVOID pvDst, LPCVOID pvSrc, SIZE_T cbCopy)
{
	if (pvDst && pvSrc && cbCopy)
	{
		while (cbCopy--)
			((BYTE*)pvDst)[cbCopy] = ((CONST BYTE*)pvSrc)[cbCopy];
	}
}


/*
	遍历模块信息
*/
#include <winternl.h>
DWORD enumerate_module_in_memory()
{
	PPEB pPeb = NULL;
	PPEB_LDR_DATA pPebLdrData = NULL;


#ifdef _WIN64
	pPeb = (PPEB)__readgsqword(0x60);
#else
	pPeb = (PPEB)__readfsdword(0x30);
#endif


	// ref: http://msdn.microsoft.com/en-us/library/aa813708(VS.85).aspx
	// mov eax, dword ptr ds:[eax + 0Ch]
	pPebLdrData = pPeb->Ldr;


	PLIST_ENTRY pListEntryNode = &pPebLdrData->InMemoryOrderModuleList;


	for (PLIST_ENTRY pIterNode = pListEntryNode->Flink;
		pIterNode != pListEntryNode;
		pIterNode = pIterNode->Flink)
	{

		PLDR_DATA_TABLE_ENTRY pLdrDataEntry = (PLDR_DATA_TABLE_ENTRY)CONTAINING_RECORD(pIterNode, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);

		UINT_PTR DllBase = (UINT_PTR)pLdrDataEntry->DllBase;
		UNICODE_STRING pFullDllName = pLdrDataEntry->FullDllName;
	}
}