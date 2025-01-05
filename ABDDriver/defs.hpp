#pragma once
#include <ntifs.h>
#include <ntdef.h>

#define TO_PROTECT L"msedge_helper.exe"
# define MAX_BLACKLISTED 10

// MACRO
#define DEBUG_PRINT(format , ...) DbgPrintEx(0, 0, format , __VA_ARGS__)

// IOCL Custom codes
#define IOCTL_UNWANTED_DLL_DETECTION CTL_CODE(FILE_DEVICE_UNKNOWN, 0x921, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define IOCTL_UNWANTED_DLL_READ CTL_CODE(FILE_DEVICE_UNKNOWN, 0x922, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)



typedef struct _UNWANTED_DLL_DATA {
		
	HANDLE hProcess;
	HANDLE hModule;
	PEPROCESS processData;
} UNWANTED_DLL_DATA , *PUNWATED_DLL_DATA ;


extern UNWANTED_DLL_DATA unwanted;
extern KSPIN_LOCK unwantedLock;



PDEVICE_OBJECT pDeviceObject;
UNICODE_STRING dev, dos;



NTSTATUS IoControl(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS CloseCall(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS CreateCall(PDEVICE_OBJECT DeviceObject, PIRP Irp);


// Some Helper Functions
PVOID GetEntryPoint(PVOID ImageBaseAddress);
NTSTATUS BlockEntry(PVOID entryPoint);












