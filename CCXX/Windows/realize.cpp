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