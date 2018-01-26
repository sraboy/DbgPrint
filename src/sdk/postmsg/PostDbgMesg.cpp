#ifndef USER_MODE
#define USER_MODE
#endif //USER_MODE

#ifdef KERNEL_MODE
#include "nt_2_zw.h"
#endif

#ifdef NATIVE_MODE
#include "nt_native.h"
#else
#include "windows.h"
#endif //ENV_XXX

#include "stdio.h"
#include "stdarg.h"

#ifdef KERNEL_MODE

typedef UCHAR KIRQL;
typedef KIRQL *PKIRQL;

#define PASSIVE_LEVEL 0             // Passive release level
#define LOW_LEVEL 0                 // Lowest interrupt level
#define APC_LEVEL 1                 // APC interrupt level
#define DISPATCH_LEVEL 2            // Dispatcher level

extern "C"
KIRQL
__stdcall
KeGetCurrentIrql();

#endif

#include "..\..\inc\DbgPrnHk.h"
#include "..\..\inc\tools.h"
#include "PostDbgMesg.h"

HANDLE h_DbgPrintHookDriver = NULL;
BOOLEAN RememberNoDriver = TRUE;

BOOLEAN
DbgDump_i_ConnectDriver()
{
#ifdef NATIVE_MODE
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK   IoStatus;
    UNICODE_STRING fName;
    NTSTATUS Status;
#endif //ENV_XXX
    HANDLE h;
    
    if(h_DbgPrintHookDriver == (HANDLE)(-1))
        return FALSE;
    if(h_DbgPrintHookDriver)
        return TRUE;

#ifdef NATIVE_MODE

#ifdef KERNEL_MODE
    if(KeGetCurrentIrql() != PASSIVE_LEVEL)
        return FALSE;
#endif


    fName.Buffer = NT_DbgPrnHk_NAME_W;
    fName.Length = sizeof(NT_DbgPrnHk_NAME_W) - sizeof(WCHAR);
    fName.MaximumLength = sizeof(NT_DbgPrnHk_NAME_W);
    InitializeObjectAttributes(&ObjectAttributes, &fName, OBJ_CASE_INSENSITIVE, NULL, NULL);

    Status = NtCreateFile(&h,
                             GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                             &ObjectAttributes,
                             &IoStatus,
                             NULL,
                             FILE_ATTRIBUTE_NORMAL,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             FILE_OPEN,
                             FILE_SYNCHRONOUS_IO_NONALERT | FILE_COMPLETE_IF_OPLOCKED /*| FILE_WRITE_THROUGH*/,
                             NULL,
                             0);
    if(!NT_SUCCESS(Status)) {
        if(RememberNoDriver) {
            h_DbgPrintHookDriver = (HANDLE)(-1);
        }
        return FALSE;
    }
#else
    h = CreateFile(NT_DbgPrnHk_USER_NAME, GENERIC_READ | GENERIC_WRITE,
                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                   NULL,
                   OPEN_EXISTING,
                   FILE_ATTRIBUTE_NORMAL/* | FILE_FLAG_NO_BUFFERING*/,  NULL);
    if(!h || h == (HANDLE)(-1)) {
        if(RememberNoDriver) {
            h_DbgPrintHookDriver = (HANDLE)(-1);
        }
        return FALSE;
    }
#endif //ENV_XXX
    h_DbgPrintHookDriver = h;
    return TRUE;
} // end DbgDump_i_ConnectDriver()

BOOLEAN
DbgDump_i_SendIoctl(
    HANDLE h,
    ULONG Ioctl,
    PVOID inBuffer,
    ULONG inLength,
    PVOID outBuffer,
    ULONG outLength,
    PULONG returned
    )
{
#ifdef NATIVE_MODE
    IO_STATUS_BLOCK   IoStatus;
    NTSTATUS Status;
#endif //ENV_XXX

#ifdef NATIVE_MODE
    Status = NtDeviceIoControlFile(h,
                NULL, NULL, NULL,
                &IoStatus,
                Ioctl,
                inBuffer,
                inLength,
                outBuffer,
                outLength
                );
/*    if(Status == STATUS_PENDING) {
        Status = NtWaitForSingleObject(h, FALSE, NULL);
        if(NT_SUCCESS(Status)) {
            Status = IoStatus.Status;
        }
    }*/
    *returned = IoStatus.Information;
    if(!NT_SUCCESS(Status)) {
        return FALSE;
    } else {
        return TRUE;
    }
#else
    if(!DeviceIoControl(h, Ioctl,
                        inBuffer, inLength,
                        outBuffer, outLength,
                        returned, NULL)) {
        return FALSE;
    } else {
        return TRUE;
    }
#endif //ENV_XXX
} // end DbgDump_i_SendIoctl()

