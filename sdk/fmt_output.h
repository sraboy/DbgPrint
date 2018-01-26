#ifndef __DEBUG_PRINT_FORMAT_MESSAGES_H__
#define __DEBUG_PRINT_FORMAT_MESSAGES_H__

extern "C" {

#pragma pack(push, 8)

typedef struct _DBGPRN_FORMAT_MESSAGE_CTX {
    BOOLEAN          new_line;
    HANDLE           prev_ProcessId;
    HANDLE           prev_ThreadId;
    ULONG            syslog_hrd_len;
} DBGPRN_FORMAT_MESSAGE_CTX, *PDBGPRN_FORMAT_MESSAGE_CTX;

typedef struct _DBGPRN_TIMESTAMP_CTX {
    DbgPrnHk_GetRdtscCalibration_USER_OUT CalibrationBuffer;
    ULONGLONG        FtmSysTime;
    ULONGLONG        UtcSysTime;
    LONGLONG         RelPerfCounter;
    ULONGLONG        Frequency;
    double           RdtscCalibration;
} DBGPRN_TIMESTAMP_CTX, *PDBGPRN_TIMESTAMP_CTX;

void
dbgprint_format_msg_init(
    PDBGDUMP_OPT_RAW opt,
    PDBGPRN_FORMAT_MESSAGE_CTX msgst
    );

ULONG
dbgprint_format_msg(
    PDBGDUMP_OPT_RAW opt,
    PDbgPrnHk_GetMessages_USER_OUT CurMsg,
    PCHAR buf,
    PDBGPRN_TIMESTAMP_CTX      timest,
    PDBGPRN_FORMAT_MESSAGE_CTX msgst
    );

void
dbgprint_timestamp_init(
    PDBGDUMP_OPT_RAW           opt,
    HANDLE                     h_drv,
    PDBGPRN_TIMESTAMP_CTX      timest
    );

void
dbgprint_timestamp_resync(
    PDBGDUMP_OPT_RAW           opt,
    HANDLE                     h_drv,
    PDBGPRN_TIMESTAMP_CTX      timest
    );

#pragma pack(pop)

};

#endif //__DEBUG_PRINT_FORMAT_MESSAGES_H__
