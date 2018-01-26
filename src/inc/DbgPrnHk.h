///////////////////////////////////////////////////////////////////////////////
// DbgPrnHk.h

#ifndef __DEBUG_PRINT_HOOK_H__
#define __DEBUG_PRINT_HOOK_H__

extern "C" {

#pragma pack(push, 1)

#define DBGPRNHK_POOLTAG  'kHPD'

#ifndef USER_MODE

#pragma pack(push, 8)

#include <ntddk.h>                  // various NT definitions
#include <stddef.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "ntddk_ex.h"
#include "tools.h"

#pragma pack(pop)

#ifdef POOL_TAGGING
#ifdef ExAllocatePool
#undef ExAllocatePool
#endif //ExAllocatePool
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,DBGPRNHK_POOLTAG)
#endif //POOL_TAGGING

#define NT_DBGPRNHK_REG_PATH        L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Services\\DbgPrnHk"
#define NT_CPUDESCR_REG_PATH        L"\\REGISTRY\\MACHINE\\HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0"

#else //USER_MODE

#define NT_DBGPRNHK_REG_PATH        L"SYSTEM\\CurrentControlSet\\Services\\DbgPrnHk"
#define PAGE_SIZE 4096

#endif //USER_MODE

#define NT_DBGPRNHK_REG_TAG_VAL              L"Tag"
#define NT_DBGPRNHK_DRV_TAG                  0x0000DBC0
#define NT_DBGPRNHK_REG_GROUP_VAL            L"Group"

// Device names
#define NT_DbgPrnHk_DOS_NAME     ("\\DosDevices\\DbgPrnHk")
#define NT_DbgPrnHk_NAME         ("\\Device\\DbgPrnHk")
#define NT_DbgPrnHk_NAME_W       (L"\\Device\\DbgPrnHk")
#define NT_DbgPrnHk_USER_NAME    ("\\\\.\\DbgPrnHk")
#define NT_DbgPrnHk_USER_NAME_W  (L"\\\\.\\DbgPrnHk")
#define NT_DbgPrnHk_SVC_NAME     ("DbgPrnHk")

#ifndef CTL_CODE
#include "winioctl.h"
#endif //CTL_CODE

#include "version.h"

#define DPHK_CTL_CODE_X(a,b)    CTL_CODE(FILE_DEVICE_UNKNOWN, a,b, FILE_ANY_ACCESS )

#define IOCTL_DbgPrnHk_GetMessages           DPHK_CTL_CODE_X(0x801, METHOD_BUFFERED)
#define IOCTL_DbgPrnHk_SetMessageBufferSize  DPHK_CTL_CODE_X(0x802, METHOD_BUFFERED)
#define IOCTL_DbgPrnHk_GetVersion            DPHK_CTL_CODE_X(0x803, METHOD_BUFFERED)
#define IOCTL_DbgPrnHk_PostMessage           DPHK_CTL_CODE_X(0x804, METHOD_BUFFERED)
#define IOCTL_DbgPrnHk_GetRdtscCalibration   DPHK_CTL_CODE_X(0x805, METHOD_BUFFERED)
#define IOCTL_DbgPrnHk_GetRdtsc              DPHK_CTL_CODE_X(0x806, METHOD_BUFFERED)
#define IOCTL_DbgPrnHk_CanUnload             DPHK_CTL_CODE_X(0x807, METHOD_BUFFERED)
#define IOCTL_DbgPrnHk_PostMessageEx         DPHK_CTL_CODE_X(0x808, METHOD_BUFFERED)
#define IOCTL_DbgPrnHk_PostBinMessage        DPHK_CTL_CODE_X(0x809, METHOD_BUFFERED)
#define IOCTL_DbgPrnHk_ReadRegConf           DPHK_CTL_CODE_X(0x80a, METHOD_BUFFERED)
#define IOCTL_DbgPrnHk_GetDrvConf            DPHK_CTL_CODE_X(0x80b, METHOD_BUFFERED)
#define IOCTL_DbgPrnHk_SetDrvConf            DPHK_CTL_CODE_X(0x80c, METHOD_BUFFERED)

#define MAX_MSG_SIZE  (128-sizeof(LARGE_INTEGER)-sizeof(PVOID)*2-sizeof(USHORT)-sizeof(UCHAR)*2-sizeof(UCHAR)*4)