extern "C"
BOOLEAN
_cdecl
DbgDump_Printf(
    PCHAR Format,
    ...
    )
{
    ULONG returned;
    CHAR buff[2048];
    va_list ap;
    va_start(ap, Format);
    BOOLEAN retval;
    ULONG len;

    if(!DbgDump_i_ConnectDriver())
        return FALSE;

    len = _vsnprintf(buff, sizeof(buff), Format, ap);
    buff[sizeof(buff)-1] = 0;

    if(!DbgDump_i_SendIoctl(h_DbgPrintHookDriver, IOCTL_DbgPrnHk_PostMessage,
                        buff, len,
                        NULL,0,
                        &returned)) {
        retval = FALSE;
    } else {
        retval = TRUE;
    }

    va_end(ap);
    return retval;
} // end DbgDump_Printf()


extern "C"
BOOLEAN
__stdcall
DbgDump_Print(
    PCHAR Msg
    )
{
    ULONG returned;
    if(!DbgDump_i_ConnectDriver())
        return FALSE;

    if(!DbgDump_i_SendIoctl(h_DbgPrintHookDriver, IOCTL_DbgPrnHk_PostMessage,
                        Msg, strlen(Msg),
                        NULL,0,
                        &returned)) {
        return FALSE;
    }
    return TRUE;
} // end DbgDump_Print()

extern "C"
BOOLEAN
__stdcall
DbgDump_Printn(
    PCHAR Msg,
    ULONG Length
    )
{
    ULONG returned;
    if(!DbgDump_i_ConnectDriver())
        return FALSE;

    if(!DbgDump_i_SendIoctl(h_DbgPrintHookDriver, IOCTL_DbgPrnHk_PostMessage,
                        Msg, Length,
                        NULL,0,
                        &returned)) {
        return FALSE;
    }
    return TRUE;
} // end DbgDump_Printn()

extern "C"
BOOLEAN
__stdcall
DbgDump_Bin(
    PCHAR Msg,
    ULONG Length
    )
{
    ULONG returned;
    if(!DbgDump_i_ConnectDriver())
        return FALSE;

    if(!DbgDump_i_SendIoctl(h_DbgPrintHookDriver, IOCTL_DbgPrnHk_PostBinMessage,
                        Msg, Length,
                        NULL,0,
                        &returned)) {
        return FALSE;
    }
    return TRUE;
} // end DbgDump_Printn()

extern "C"
BOOLEAN
__stdcall
DbgDump_Reconnect()
{
    if(!h_DbgPrintHookDriver)
        return DbgDump_i_ConnectDriver();
    if(h_DbgPrintHookDriver == (HANDLE)(-1))
        h_DbgPrintHookDriver = NULL;
    return DbgDump_i_ConnectDriver();
} // end DbgDump_Reconnect()

extern "C"
VOID
__stdcall
DbgDump_Disconnect()
{
    if(!h_DbgPrintHookDriver)
        return;
    if(h_DbgPrintHookDriver == (HANDLE)(-1))
        return;
#ifdef NATIVE_MODE
    NtClose(h_DbgPrintHookDriver);
#else
    CloseHandle(h_DbgPrintHookDriver);
#endif //ENV_XXX
    h_DbgPrintHookDriver = NULL;
} // end DbgDump_Disconnect()

extern "C"
VOID
__stdcall
DbgDump_SetAutoReconnect(
    BOOLEAN AutoReconnect
    )
{
    RememberNoDriver = !AutoReconnect;
} // end DbgDump_SetAutoReconnect()

BOOLEAN
__stdcall
DbgDump_PostMsgEx(
    PDbgPrnHk_PostMessageEx_USER_IN pPutMsgBuf
    )
{
    ULONG returned;
    if(!DbgDump_i_ConnectDriver())
        return FALSE;

    if(!DbgDump_i_SendIoctl(h_DbgPrintHookDriver, IOCTL_DbgPrnHk_PostMessageEx,
                        pPutMsgBuf, pPutMsgBuf->Length + offsetof(DbgPrnHk_PostMessageEx_USER_IN, Msg),
                        NULL,0,
                        &returned)) {
        return FALSE;
    }
    return TRUE;
} // end DbgDump_PostMsgEx()
