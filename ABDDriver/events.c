#include <ntifs.h>
#include <wdm.h>
#include <ntimage.h>
#include "defs.hpp"
#include "memory_operations.hpp"


#pragma warning (disable : 4047 6273 4090 4133)

#define BLACKLIST_COUNT (sizeof(blacklisted_dlls) / sizeof(blacklisted_dlls[0]))





WCHAR blacklisted_dlls[][256] = {
    L"bdhkm64.dll",
    L"atcuf64.dll"
};

UCHAR blocker[] = { 0x31, 0xC0, 0xC3, 0x90 };



NTSTATUS BlockEntry(PVOID entryPoint) {

    KAPC_STATE apc_state = { 0 };
    KeStackAttachProcess(unwanted.processData, &apc_state);

    __try {
        ProbeForWrite(entryPoint, sizeof(blocker), sizeof(ULONG));
        RtlCopyMemory(entryPoint, blocker, sizeof(blocker));
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        
        DEBUG_PRINT("[--] Exception occured on write\n");
        KeUnstackDetachProcess(&apc_state);
        return STATUS_ACCESS_VIOLATION;
    }

    KeUnstackDetachProcess(&apc_state);
 
    return STATUS_SUCCESS;
}


PVOID GetEntryPoint(PVOID ImageBaseAddress) {

    if (!ImageBaseAddress) return NULL;

    KAPC_STATE apc_state = { 0 };
    KeStackAttachProcess(unwanted.processData, &apc_state);

    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)ImageBaseAddress;
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) return NULL;

    PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((PUCHAR)ImageBaseAddress + dosHeader->e_lfanew);
    if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) return NULL;

    ULONG entryPointRVA = ntHeaders->OptionalHeader.AddressOfEntryPoint;
    if (!entryPointRVA) return NULL;

    KeUnstackDetachProcess(&apc_state);

    return (PUCHAR)ImageBaseAddress + entryPointRVA;
}


BOOLEAN IsInBlacklist(PWCH Buffer) {

    UNICODE_STRING tocomp = { 0 };

    for (ULONG i = 0; i < BLACKLIST_COUNT; i++) {
        RtlInitUnicodeString(&tocomp, blacklisted_dlls[i]);
        if (tocomp.Buffer && wcsstr(Buffer, tocomp.Buffer) != NULL && wcscmp(Buffer, L"") != 0) {
            return TRUE;
        }
    }
    return FALSE;
}


PLOAD_IMAGE_NOTIFY_ROUTINE FilterUnwantedDll(PUNICODE_STRING FullImageName, HANDLE Process, PIMAGE_INFO ImageInfo) {

    if (FullImageName && FullImageName->Buffer) {

        KIRQL oldIrql;
        KeAcquireSpinLock(&unwantedLock, &oldIrql);

        if (unwanted.hProcess == Process && IsInBlacklist(FullImageName->Buffer) == TRUE) {
            unwanted.hModule = ImageInfo->ImageBase;
            PVOID entry = GetEntryPoint(ImageInfo->ImageBase);
            if (entry) {
                DEBUG_PRINT("[!] Unwanted DLL detected in a Protected Process %lu at %p with Entrypoint at %p: %ws\n", HandleToULong(Process), ImageInfo->ImageBase, entry, FullImageName->Buffer);
                
                /*
                if (NT_SUCCESS(BlockEntry(entry))) {
                    DEBUG_PRINT("[+] Unwanted DLL blocked\n");
                }
                */

            }
        }

        KeReleaseSpinLock(&unwantedLock, oldIrql);
    }

    return STATUS_SUCCESS;
}



PCREATE_PROCESS_NOTIFY_ROUTINE_EX FilterProtectedProcess(PEPROCESS Process, HANDLE ProcessId, PPS_CREATE_NOTIFY_INFO CreateInfo) {
    
    //UNREFERENCED_PARAMETER(Process);

    if (CreateInfo && CreateInfo->ImageFileName) {

        if (wcsstr(CreateInfo->CommandLine->Buffer, TO_PROTECT) != NULL)
        {
            // lock the unwanted_dll strucut to avoid race condition
            // between this callback write op and the dll load callback read op

            KIRQL oldIrql;
            KeAcquireSpinLock(&unwantedLock, &oldIrql);
            
            unwanted.hProcess = ProcessId;
            unwanted.processData = Process;
            
            KeReleaseSpinLock(&unwantedLock, oldIrql);

        }
    }

    return STATUS_SUCCESS;
}
