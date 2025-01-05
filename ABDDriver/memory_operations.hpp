#pragma once
#include <ntdef.h>
#include <ntifs.h>
#include <ntddk.h>
#include <ntstrsafe.h>
#pragma comment (lib, "ntoskrnl.lib")


__declspec (dllimport) NTSTATUS NTAPI MmCopyVirtualMemory
(
    PEPROCESS SourceProcess,
    PVOID SourceAddress,
    PEPROCESS TargetProcess,
    PVOID TargetAddress,
    SIZE_T BufferSize,
    KPROCESSOR_MODE PreviousMode,
    PSIZE_T ReturnSize
);


NTSTATUS KeReadVirtualMemory(PEPROCESS Process, PVOID Src, PVOID Dst, SIZE_T Size) {
    SIZE_T returned;
    return MmCopyVirtualMemory(Process, Src, PsGetCurrentProcess(), Dst, Size, KernelMode, &returned);
}


NTSTATUS KeWriteVirtualMemory(PEPROCESS Process, PVOID Src, PVOID Dst, SIZE_T Size) {
    SIZE_T returned;
    return MmCopyVirtualMemory(PsGetCurrentProcess(), Src, Process, Dst, Size, KernelMode, &returned);
}