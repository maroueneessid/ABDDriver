#include <ntifs.h>
#include <ntddk.h>
#include "defs.hpp"
#include "events.hpp"

#pragma warning (disable: 4047 4024)

UNWANTED_DLL_DATA unwanted;
UNWANTED_DLL_DATA_LIST  listOfUnwanted = { 0 };
KSPIN_LOCK unwantedLock = 0;
int GlobalUnwantedIndex = 0;





NTSTATUS DriverUnload(PDRIVER_OBJECT DriverObject) {


    UNREFERENCED_PARAMETER(DriverObject);
    NTSTATUS status = STATUS_SUCCESS;
    DEBUG_PRINT("[+] Driver Unloaded\n");

    PsSetCreateProcessNotifyRoutineEx(FilterProtectedProcess, TRUE);
    PsRemoveLoadImageNotifyRoutine(FilterUnwantedDll);

    IoDeleteSymbolicLink(&dos);
    IoDeleteDevice(DriverObject->DeviceObject);

    return status;

}



NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT     DriverObject,
    _In_ PUNICODE_STRING    RegistryPath
)
{

    UNREFERENCED_PARAMETER(RegistryPath);
    KeInitializeSpinLock(&unwantedLock);




    NTSTATUS status = STATUS_SUCCESS;
    DriverObject->DriverUnload = DriverUnload;
    DEBUG_PRINT("[+] Driver loaded\n");

    PsSetCreateProcessNotifyRoutineEx(FilterProtectedProcess, FALSE);
    PsSetLoadImageNotifyRoutine(FilterUnwantedDll);
    

    RtlInitUnicodeString(&dev, L"\\Device\\AntiBD");
    RtlInitUnicodeString(&dos, L"\\DosDevices\\AntiBD");

    IoCreateDevice(DriverObject, 0, &dev, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &pDeviceObject);
    IoCreateSymbolicLink(&dos, &dev);



    DriverObject->MajorFunction[IRP_MJ_CREATE] = CreateCall;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = CloseCall;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IoControl;



    DriverObject->Flags |= DO_DIRECT_IO;
    DriverObject->Flags &= ~DO_DEVICE_INITIALIZING;
    


    return status;
}

