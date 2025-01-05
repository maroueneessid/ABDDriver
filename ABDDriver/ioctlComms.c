#include "defs.hpp"
#include <ntddk.h>




NTSTATUS CreateCall(PDEVICE_OBJECT DeviceObject, PIRP Irp) {

	UNREFERENCED_PARAMETER(DeviceObject);
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;

}


NTSTATUS CloseCall(PDEVICE_OBJECT DeviceObject, PIRP Irp) {

	UNREFERENCED_PARAMETER(DeviceObject);
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;

}


NTSTATUS IoControl(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    ULONG ByteIO = 0;

    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    ULONG controlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;
    PUNWATED_DLL_DATA output = (PUNWATED_DLL_DATA)Irp->AssociatedIrp.SystemBuffer;

    switch (controlCode) {
    case IOCTL_UNWANTED_DLL_DETECTION:
        status = STATUS_SUCCESS;
        *output = unwanted;
        ByteIO = sizeof(UNWANTED_DLL_DATA);
        break;

    default:
        ByteIO = 0;
        break;
    }

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = ByteIO;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return status;
}