#define BufferOverflow_Continue        0x00
#define BufferOverflow_Stop            0x01
#define BufferOverflow_CallDebugger    0x02
#define BufferOverflow_Max             0x02

#define DoNotPassMessages_Off          0x00
#define DoNotPassMessages_On           0x01
#define DoNotPassMessages_NoBuffer     0x02

#define TimeStampType_SysPerfCounter   ((ULONG)0x00)
#define TimeStampType_RdtscPerfCounter ((ULONG)0x01)
#define TimeStampType_SysTime          ((ULONG)0x02)
#define TimeStampType_Max              TimeStampType_SysTime

#define DbgCallerMode_K                ((ULONG)0x00)
#define DbgCallerMode_U                ((ULONG)0x01)

#define MsgFlags_DbgCallerMode_K       (DbgCallerMode_K  << 2)
#define MsgFlags_DbgCallerMode_U       (DbgCallerMode_U  << 2)

#define DbgDataType_Text               ((ULONG)0x00)
#define DbgDataType_Bin                ((ULONG)0x01)
#define DbgDataType_Dup                ((ULONG)0x02)    // reference previous item
#define DbgDataType_Max                DbgDataType_Dup

#define MsgFlags_DbgDataType_Text      (DbgDataType_Text << (2+2))
#define MsgFlags_DbgDataType_Bin       (DbgDataType_Bin  << (2+2))
#define MsgFlags_DbgDataType_Dup       (DbgDataType_Dup  << (2+2))

#define MsgFlags_DumpStackFramePtr     (1                << (2+2+2))

#define MsgFlags_ValidMask             (((ULONG)0x1 << (2+2+2+1))-1)

typedef union _DbgPrnHk_MsgFlags {
    ULONG Flags;
    struct {
        ULONG         TimeStampType:2;
        ULONG         CallerMode:2;
        ULONG         DataType:2;
        ULONG         WithSF:1;
        ULONG         Reserved0:1;
        ULONG         Reserved:24;
    };
} DbgPrnHk_MsgFlags, *PDbgPrnHk_MsgFlags;

typedef struct _DbgPrnHk_MsgDup {
    ULONG DupCounter;
} DbgPrnHk_MsgDup, *PDbgPrnHk_MsgDup;

typedef struct _DbgPrnHk_GetMessages_USER_OUT {
    LARGE_INTEGER TimeStamp;
    PVOID         ProcessId;
    PVOID         ThreadId;
    USHORT        Length;
    UCHAR         Irql;
    UCHAR         CpuNumber;
    //
    union {
        ULONG Flags;
        struct {
            ULONG         TimeStampType:2;
            ULONG         CallerMode:2;
            ULONG         DataType:2;
            ULONG         WithSF:1;
            ULONG         Reserved0:1;
            ULONG         Reserved:24;
        };
    };
    /*
    UCHAR         TimeStampType:2;
    UCHAR         CallerMode:1;
    UCHAR         DataType:2;
    UCHAR         Reserved0:3;
    UCHAR         Reserved[3];
    //
    */
    union {
        ULONG            StackFramePtr;
        CHAR             Msg[MAX_MSG_SIZE];
        DbgPrnHk_MsgDup  MsgDup;
    };
} DbgPrnHk_GetMessages_USER_OUT, *PDbgPrnHk_GetMessages_USER_OUT;

typedef struct _DbgPrnHk_SetMessageBuffer_USER_IN {
    ULONG BufferSize;
} DbgPrnHk_SetMessageBuffer_USER_IN, *PDbgPrnHk_SetMessageBuffer_USER_IN;

typedef struct _DbgPrnHk_GetVersion_USER_OUT {
    ULONG         Major;
    ULONG         Minor;
    ULONG         Sub;
} DbgPrnHk_GetVersion_USER_OUT, *PDbgPrnHk_GetVersion_USER_OUT;

typedef struct _DbgPrnHk_GetRdtscCalibration_USER_IN {
    BOOLEAN       Recalibrate;
    BOOLEAN       ChangeType;
    UCHAR         TimeStampType;  // valid if ChangeType == TRUE
} DbgPrnHk_GetRdtscCalibration_USER_IN, *PDbgPrnHk_GetRdtscCalibration_USER_IN;

