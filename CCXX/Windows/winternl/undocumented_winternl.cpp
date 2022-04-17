#include <windows.h>



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