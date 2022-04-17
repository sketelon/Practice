#include "Driver.h"





NTSTATUS DriverEntry (
			IN PDRIVER_OBJECT pDriverObject,
			IN PUNICODE_STRING pRegistryPath	) 
{
	UNREFERENCED_PARAMETER(pRegistryPath);

	NTSTATUS status = STATUS_UNSUCCESSFUL;

	KdPrint(("Enter DriverEntry\n"));

	pDriverObject->DriverUnload = OnUnload;
	for (SIZE_T i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		pDriverObject->MajorFunction[i] = DispatchRoutine;
	}

	status = CreateDevice(pDriverObject);

	KdPrint(("DriverEntry end\n"));
	return status;
}


NTSTATUS CreateDevice (
		IN PDRIVER_OBJECT	pDriverObject) 
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PDEVICE_OBJECT pDevObj = NULL;
	PDEVICE_EXTENSION pDevExt = NULL;
	UNICODE_STRING ustrDevName = RTL_CONSTANT_STRING(DEVICE_NAME);
	UNICODE_STRING ustrSymLinkName = RTL_CONSTANT_STRING(SYMBOLIC_NAME);

	status = IoCreateDevice(pDriverObject,
						sizeof(DEVICE_EXTENSION),	// allocate memory to pDevExt
						&ustrDevName,
						FILE_DEVICE_UNKNOWN,
						0, TRUE,
						&pDevObj);
	if (!NT_SUCCESS(status))
	{
		return status;
	}


	pDevObj->Flags |= DO_BUFFERED_IO;
	pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
	
	pDevExt->pDevice = pDevObj;
	RtlInitUnicodeString(&(pDevExt->ustrDeviceName), DEVICE_NAME);
	RtlInitUnicodeString(&(pDevExt->ustrSymLinkName), SYMBOLIC_NAME);


	status = IoCreateSymbolicLink(&ustrSymLinkName, &ustrDevName);
	if (!NT_SUCCESS(status)) 
	{
		IoDeleteDevice(pDevObj);
		return status;
	}

	return STATUS_SUCCESS;
}

VOID OnUnload(IN PDRIVER_OBJECT pDriverObject)
{
	KdPrint(("OnUnload\n"));

	PDEVICE_OBJECT	pNextObj = NULL;

	pNextObj = pDriverObject->DeviceObject;
	while (pNextObj != NULL) 
	{
		PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)
			pNextObj->DeviceExtension;

		IoDeleteSymbolicLink(&(pDevExt->ustrSymLinkName));
		IoDeleteDevice(pDevExt->pDevice);

		pNextObj = pNextObj->NextDevice;

	}
}

NTSTATUS DispatchRoutine(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
	UNREFERENCED_PARAMETER(pDeviceObject);
	NTSTATUS status = STATUS_SUCCESS;
	ULONG               inBufLength; 
	ULONG               outBufLength;

	pIrp->IoStatus.Information = 0;

	PIO_STACK_LOCATION pStack = IoGetCurrentIrpStackLocation(pIrp);
	if (NULL == pStack)
	{
		status = STATUS_INTERNAL_ERROR;
		goto _EXIT;
	}

	inBufLength = pStack->Parameters.DeviceIoControl.InputBufferLength;
	outBufLength = pStack->Parameters.DeviceIoControl.OutputBufferLength;


	switch (pStack->MajorFunction)
	{
	case IRP_MJ_WRITE:
	{
		if (0 != pStack->Parameters.Write.Length && pIrp->AssociatedIrp.SystemBuffer)
		{
			KdPrint(("%s", pIrp->AssociatedIrp.SystemBuffer));
		}
	}
	break;
	case IRP_MJ_READ:
	{
		if (0 != pStack->Parameters.Read.Length && pIrp->AssociatedIrp.SystemBuffer)
		{
			char* str = "Hello World!\n";
			strcpy(pIrp->AssociatedIrp.SystemBuffer, str);
			pIrp->IoStatus.Information = strlen(str) + 1;
		}
	}
	break;
	default:
		break;
	}

	switch (pStack->Parameters.DeviceIoControl.IoControlCode)
	{
	case REQUEST_MY:
	{
		if (pStack->Parameters.DeviceIoControl.InputBufferLength != 0)
		{
			
		}
	}
	break;
	default:
		break;
	};



_EXIT:
	pIrp->IoStatus.Status = status;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return pIrp->IoStatus.Status;
}