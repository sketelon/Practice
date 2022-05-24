/*
	内核链表
*/

#define MEMORY_TAG 'mem'
typedef struct _MYDATA
{
	LIST_ENTRY ListEntry;
	ULONG x;
	ULONG y;
}MYDATA, *PMYDATA;

void list_demo()
{
	/*
		初始化头部链表
	*/
	LIST_ENTRY	ListHead;
	InitializeListHead(&ListHead);

	/*
		将数据插入链表中
	*/
	PMYDATA pMyDataTemp = (PMYDATA)ExAllocatePoolWithTag(PagedPool, 
		sizeof(PMYDATA), MEMORY_TAG);
	if (NULL == pMyDataTemp) {
		goto _EXIT;
	}

	// 若需同步，可调用 ExInterlockedInsertHeadList 
	InsertHeadList(&ListHead, &pMyDataTemp->ListEntry);


	/*
		遍历链表
	*/
	for (PLIST_ENTRY pEntry = ListHead.Flink;
		pEntry != &ListHead; pEntry = pEntry->Flink)
	{
		PMYDATA pMyData = CONTAINING_RECORD(pEntry,	
			MYDATA,
			ListEntry);

		// do something

	}

_EXIT:
	/*
		移除链表数据
	*/
	while (!IsListEmpty(&ListHead)) {
		PLIST_ENTRY pEntry = RemoveTailList(&ListHead);
		PMYDATA pMyData = CONTAINING_RECORD(pEntry,
			MYDATA,
			ListEntry);
		// do something

		ExFreePoolWithTag(pMyData, MEMORY_TAG);
	}
}


/*
	内核自旋锁
*/
void interlocked_demo()
{
	// 注意调用 KeAcquireSpinLock 会将 IRQL 提升至 DISPATCH_LEVEL

	KSPIN_LOCK SpinLock;
	KeInitializeSpinLock(&SpinLock);
	KIRQL OldIrql;
	KeAcquireSpinLock(&SpinLock, &OldIrql);

	KeReleaseSpinLock(&SpinLock, OldIrql);
}


/*
	异常处理
*/

void exception_demo()
{
	__try
	{
		PMYDATA pData = NULL;
		pData->x = 1;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		// 进入到 __except 进行处理，处理完后不再回到__try{}块中，转而继续执行。
		KdPrint(("EXCEPTION_EXECUTE_HANDLER\n"));
	}
}