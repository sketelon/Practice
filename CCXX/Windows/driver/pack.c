
// for warning C4996: 'ExAllocatePool': was declared deprecated
#pragma warning(disable:4996)
#define BUFFER_SIZE 1024
VOID allocate_string(PUNICODE_STRING pUnicodeString, LPCWSTR lpStrings)
{

	pUnicodeString->Length = (USHORT)(wcslen(lpStrings) * sizeof(WCHAR));
	pUnicodeString->MaximumLength = BUFFER_SIZE;

	ASSERT(pUnicodeString->MaximumLength >= pUnicodeString->Length);

	pUnicodeString->Buffer = ExAllocatePool(PagedPool, BUFFER_SIZE);
	RtlCopyMemory(pUnicodeString->Buffer, lpStrings, pUnicodeString->Length);
}

VOID free_string(PUNICODE_STRING pUnicodeString)
{
	ExFreePool(pUnicodeString->Buffer);
	pUnicodeString->Buffer = NULL;
	pUnicodeString->Length = pUnicodeString->MaximumLength = 0;
}