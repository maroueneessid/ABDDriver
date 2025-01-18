#include "defs.hpp"
#include "memory_operations.hpp"
#include <ntddk.h>
#include <ntimage.h>
#include <wdm.h>
#pragma warning (disable:4022)


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
    
    PUNWATED_DLL_DATA_LIST output = (PUNWATED_DLL_DATA_LIST)Irp->AssociatedIrp.SystemBuffer;
    PWRITE_OP_INPUT input = (PWRITE_OP_INPUT)Irp->AssociatedIrp.SystemBuffer;

    PEPROCESS target;

    switch (controlCode) {
    
    case IOCTL_UNWANTED_DLL_DETECTION:
        status = STATUS_SUCCESS;
        *output = listOfUnwanted;
        ByteIO = sizeof(UNWANTED_DLL_DATA_LIST); 
        break;

    case IOCTL_RESUME_PROTECTED_PROCESS:
        if (NT_SUCCESS(PsLookupProcessByProcessId(listOfUnwanted.entry[GlobalUnwantedIndex-1].hProcess, &target))) {
            status = STATUS_SUCCESS;
            DEBUG_PRINT("[IOCTL] RESUME\n");
            PsResumeProcess(target);
        } else { status = STATUS_UNSUCCESSFUL; }
        ByteIO = 0;
        break;

    case IOCTL_SUSPEND_PROCESS:
        if (NT_SUCCESS(PsLookupProcessByProcessId(listOfUnwanted.entry[GlobalUnwantedIndex-1].hProcess, &target))) {
            status = STATUS_SUCCESS;
            DEBUG_PRINT("[IOCTL] SUSPEND\n");
            PsSuspendProcess(target);
        } else { status = STATUS_UNSUCCESSFUL; }
        ByteIO = 0;
        break;

    // UNUSED for now
    case IOCTL_BLOCK_ENTRY:
        // Info Log
        DEBUG_PRINT("[INFO] hProcess=0x%p, pBuffer=0x%p, pEntry=0x%p, szBuffer=%lu\n",
            input->hProcess, input->pBuffer, input->pEntry, input->szBuffer);

        if (input->pBuffer && input->szBuffer > 0) {
            DEBUG_PRINT("Buffer Contents: ");
            for (ULONG i = 0; i < input->szBuffer; i++) {
                DEBUG_PRINT("%02X ", ((unsigned char*)input->pBuffer)[i]);
            }
        } else { DEBUG_PRINT("Buffer is NULL or size is zero.\n"); }

        if (NT_SUCCESS(PsLookupProcessByProcessId((HANDLE)input->hProcess, &target))) {
            
            //KAPC_STATE apcState;
            //KeStackAttachProcess(target, &apcState);
            status = KeWriteVirtualMemory(target, input->pBuffer, input->pEntry, input->szBuffer);
            //KeUnstackDetachProcess(&apcState);
            //ObDereferenceObject(target);

            DEBUG_PRINT("[BLOCK IOCTL WITH 0x%X]\n", status);
        } else { status = STATUS_UNSUCCESSFUL; }
        ByteIO = sizeof(WRITE_OP_INPUT);
        break;

    case IOCTL_FINISH_OPERATION:
        DEBUG_PRINT("[IOCTL] FINISH: Emptying existing entries\n");
        memset(&listOfUnwanted, 0, sizeof(UNWANTED_DLL_DATA_LIST));
        GlobalUnwantedIndex = 0;
        ByteIO = 0;
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






