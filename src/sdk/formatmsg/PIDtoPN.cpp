// PIDtoPN.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#ifdef __cplusplus
extern "C" {
#endif

// defines, types
#define SystemProcessesAndThreadsInformation    5
#define STATUS_INFO_LENGTH_MISMATCH      (0xC0000004L)
typedef LONG    KPRIORITY;



// structs
typedef struct _CLIENT_ID {
    DWORD         UniqueProcess;
    DWORD         UniqueThread;
} CLIENT_ID;

typedef struct _VM_COUNTERS {
    SIZE_T        PeakVirtualSize;
    SIZE_T        VirtualSize;
    ULONG         PageFaultCount;
    SIZE_T        PeakWorkingSetSize;
    SIZE_T        WorkingSetSize;
    SIZE_T        QuotaPeakPagedPoolUsage;
    SIZE_T        QuotaPagedPoolUsage;
    SIZE_T        QuotaPeakNonPagedPoolUsage;
    SIZE_T        QuotaNonPagedPoolUsage;
    SIZE_T        PagefileUsage;
    SIZE_T        PeakPagefileUsage;
} VM_COUNTERS;

typedef struct _SYSTEM_THREADS {
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER CreateTime;
    ULONG         WaitTime;
    PVOID         StartAddress;
    CLIENT_ID     ClientId;
    KPRIORITY     Priority;
    KPRIORITY     BasePriority;
    ULONG         ContextSwitchCount;
    LONG          State;
    LONG          WaitReason;
} SYSTEM_THREADS, * PSYSTEM_THREADS;

typedef struct _SYSTEM_PROCESSES {
    ULONG             NextEntryDelta;
    ULONG             ThreadCount;
    ULONG             Reserved1[6];
    LARGE_INTEGER     CreateTime;
    LARGE_INTEGER     UserTime;
    LARGE_INTEGER     KernelTime;
    UNICODE_STRING    ProcessName;
    KPRIORITY         BasePriority;
    ULONG             ProcessId;
    ULONG             InheritedFromProcessId;
    ULONG             HandleCount;
    ULONG             Reserved2[2];
    VM_COUNTERS       VmCounters;
#if _WIN32_WINNT >= 0x500
    IO_COUNTERS       IoCounters;
#endif
    SYSTEM_THREADS    Threads[1];
} SYSTEM_PROCESSES, * PSYSTEM_PROCESSES;


// functions
ULONG (__stdcall *NtQuerySystemInformation) (
    ULONG   SystemInformationClass,
    PVOID   SystemInformation,
    ULONG   SystemInformationLength,
    PULONG  ReturnLength
) = NULL;


extern "C"
PWCHAR
FindProcessNameW(
    ULONG pid
    )
{
    WCHAR* ImagePathName = NULL;
    //ULONG ImagePathNameLen = MAX_PATH+1; // +1 if zero terminated - examine it!
    ULONG Status;
    ULONG cbBuffer = 0x8000;
    PVOID pBuffer = NULL;
    ULONG l=0;        
    
    if (NtQuerySystemInformation == NULL)
    {
        NtQuerySystemInformation = (unsigned long (__stdcall *)(ULONG   SystemInformationClass,
                                                                PVOID   SystemInformation,
                                                                ULONG   SystemInformationLength,
                                                                PULONG  ReturnLength)) GetProcAddress( GetModuleHandle("ntdll.dll"), "NtQuerySystemInformation" );
    }
    
    do {
        pBuffer = GlobalAlloc(GMEM_FIXED, cbBuffer);
        if(!pBuffer)
            return NULL;

        Status = NtQuerySystemInformation(
                    SystemProcessesAndThreadsInformation,
                    pBuffer, cbBuffer, NULL);

        if (Status == STATUS_INFO_LENGTH_MISMATCH) {
            GlobalFree(pBuffer);
            cbBuffer *= 2;
        }
    } while (Status == STATUS_INFO_LENGTH_MISMATCH ||
             Status != 0);

    PSYSTEM_PROCESSES pProcesses = (PSYSTEM_PROCESSES)pBuffer;
    for (;;)
    {
        if(pid == pProcesses->ProcessId)
        {
            ImagePathName = (WCHAR*) GlobalAlloc(GMEM_FIXED, pProcesses->ProcessName.Length+sizeof(WCHAR) );
            memcpy( ImagePathName, pProcesses->ProcessName.Buffer , pProcesses->ProcessName.Length );
            ImagePathName[pProcesses->ProcessName.Length/sizeof(WCHAR)] =  0;
            break;
        }
        if(!pProcesses->NextEntryDelta)        
            break;
        l += pProcesses->NextEntryDelta;
        if(l > cbBuffer)
            break;
        pProcesses = (PSYSTEM_PROCESSES)(((LPBYTE)pProcesses)
                        + pProcesses->NextEntryDelta);
        if(l+pProcesses->NextEntryDelta > cbBuffer)
            break;
    }

    GlobalFree(pBuffer);
    return ImagePathName;
} // end FindProcessNameW()

#ifdef __cplusplus
};
#endif

