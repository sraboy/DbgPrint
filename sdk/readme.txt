This SDK simplifies communications with DbgDump tools.
You can use either Debug (PostDbgMesgD.lib) or Release (PostDbgMesg.lib)
static library in you project to post debug messages directly to
DbgPrnHk driver. Posted messages are stored in driver's internal buffer
and can be later obtained by DbgPrintLog.exe either in service mode or
console mode.

It is useful for capturing logs when Service Manager or Win32 subsystem
is unavailable.

Include PostDbgMesg.h and PostDbgMesg.lib into your project.
After that you will be able to use DbgDump_Printf(), DbgDump_Print()
and DbgDump_Printn() API.

DbgDump_Printf() is similar to standard printf(). It uses format string
as first operand. Note, total length of final string is limited to 2047 
characters.

DbgDump_Print() simply prints ASCIIZ string.

DbgDump_Printn() prints specified number of bytes.

Find sample in sdk\test\
