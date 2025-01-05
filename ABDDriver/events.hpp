#pragma once

#include <ntifs.h>

PCREATE_PROCESS_NOTIFY_ROUTINE_EX FilterProtectedProcess(PEPROCESS Process, HANDLE ProcessId, PPS_CREATE_NOTIFY_INFO CreateInfo);
PLOAD_IMAGE_NOTIFY_ROUTINE FilterUnwantedDll(PUNICODE_STRING FullImageName, HANDLE Process, PIMAGE_INFO ImageInfo);