typedef struct _DbgPrnHk_GetRdtscCalibration_USER_OUT {
    __int64       RdtscTimeStampCalibration0;
    __int64       SysTimeStampCalibration0;
    __int64       RdtscTimeStampCalibration;
    __int64       SysTimeStampCalibration;
    UCHAR         TimeStampType;
} DbgPrnHk_GetRdtscCalibration_USER_OUT, *PDbgPrnHk_GetRdtscCalibration_USER_OUT;

typedef DbgPrnHk_GetMessages_USER_OUT  DbgPrnHk_PostMessageEx_USER_IN;
typedef PDbgPrnHk_GetMessages_USER_OUT PDbgPrnHk_PostMessageEx_USER_IN;

#ifndef USER_MODE

// Function declarations
extern NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath);
extern NTSTATUS DbgPrnHkCreateClose(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
extern NTSTATUS DbgPrnHkDeviceControl(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
extern VOID     DbgPrnHkUnload(IN PDRIVER_OBJECT DriverObject);

#endif //USER_MODE

/* driver registry options */

#define NT_DBGPRNHK_REG_CHECK_IRQL_VAL       L"CheckIrql"
#define NT_DBGPRNHK_REG_BUFFER_SIZE_VAL      L"BufferSize"
#define NT_DBGPRNHK_REG_BLOCK_MSG_VAL        L"DoNotPassMessagesDown"
#define NT_DBGPRNHK_REG_STOP_OVERFLOW_VAL    L"StopOnBufferOverflow"
#define NT_DBGPRNHK_REG_TSTAMP_TYPE_VAL      L"TimeStampType"
#define NT_DBGPRNHK_REG_AGGREGATE_MSG_VAL    L"AggregateMessages"
#define NT_DBGPRNHK_REG_SUPPRESS_DUPS_VAL    L"SuppressDuplicates"
#define NT_DBGPRNHK_REG_HAL_DISPLAY_VAL      L"DumpToHalDisplay"
#define NT_DBGPRNHK_REG_STACK_PTR_VAL        L"DumpStackFramePtr"

#define NT_DBGPRNHK_REG_KM_SAVING_VAL        L"KMSaving"
#define NT_DBGPRNHK_REG_KMS_OWN_DEV_DRV_VAL  L"KMSOwnDevDriver"
#define NT_DBGPRNHK_REG_KMS_TARGET_TYPE_VAL  L"KMSTargetType"     // block-dev, file, serial
#define NT_DBGPRNHK_REG_KMS_TARGET_FILE_VAL  L"KMSTargetFile"     // \device\floppy0, \device\harddisk0\partition1\log.txt, etc.
#define NT_DBGPRNHK_REG_KMS_TARGET_IP_VAL    L"KMSTargetIP"       // 10.0.0.1
#define NT_DBGPRNHK_REG_KMS_TARGET_PORT_VAL  L"KMSTargetPort"
#define NT_DBGPRNHK_REG_KMS_LOCAL_IP_VAL     L"KMSLocalIP"        // 10.0.0.2
#define NT_DBGPRNHK_REG_KMS_LOCAL_PORT_VAL   L"KMSLocalPort"
#define NT_DBGPRNHK_REG_KMS_PCI_ADDR_VAL     L"KMSPCIAddr"        // ULONG with Bus/Dev/Func
#define NT_DBGPRNHK_REG_KMS_STREAM_TYPE_VAL  L"KMSStreamType"     // stream, loop
#define NT_DBGPRNHK_REG_KMS_BLOCK_SIZE_VAL   L"KMSBlockSize"      // 256, 512, 1024, 2048, etc.

/*
typedef struct _DBGPRINT_HOOK_OPTIONS {
    PWCHAR OptionName;
    ULONG  OptionType;
    ULONG  UpdateFlag;
    union {
        ULONG OptDword;
        PWCHAR OptStr;
    } DefValue;
    union {
        ULONG OptDword;
        PWCHAR OptStr;
    } Value;
} DBGPRINT_HOOK_OPTIONS, *PDBGPRINT_HOOK_OPTIONS;
*/
//extern DBGPRINT_HOOK_OPTIONS DbgPrintHook_Options[];

typedef struct _DBGPRNHK_INTERNAL_STATE {

    DbgPrnHk_GetVersion_USER_OUT   Ver;

    PDbgPrnHk_GetMessages_USER_OUT MsgBuffer;
    LARGE_INTEGER ReadPosition;
    LARGE_INTEGER WritePosition;
    ULONG   BufferSize;
    ULONG   BufferSizeMask;
    ULONG   QueueSize;
    ULONG   MaxQueueSize;
    BOOLEAN CheckIrql;
    UCHAR   DoNotPassMessagesDown;
    UCHAR   StopOnBufferOverflow;
    BOOLEAN LoggingPaused;
    UCHAR TimeStampType;
    BOOLEAN BugCheckRegistered;
    BOOLEAN AggregateMessages;
    BOOLEAN SuppressDuplicates;
    ULONG   MaxPage;
    BOOLEAN DumpToHalDisplay;
    BOOLEAN DumpStackFramePtr;
    //
    BOOLEAN KdDebuggerEnabled;
    BOOLEAN KdDebuggerNotPresent;
#ifndef USER_MODE
    PMDL    BufferMdl;
#else
    PVOID   BufferMdl;
#endif
    struct {
        BOOLEAN Enabled;
        BOOLEAN KMSOwnDevDriver;
        UCHAR   Reserved[2];
        ULONG   KMSTargetType;
        PWCHAR  KMSTargetFile;
        PWCHAR  KMSTargetIP;
        ULONG   KMSTargetPort;
        PWCHAR  KMSLocalIP;
        ULONG   KMSLocalPort;
        ULONG   KMSPCIAddr;
        ULONG   KMSStreamType;
        ULONG   KMSBlockSize;
    } KMSaving;

    PCHAR  MsgBuffers[32];
    ULONG  MsgBuffersSz[32];

    BOOLEAN ContiguousMemory;

} DBGPRNHK_INTERNAL_STATE, *PDBGPRNHK_INTERNAL_STATE;

#define DBGPRN_KMSAVING_TARGET_TYPE_BLOCK    0x01
#define DBGPRN_KMSAVING_TARGET_TYPE_FILE     0x02
#define DBGPRN_KMSAVING_TARGET_TYPE_SERIAL   0x03

typedef union _DBGPRNHK_SIGNATURE_PAGE {
    UCHAR    _Bytes_[PAGE_SIZE];
    struct {
        UCHAR                    Signature[PAGE_SIZE/2];
        PDBGPRNHK_INTERNAL_STATE st;
        ULONG                    state;
#ifndef USER_MODE
        PHYSICAL_ADDRESS         ph_st;
        PHYSICAL_ADDRESS         ph_buf;
        PHYSICAL_ADDRESS         ph_buf_descr;
#else
        LARGE_INTEGER            ph_st;
        LARGE_INTEGER            ph_buf;
        LARGE_INTEGER            ph_buf_descr;
#endif
        PULONG                   buf_descr;
    };
} DBGPRNHK_SIGNATURE_PAGE, *PDBGPRNHK_SIGNATURE_PAGE;

#define DBGPRNHK_SIGNATURE      "*SIG:DbgPrnHk*\n"

#define MIN_SECTOR_SIZE     512

typedef struct _DBGPRNHK_ATA_LAYOUT {
    LARGE_INTEGER lba;
    ULONG         bcount;
    ULONG         flags;
} DBGPRNHK_ATA_LAYOUT, *PDBGPRNHK_ATA_LAYOUT;

#define DBGPRNHK_ATA_LAYOUT_FLAG_LAST       0x80000000
#define DBGPRNHK_ATA_LAYOUT_FLAG_NEXT_DESCR 0x00000001

typedef union _DBGPRNHK_ATA_SIGNATURE_BLOCK {
    UCHAR    _Bytes_[MIN_SECTOR_SIZE];
    struct {
        UCHAR                    Signature[MIN_SECTOR_SIZE/2];
        DBGPRNHK_ATA_LAYOUT      Layout[1];
    };
} DBGPRNHK_ATA_SIGNATURE_BLOCK, *PDBGPRNHK_ATA_SIGNATURE_BLOCK;

#define DBGPRNHK_ATA_DESCR_SIGNATURE  "*SIG:DbgPrnHk-ATA-Layout-Descr*\n"
#define DBGPRNHK_ATA_DATA_SIGNATURE   "*SIG:DbgPrnHk-ATA-[Data-Block]*\n"

#pragma pack(pop)

};

#endif // __DEBUG_PRINT_HOOK_H__

