#ifndef __DBG_PRINT_INTERNAL__OPTIONS__H__
#define __DBG_PRINT_INTERNAL__OPTIONS__H__

#pragma pack(push, 1)

typedef struct _DBGDUMP_OPT {

    // generate member definitions
    #define DBGPRINT_OPT_STRUCT
    #include "dbgprint_opt_list.h"

} DBGDUMP_OPT, *PDBGDUMP_OPT;

typedef struct _DBGDUMP_OPT_RAW {

    // generate member definitions
    #define DBGPRINT_OPT_RAW_STRUCT
    #include "dbgprint_opt_list.h"

} DBGDUMP_OPT_RAW, *PDBGDUMP_OPT_RAW;

#pragma pack(pop)

#define DbgPrint_OptStruct_Reinit_Drv  1
#define DbgPrint_OptStruct_Reinit_All  2

int
DbgPrint_CmdLine_to_OptStruct(
    IN PWCHAR CmdLine,
    IN BOOLEAN Reinit,
    OUT PDBGDUMP_OPT opt_struct
    );

VOID
DbgPrint_Cleanup_OptStruct(
    OUT PDBGDUMP_OPT opt_struct
    );

#endif //__DBG_PRINT_INTERNAL__OPTIONS__H__