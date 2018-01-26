#include "windows.h"
#include "..\PostDbgMesg.h"

void main()
{
    int i;
    
    DbgDump_Printf("printf test: %d=10\n", 10);
    DbgDump_Print ("print test: 10=10\n");
    DbgDump_Printn ("print test: 10=10\n" "must not be displayed", sizeof("print test: 10=10\n")-1);

    DbgDump_Printf("begin overflow test\n");
    for(i=0; i<1024*1024; i++) {
        DbgDump_Printf("overflow test: %d\n", i);
    }
    DbgDump_Printf("end overflow test\n");
}