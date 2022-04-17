/*

	重定位表修复

*/

typedef struct _REL_BLOCK
{
	WORD    Offset : 12;
	WORD    Type : 4;
}REL_BLOCK, *PREL_BLOCK;

DWORD relocations(const IMAGE_NT_HEADERS *pNtHeader, PVOID pvImageBase)
{
	DWORD dwStatus = ERROR_INVALID_PARAMETER;

	DWORD dwSize = pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;
	DWORD dwVirtualAddress = pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;
	if (!(dwSize && dwVirtualAddress))
	{
		// no relocations
		dwStatus = ERROR_DATABASE_DOES_NOT_EXIST;
		return dwStatus;
	}

	const IMAGE_BASE_RELOCATION *pBaseRelocation = (const PIMAGE_BASE_RELOCATION)((UINT_PTR)pvImageBase + dwVirtualAddress);

	if (IsBadReadPtr(pBaseRelocation, sizeof(PIMAGE_BASE_RELOCATION)))
	{
		// probably wrong ImageBase
		dwStatus = ERROR_ACCESS_DENIED;
		return dwStatus;
	}

	while (pBaseRelocation->SizeOfBlock)
	{
		const DWORD dwBlockCounts = (pBaseRelocation->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
		if (dwBlockCounts)
		{
			//relocations follow the header
			PREL_BLOCK pBlocks = (PREL_BLOCK)((UINT_PTR)pBaseRelocation + sizeof(IMAGE_BASE_RELOCATION));
			UINT_PTR uDifference = ((UINT_PTR)pvImageBase - pNtHeader->OptionalHeader.ImageBase);

			for (DWORD dwCount = 0; dwCount < dwBlockCounts; dwCount++)
			{

				PUINT_PTR pAddress = (PUINT_PTR)((UINT_PTR)pvImageBase + pBaseRelocation->VirtualAddress + pBlocks[dwCount].Offset);

				switch (pBlocks[dwCount].Type)
				{
				case IMAGE_REL_BASED_DIR64:
				case IMAGE_REL_BASED_HIGHLOW:
					*pAddress += uDifference;
					break;
				case IMAGE_REL_BASED_HIGH:
					*pAddress += HIWORD(uDifference);
					break;
				case IMAGE_REL_BASED_LOW:
					*pAddress += LOWORD(uDifference);
					break;
				}
			}
		}

		//next relocation table
		pBaseRelocation = (PIMAGE_BASE_RELOCATION)((UINT_PTR)pBaseRelocation + pBaseRelocation->SizeOfBlock);
		
	}

	dwStatus = ERROR_SUCCESS;

	return dwStatus;
}


/*

	导入表修复

*/

DWORD fill_import_table(const IMAGE_NT_HEADERS *pNtHeader, PVOID pvImageBase)
{
	DWORD dwStatus = ERROR_INVALID_PARAMETER;

	DWORD dwSize = pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
	DWORD dwVirtualAddress = pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	if (!(dwSize && dwVirtualAddress))
	{
		// no import table
		dwStatus = ERROR_DATABASE_DOES_NOT_EXIST;
		return dwStatus;
	}


	const IMAGE_IMPORT_DESCRIPTOR* pDescriptor = (const PIMAGE_IMPORT_DESCRIPTOR)((UINT_PTR)pvImageBase + dwVirtualAddress);;
	if (IsBadReadPtr(pDescriptor, sizeof(PIMAGE_IMPORT_DESCRIPTOR)))
	{
		// probably wrong ImageBase
		dwStatus = ERROR_ACCESS_DENIED;
		return dwStatus;
	}


	//the final descriptor is a blank entry
	while (pDescriptor->Name != NULL)
	{

		LPCSTR szModuleName = (LPCSTR)((UINT_PTR)pvImageBase + pDescriptor->Name);
		HMODULE hModule = LoadLibraryA(szModuleName);
		if (hModule == NULL)
		{
			dwStatus = ERROR_MOD_NOT_FOUND;
			return dwStatus;
		}

		PIMAGE_THUNK_DATA pINTThunk = NULL;
		PIMAGE_THUNK_DATA pIATThunk = NULL;

		if (pDescriptor->OriginalFirstThunk != 0)
		{
			pINTThunk = (PIMAGE_THUNK_DATA)((UINT_PTR)pvImageBase + pDescriptor->OriginalFirstThunk);
		}
		else
		{
			pINTThunk = (PIMAGE_THUNK_DATA)((UINT_PTR)pvImageBase + pDescriptor->FirstThunk);
		}

		pIATThunk = (PIMAGE_THUNK_DATA)((UINT_PTR)pvImageBase + pDescriptor->FirstThunk);


		while (pIATThunk &&
			pINTThunk &&
			pINTThunk->u1.AddressOfData)
		{
			if (IMAGE_SNAP_BY_ORDINAL(pINTThunk->u1.Ordinal))
			{
				LPCSTR Ordinal = (LPCSTR)IMAGE_ORDINAL(pIATThunk->u1.Ordinal);
				pIATThunk->u1.Function = (UINT_PTR)GetProcAddress(hModule, Ordinal);
			}
			else
			{
				PIMAGE_IMPORT_BY_NAME pImportByName = (PIMAGE_IMPORT_BY_NAME)((UINT_PTR)pvImageBase + pINTThunk->u1.AddressOfData);
				pIATThunk->u1.Function = (UINT_PTR)GetProcAddress(hModule, pImportByName->Name);
			}

			pINTThunk ++;
			pIATThunk ++;
		}


		pDescriptor ++;
	}

	dwStatus = ERROR_SUCCESS;

	return dwStatus;
}