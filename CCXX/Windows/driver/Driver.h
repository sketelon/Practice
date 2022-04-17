#pragma once
#include <ntddk.h>

#define DEVICE_NAME    L"\\Device\\MyHelloDeviceName"
#define SYMBOLIC_NAME  L"\\??\\MyHelloSymLinkName"


#define REQUEST_MY           CTL_CODE(FILE_DEVICE_UNKNOWN, 0x101, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)


typedef struct _DEVICE_EXTENSION {
	PDEVICE_OBJECT pDevice;
	UNICODE_STRING ustrDeviceName;
	UNICODE_STRING ustrSymLinkName;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

DRIVER_INITIALIZE DriverEntry;
NTSTATUS CreateDevice (IN PDRIVER_OBJECT pDriverObject);
VOID OnUnload(IN PDRIVER_OBJECT pDriverObject);
NTSTATUS DispatchRoutine(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp);


#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(INIT, CreateDevice)
//#pragma alloc_text(PAGE, DispatchRoutine)

#endif