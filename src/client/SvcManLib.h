#ifndef __SERVICE_MANAGEMENT_LIB__H__
#define __SERVICE_MANAGEMENT_LIB__H__

#include "windows.h"
#include "winsvc.h"
#include <stdio.h>

UINT
NtServiceIsRunning(
    LPCTSTR ServiceName
    );

UINT
NtServiceStart(
    LPCTSTR ServiceName
    );

UINT
NtServiceStop(
    LPCTSTR ServiceName,
    ULONG   TimeoutSeconds
    );

UINT
NtServiceInstall(
    LPCTSTR ServiceName,
    PCHAR   PathToExecutable,
    BOOLEAN KernelDriver = TRUE,
    ULONG   StartType = SERVICE_DEMAND_START,
    PCHAR   Dependencies = NULL
    );

UINT
NtServiceRemove(
    LPCTSTR ServiceName
    );

UINT
NtServiceSetStartMode(
    LPCTSTR ServiceName,
    ULONG StartMode
    );

UINT
NtServiceGetStartMode(
    LPCTSTR ServiceName,
    ULONG*  StartMode
    );

#endif //__SERVICE_MANAGEMENT_LIB__H__
