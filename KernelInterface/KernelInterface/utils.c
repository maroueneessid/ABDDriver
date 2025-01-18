#include "defs.hpp"

// IOCTL Block
BOOL BlockRequest(HANDLE hDevice, WRITE_OP_INPUT towrite) {

    DWORD bytesReturned;
    BOOL result = DeviceIoControl(hDevice, IOCTL_BLOCK_ENTRY, &towrite, sizeof(WRITE_OP_INPUT), &towrite, sizeof(WRITE_OP_INPUT), 0, 0);


    if (!result) {
        return FALSE;
    }


    return TRUE;
}

BOOL SuspendRequest(HANDLE hDevice) {

    DWORD bytesReturned;
    BOOL result = DeviceIoControl(
        hDevice,
        IOCTL_SUSPEND_PROCESS,
        NULL,
        0,
        NULL,
        0,
        &bytesReturned,
        NULL
    );

    if (!result) {
        return FALSE;
    }


    return TRUE;
}

BOOL ResumeRequest(HANDLE hDevice) {

    DWORD bytesReturned;
    BOOL result = DeviceIoControl(
        hDevice,
        IOCTL_RESUME_PROTECTED_PROCESS,
        NULL,
        0,
        NULL,
        0,
        &bytesReturned,
        NULL
    );

    if (!result) {
        return FALSE;
    }


    return TRUE;
}

BOOL FinishOperationRequest(HANDLE hDevice) {
    DWORD bytesReturned;
    BOOL result = DeviceIoControl(
        hDevice,
        IOCTL_FINISH_OPERATION,
        NULL,
        0,
        NULL,
        0,
        &bytesReturned,
        NULL
    );

    if (!result) {
        return FALSE;
    }


    return TRUE;
}

// UM Block
BOOL BlockEntry(UNWANTED_DLL_DATA data) {


    if (!data.hProcess || !data.pEntry) {
        printf("[ERROR] Invalid Data\n");
        return FALSE;
    }

    DWORD oldProtect = NULL;
    SIZE_T written = 0;

    HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, (DWORD)data.hProcess);
    if (!handle) {
        printf("[ERROR %lu] Failed to open process for the needed operation\n", GetLastError());
        return FALSE;
    }

    if (!VirtualProtectEx(handle, data.pEntry, sizeof(blocker), PAGE_EXECUTE_READWRITE, &oldProtect)) {
        printf("[ERROR %lu] Failed to change memory protection 1\n", GetLastError());
        return FALSE;
    }




    if (!WriteProcessMemory(handle, data.pEntry, blocker, sizeof(blocker), &written) || written != sizeof(blocker)) {
        printf("[ERROR %lu] Failed to write blocker\n", GetLastError());
        return FALSE;
    }


    if (!VirtualProtectEx(handle, data.pEntry, sizeof(blocker), oldProtect, &oldProtect)) {
        printf("[ERROR %lu] Failed to change memory protection 2\n", GetLastError());
        return FALSE;
    }


    return TRUE;
}
