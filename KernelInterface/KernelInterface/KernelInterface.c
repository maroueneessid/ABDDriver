#include "defs.hpp"

UNWANTED_DLL_DATA_LIST  listOfUnwanted;
int unwantedListIndex = 0;
BOOL active = TRUE;
HANDLE hDevice = NULL;
extern UCHAR blocker[] = { 0x31, 0xC0, 0xC3, 0x90, 0x90 };


void StartDebugging(UNWANTED_DLL_DATA data) {

    DEBUG_EVENT dbgEvent;
    active = TRUE;

    if (!DebugActiveProcess((DWORD)data.hProcess)) {
        printf("[ERROR 0x%lu] Could not attach debugger to target process.\n",GetLastError());
        return FALSE;
    } else { printf("[Success] Debugger attached to %lu\n", data.hProcess); }


    while (active) {

        if (!WaitForDebugEvent(&dbgEvent, INFINITE)) {
            printf("[ERROR 0x%lu] Could not wait for debug event.\n", GetLastError());
            return FALSE;
        }

        switch (dbgEvent.dwDebugEventCode) {

        case LOAD_DLL_DEBUG_EVENT:
             if (dbgEvent.u.LoadDll.lpBaseOfDll == data.hModule) {
                
                printf("[INFO] Blacklisted Dll fully loaded\n");
                
                if (SuspendRequest(hDevice)) {
                    printf("[INFO] Sending Suspension Request\n");
                }

                printf("[INFO] Detaching and stopping Debugger\n");                
                active = FALSE;
                DebugActiveProcessStop(listOfUnwanted.entry[unwantedListIndex].hProcess);

                if (BlockEntry(listOfUnwanted.entry[unwantedListIndex])) {
                    printf("[INFO] Entry Point of blacklisted Dll blocked\n");
                }
                
                printf("[INFO] Send Resuming Request\n");
                if (ResumeRequest(hDevice)) {
                    printf("[INFO] Send Resuming Request\n");
                }

                
             }

            break;

        default:
            break;
        }
        ContinueDebugEvent(dbgEvent.dwProcessId, dbgEvent.dwThreadId, DBG_CONTINUE);
    }
}



int main()
{

    hDevice = CreateFile(L"\\\\.\\AntiBD", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

    if (hDevice == INVALID_HANDLE_VALUE) {
        printf("Failed to open device. Error: %d\n", GetLastError());
        return 1;
    }

    while (1) {
        DWORD bytesReturned;
        BOOL result = DeviceIoControl(
            hDevice,
            IOCTL_UNWANTED_DLL_DETECTION,
            NULL,
            0,
            &listOfUnwanted,
            sizeof(UNWANTED_DLL_DATA_LIST),
            &bytesReturned,
            NULL
        );

        if (result && bytesReturned == sizeof(UNWANTED_DLL_DATA_LIST)) {

            if (HandleToULong(listOfUnwanted.entry[unwantedListIndex].hProcess) != 0) {
                UNWANTED_DLL_DATA tmp = listOfUnwanted.entry[unwantedListIndex];


                DWORD threadId;
                HANDLE thread = CreateThread(NULL, 0, StartDebugging, &tmp, 0, &threadId);
                if (!thread) {
                    printf("[ERROR %lu] Error creating the debugger thread\n", GetLastError());
                    break;
                }
                else { printf("[INFO] Started Debugger thread on %lu\n", threadId); }


                if (!ResumeRequest(hDevice)) {
                    printf("[ERROR] Error resuming process 1\n");
                }

                WaitForSingleObject(thread, 2000);
                CloseHandle(thread);

                unwantedListIndex++;


            }
        }
        DWORD exitCode;
        GetExitCodeProcess(listOfUnwanted.entry[0].hProcess, &exitCode);
        if (listOfUnwanted.entry[0].hProcess && exitCode != STILL_ACTIVE) {
            printf("[INFO] Sending Finish Request to empty existing entries\n");
            printf("=====================================================================\n");
            FinishOperationRequest(hDevice);
            memset(&listOfUnwanted, 0, sizeof(PUNWATED_DLL_DATA_LIST));
            unwantedListIndex = 0;
        }

    }

    // Cleanup
    CloseHandle(hDevice);
    return 0;
}
