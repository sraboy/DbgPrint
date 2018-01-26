#ifndef __DBG_PRINT_LOG__H__
#define __DBG_PRINT_LOG__H__

#define NT_DbgPrnHk_Client_SVC_NAME     "DbgPrintLog"
#define NT_DbgPrnHk_Reg_CmdLineW        L"CmdLine"
#define NT_DbgPrnHk_Reg_WDirW           L"WorkingDirectory"
#define NT_DbgPrnHk_Reg_ImagePathW      L"ImagePath"

typedef struct _DBG_OUTPUT_DEBUG_STRING_BUFFER {
    ULONG ProcessId;
    UCHAR Msg[4096-sizeof(ULONG)];
} DBG_OUTPUT_DEBUG_STRING_BUFFER, *PDBG_OUTPUT_DEBUG_STRING_BUFFER;

#define IDX_VER_MJ    0
#define IDX_VER_MN    0
#define IDX_VER_SUB   ""
#define IDX_VER_NSUB  0

typedef struct _DBG_PRINT_LOG_IDX_HEADER {
    ULONG         Length;
    ULONG         Major;
    ULONG         Minor;
    ULONG         Sub;

    ULONG         IdxItemSize;
    ULONG         Reserved0[3];

    BOOLEAN log_pid      ;
    BOOLEAN log_tid      ;
    BOOLEAN log_mode     ;
    BOOLEAN log_irql     ;
    BOOLEAN log_time_ex  ;
    BOOLEAN log_time_rel ;
    BOOLEAN log_time_date;
    BOOLEAN log_time_time;

    BOOLEAN skip_kernel  ;
    BOOLEAN skip_umode   ;

    BOOLEAN log_time_nano;

    BOOLEAN Reserved[1+4]; // 8-byte padding

} DBG_PRINT_LOG_IDX_HEADER, *PDBG_PRINT_LOG_IDX_HEADER;

typedef struct _DBG_PRINT_LOG_IDX_ITEM {
    ULONG Offset;
//    ULONG Reserved[3];
} DBG_PRINT_LOG_IDX_ITEM, *PDBG_PRINT_LOG_IDX_ITEM;

// print_log()
#define print_log  printf

#endif //__DBG_PRINT_LOG__H__
