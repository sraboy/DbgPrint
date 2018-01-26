/*++

Copyright (c) 2004-2011 Alexander A. Telyatnikov (Alter)

Module Name:
    DbgPrintLog.cpp

Abstract:

Authors:
    Alexander A. Telyatnikov (Alter)
    + thanks to Max Sivkov for Service Mode

Environment:
    User mode only

Notes:

    THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
    OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
    IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
    NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
    THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Revision History:

--*/

//#define WIN32_LEAN_AND_MEAN

#include <windows.h>
//#include <winsock2.h>
#include <stdio.h>
#include "DbgPrnHk.h"
#include "SvcManLib.h"
#include "Privilege.h"
#include "flusher.h"
#include "DbgPrintLog.h"
#include "dbgprint_opt.h"
#include "..\sdk\postmsg\PostDbgMesg.h"
#include "..\sdk\formatmsg\fmt_output.h"
#include "..\sdk\kdapis\kdapis.h"
#include <Sddl.h>
#include "tools.h"

//#undef print_log
//#define print_log     print_err
#define KdPrint(x)    print_log x

//#include "dbgprn_options.h"

#define REG_CPU_DESCR_KEY_NAME_W  L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\%u"
#define REG_CPU_MHZ_VAL_NAME_W    L"~MHz"
#define REG_CPU_ID_VAL_NAME_W     L"Identifier"
#define REG_CPU_VENDOR_VAL_NAME_W L"VendorIdentifier"

#define REG_KDPRINT_DEFAULT_MASK  L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Debug Print Filter"
#define REG_KDPRINT_DEFAULT_MASK_VAL_NAME_W L"Default"

#define ENV_VALUE_COM_PORT_NAME   "_NT_DEBUG_PORT"
#define ENV_VALUE_COM_BAUD_NAME   "_NTKD_BAUD_RATE"

BOOLEAN
get_drv_conf();

BOOLEAN
set_drv_conf();

void
WINAPI
service_main(
    DWORD dwArgc,
    LPTSTR *lpszArgv);

VOID AddToMessageLog(LPTSTR lpszMsg);

BOOL ReportStatusToSCMgr(DWORD dwCurrentState,
                         DWORD dwWin32ExitCode,
                         DWORD dwWaitHint);

SERVICE_TABLE_ENTRY dispatchTable[] =
{
    { NT_DbgPrnHk_Client_SVC_NAME, (LPSERVICE_MAIN_FUNCTION)service_main },
    { NULL, NULL }
};

WCHAR IdxFileN[MAX_PATH*2];
WCHAR LogFileN[MAX_PATH*2];
WCHAR LogFileSuf[MAX_PATH];

CHAR PathToExec[MAX_PATH] = ".";

BOOLEAN esc_key       = FALSE;
BOOLEAN pause_key     = FALSE;
BOOLEAN next_log      = FALSE;
BOOLEAN flush_file    = FALSE;
BOOLEAN sys_account   = FALSE;
BOOLEAN wait_int3     = FALSE;
WCHAR   int3_key      = 0;

DBGDUMP_OPT_RAW g_opt;
PWCHAR g_CmdLine = L"";
#define pOpt     (&g_opt)

WSADATA g_WsaData;
BOOLEAN g_Wsa_init_ok = FALSE;

DBGPRNHK_INTERNAL_STATE st;

ULONG g_max_msg = 0;

/*
WCHAR  WDir[MAX_PATH*2];
PWCHAR LogFile = L"DbgPrint.log";  // default log-file name

BOOLEAN log_pid       = FALSE;
BOOLEAN log_tid       = FALSE;
BOOLEAN log_mode      = FALSE;
BOOLEAN log_irql      = FALSE;
BOOLEAN log_cpu       = FALSE;
//BOOLEAN log_time_tick = FALSE;

BOOLEAN log_time_ext  = TRUE;
BOOLEAN log_time_utc  = FALSE;
BOOLEAN log_time_perf = FALSE;

BOOLEAN log_time_date = FALSE;
BOOLEAN log_time_time = TRUE;
BOOLEAN log_time_nano = FALSE;

BOOLEAN skip_kernel   = FALSE;
BOOLEAN skip_umode    = FALSE;

//BOOLEAN stdout_mode   = FALSE;
//BOOLEAN stdout_copy   = FALSE;

BOOLEAN output_stdout = FALSE;
BOOLEAN output_file = TRUE;

//BOOLEAN change_config = FALSE;

ULONG FlushTimeout = 5;
BOOLEAN sync_mode  = FALSE;

ULONG MaxLogSize = 1024*1024*128; //128Mb
BOOLEAN prealloc_mode = FALSE;
BOOLEAN use_index       = FALSE;
ULONG MaxLogs = -1;
ULONG StartLogNum = 0;

BOOLEAN install_svc   = FALSE;
BOOLEAN install_drv   = FALSE;
BOOLEAN drv_very_first= FALSE;
BOOLEAN restart_drv   = FALSE;

ULONG   drv_mode      = SERVICE_DEMAND_START;
ULONG   svc_mode      = SERVICE_AUTO_START;

BOOLEAN deinstall     = FALSE;

BOOLEAN drv_opt       = FALSE;
*/

PDbgPrnHk_GetMessages_USER_OUT MsgBuffer = NULL;

// dbg messages capture buffers for user-mode
// (via OutputDebugString)
// '2' suffix  - buffer/counter for reading from get_messages()
// '2a' suffix - buffer/counter for writing from get_ODS_messages()
HANDLE hODS_lock = NULL;
ULONG MsgCount2 = 0;
ULONG MsgCount2a = 0;
PDbgPrnHk_GetMessages_USER_OUT MsgBuffer2 = NULL;
PDbgPrnHk_GetMessages_USER_OUT MsgBuffer2a = NULL;

ULONG BufferSize = 1024*1024;

ULONG LineNumber;

HANDLE StdIn  = NULL;
HANDLE StdOut = NULL;

HANDLE hODSreadyEvent = NULL;
HANDLE hKDreadyEvent = NULL;
HANDLE hDRVreadyEvent = NULL;
HANDLE hSYSLOGreadyEvent = NULL;

SERVICE_STATUS          ssStatus;       // current status of the service
SERVICE_STATUS_HANDLE   sshStatusHandle;

WCHAR UserName[MAX_PATH];

#define NEWSTATE        kd.StateChange.NewState
#define EXCEPTION_CODE  kd.StateChange.u.Exception.ExceptionRecord.ExceptionCode
#define FIRST_CHANCE    kd.StateChange.u.Exception.FirstChance
#define EXCEPTIONPC     (ULONG)kd.StateChange.ProgramCounter

#define EXCEPTIONREPORT kd.StateChange.ControlReport
#ifdef  i386
#define EXCEPTIONDR7    kd.StateChange.ControlReport.Dr7
#endif
#define INSTRCOUNT      kd.StateChange.ControlReport.InstructionCount
#define INSTRSTREAM     kd.StateChange.ControlReport.InstructionStream

struct {
    DBGKD_WAIT_STATE_CHANGE StateChange;
    char Buffer[256];

    //PUCHAR      Switch;

//    int         Index;
    DBGKD_CONTROL_SET ControlSet;

    DBGKD_GET_VERSION VersionBuffer;

    BOOLEAN Connected;
} kd;

static const char help_text2_std[] = 
    "Commands:\n"
    "\t'Esc'   - exit\n"
    "\t'N'     - start new log\n"
    "\t'F'     - flush log buffer\n"
    "\t'H'     - display this help message\n"
    "Mode switchers (toggle on'off):\n"
    "\t'Space' - pause                    '?'  - get current options\n"
    "\t'S'     - synchronous mode         'C'  - copy log to stdout\n"

    "\t'K'     - capture kernel messages  'U'  - capture user-mode\n"
    "\t'I'     - check IRQL               'P'  - pass down\n"
    "\t'O'     - stop on overflow         'A'  - aggregate\n"
    ;

static const char help_text2_proxy[] = 
    "Commands:\n"
    "\t'Esc'   - exit and stop user-mode capturing\n"
    "\t'H'     - display this help message\n"
//    "Mode switchers (toggle on'off):\n"
//    "\t'Space' - pause user-mode capturing\n"
//    "\t'C'     - copy log to stdout\n"
//    "\t'U'     - capture user-mode messages\n"
    "All debug messages are now routed to special memory buffer.\n"
    "You can use WinDbg with Dbgprn extension to extract logs from crash-dump.\n"
    "In-memory log capturing is active...\n"
    ;

char* help_text2 = (char*)help_text2_std;

extern "C"
BOOLEAN
_cdecl
print_err(
    PCHAR Format,
    ...
    )
{
    //ULONG returned;
    CHAR buff[2048];
    va_list ap;
    va_start(ap, Format);
    ULONG len;

    OutputDebugString("***********************\n");

    vprintf(Format, ap);
    len = _vsnprintf(buff, sizeof(buff), Format, ap);
    buff[sizeof(buff)-1] = 0;

    OutputDebugString(buff);

    va_end(ap);
    return TRUE;
} // end _print_err()




BOOLEAN
IsSystemAccount()
{
/*
    WCHAR UserName[MAX_PATH];
    ULONG len = MAX_PATH-1;

    if(!GetUserNameW(UserName, &len)) {
        UserName[0] = 0;
        return FALSE;
    }
    UserName[len] = 0;
    if(!wcsicmp(UserName, L"System")) {
        return TRUE;
    }
    return FALSE;
*/

    HANDLE              hToken;
    PTOKEN_USER         tu;
    DWORD               ReturnLength;
//    DWORD               i, j;
//    PCHAR               StrSid;
    PUCHAR              SubAuthCount;
    PDWORD              pRID;

    if(g_opt.user_run == 1) {
        return FALSE;
    }
    if(g_opt.user_run == 2) {
        return TRUE;
    }

    // obtain the token, first check the thread and then the process
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        return FALSE;
    }

    GetTokenInformation(hToken, TokenUser, NULL, 0L, &ReturnLength);
    tu = (PTOKEN_USER)GlobalAlloc(GMEM_FIXED, ReturnLength);
    if(!GetTokenInformation(hToken, TokenUser, tu, ReturnLength, &ReturnLength)) {
        return FALSE;
    }
/*
    if(!ConvertSidToStringSid(tu->User.Sid, &StrSid)) {
        return FALSE;
    }
*/
    SubAuthCount = GetSidSubAuthorityCount(tu->User.Sid);
    if((*SubAuthCount) != 1)
        return FALSE;

    pRID = GetSidSubAuthority(tu->User.Sid, 0);
    if((*pRID) != SECURITY_LOCAL_SYSTEM_RID)
        return FALSE;
/*
    j = (*SubAuthCount);
    for(i=0; i<j; i++) {
        pRID = GetSidSubAuthority(tu->User.Sid, i);

        printf("i=%x, RID=%x\n", i, *pRID);
        sprintf((PCHAR)UserName, "i=%x, RID=%x\n", i, *pRID);
        OutputDebugString((PCHAR)UserName);
        continue;

        if((*pRID) != SECURITY_LOGON_IDS_RID ||
            i != 0) {
            return FALSE;
        }
        if((*pRID) != SECURITY_LOCAL_SYSTEM_RID ||
            i != 1) {
            return FALSE;
        }
    }

    Sleep(10000);
    ExitProcess(0);
*/

    return TRUE;

} // end IsSystemAccount()

VOID
TeeOutput(
    HANDLE h,
    PCHAR Buffer,
    ULONG Length
    )
{
    if(g_opt.output_file) {
        WriteFileBuffer(h, Buffer, Length);
    }
    if(g_opt.output_stdout) {
        WriteConsole(StdOut, Buffer, Length, &Length, NULL);
    }
/*    if(g_opt.output_syslog) {
        sen(h, Buffer, Length);
    }*/
} // end TeeOutput()

#define TeeOutputConst(h, str)  TeeOutput(h, str, sizeof(str)-1)

DBG_PRINT_LOG_IDX_HEADER IdxHeader;
DBG_PRINT_LOG_IDX_ITEM   IdxItem;

VOID
WriteLineNumber(
    HANDLE h,
    ULONG  Offset
    )
{
    ULONG len;

    memset(&IdxItem, 0, sizeof(IdxItem));
    IdxItem.Offset = Offset;
    WriteFile(h, &IdxItem, sizeof(IdxItem), &len, NULL);
} // end WriteLineNumber()

#include "dump_state.h"

/*
 Catch keyboard messages
*/
DWORD
__stdcall
get_kbd(PVOID ctx)
{
    ULONG key = 0;
    ULONG tmp;
    INPUT_RECORD inp;
    BOOLEAN output_stdout = g_opt.output_stdout;

    if(!StdIn)
        return -1;
    while(!esc_key) {
        if(GetNumberOfConsoleInputEvents(StdIn, &key) && key) {
            key = ReadConsoleInput(StdIn, &inp, 1, &tmp);
            if(key &&
               inp.EventType == KEY_EVENT &&
               inp.Event.KeyEvent.bKeyDown) {

                if(inp.Event.KeyEvent.wVirtualScanCode == 0x01) { /*VK_ESC*/
                    esc_key = TRUE;
                    break;
                }

                if(wait_int3) {
                    int3_key = inp.Event.KeyEvent.wVirtualScanCode;
                    continue;
                }

                switch(inp.Event.KeyEvent.wVirtualScanCode) {
                case 0x01: /*VK_ESC*/
                    esc_key = TRUE;
                    break;
                case 0x39: /*VK_SPACE*/
                    pause_key = !pause_key;
                    print_log("Log %s\n", !pause_key ? "restarted" : "paused");
                    break;
                case 0x2e: /*VK_C*/
                    if(output_stdout)
                        break;
                    g_opt.output_stdout = !g_opt.output_stdout;
                    break;
                case 0x31: /*VK_N*/
                    if(g_opt.output_file)
                        next_log = TRUE;
                    break;
                case 0x35: /*VK_QM*/
                    if(get_drv_conf()) {
                        dp_dump_st(&st);
                    }
                    break;
                case 0x1f: /*VK_S*/
                    g_opt.sync_mode = !g_opt.sync_mode;
                    break;
                case 0x21: /*VK_F*/
                    flush_file = TRUE;
                    break;
                case 0x23: /*VK_H*/
                    print_log(help_text2);
                    break;
                case 0x25: /*VK_K*/
                    g_opt.skip_kernel = !g_opt.skip_kernel;
                    print_log("kernel messages %s\n", !g_opt.skip_kernel ? "on" : "off");
                    break;
                case 0x16: /*VK_U*/
                    g_opt.skip_umode = !g_opt.skip_umode;
                    print_log("user-mode messages %s\n", !g_opt.skip_umode ? "on" : "off");
                    break;

                case 0x1e: /*VK_A*/
                case 0x17: /*VK_I*/
                case 0x18: /*VK_O*/
                case 0x19: /*VK_P*/
                    if(get_drv_conf()) {
                        switch(inp.Event.KeyEvent.wVirtualScanCode) {
                        case 0x1e: /*VK_A*/
                            st.AggregateMessages = !st.AggregateMessages;
                            break;
                        case 0x17: /*VK_I*/
                            st.CheckIrql = !st.CheckIrql;
                            break;
                        case 0x18: /*VK_O*/
                            st.StopOnBufferOverflow++;
                            if(st.StopOnBufferOverflow > 2) {
                                st.StopOnBufferOverflow=0;
                            }
                            break;
                        case 0x19: /*VK_P*/
                            st.DoNotPassMessagesDown = !st.DoNotPassMessagesDown;
                            break;
                        }
                        if(set_drv_conf()) {
                            switch(inp.Event.KeyEvent.wVirtualScanCode) {
                            case 0x1e: /*VK_A*/
                                print_log("AggregateMessages %s\n", st.AggregateMessages ? "on" : "off");
                                break;
                            case 0x17: /*VK_I*/
                                print_log("CheckIrql %s\n", st.CheckIrql ? "on" : "off");
                                break;
                            case 0x18: /*VK_O*/
                                print_log("StopOnBufferOverflow   = ");
                                switch(st.StopOnBufferOverflow) {
                                case BufferOverflow_Continue:
                                    print_log("Continue\n");
                                    break;
                                case BufferOverflow_Stop:
                                    print_log("Stop\n");
                                    break;
                                case BufferOverflow_CallDebugger:
                                    print_log("CallDebugger\n");
                                    break;
                                }
                                break;
                            case 0x19: /*VK_P*/
                                print_log("DoNotPassMessagesDown %s\n", st.DoNotPassMessagesDown ? "on" : "off");
                                break;
                            }
                        }
                    }
                    break;

                }
            }
        }
        Sleep(100);
    }
    return 0;
} // end get_kbd()


/*
 Catch strings from OutputDebugString()
*/
DWORD
__stdcall
get_ODS_messages(PVOID ctx)
{
    HANDLE hCompleteEvent;
    HANDLE hNewMsgEvent;
    HANDLE hdbgf;
    PDBG_OUTPUT_DEBUG_STRING_BUFFER ODSmem;
    ULONG j = 0;
    ULONG status;
    ULONG len, idx;
    PCHAR sptr;
    BOOLEAN all_copied;

    SECURITY_ATTRIBUTES SecAttr;
    SECURITY_DESCRIPTOR SecDesc;

    PDbgPrnHk_PostMessageEx_USER_IN pPutMsgBuf;

    //print_err("get_ODS_messages (%#x)\n", ctx);

    // Connect to user-mode logger
    memset(&SecAttr, 0, sizeof(SecAttr));
    memset(&SecDesc, 0, sizeof(SecDesc));
    SecAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    SecAttr.bInheritHandle = TRUE;
    SecAttr.lpSecurityDescriptor = &SecDesc;

    //print_err("get_ODS_messages 1\n");

    if(!InitializeSecurityDescriptor(&SecDesc, SECURITY_DESCRIPTOR_REVISION)) {
        print_err("can't init SecurityDescriptor, (%#x)\n",
            GetLastError());
        return -1;
    }

    if(!SetSecurityDescriptorDacl(&SecDesc, TRUE, NULL, FALSE)) {
        print_err("can't set SD Dacl, (%#x)\n",
            GetLastError());
        return -1;
    }

    //print_err("get_ODS_messages 2\n");
    hNewMsgEvent = CreateEvent(&SecAttr, FALSE, FALSE, "DBWIN_DATA_READY");

    if(!hNewMsgEvent) {
        print_err("can't create sync event, (%#x)\n",
                GetLastError());
        return -1;
    }

    //print_err("get_ODS_messages 3\n");
    hCompleteEvent = CreateEvent(&SecAttr, FALSE, FALSE, "DBWIN_BUFFER_READY");

    if(!hCompleteEvent) {
        print_err("can't create sync event (2), (%#x)\n",
                GetLastError());
        return -1;
    }

    hdbgf = CreateFileMapping((HANDLE)(-1),
                        &SecAttr, PAGE_READWRITE,
                        0, 4096,
                        "DBWIN_BUFFER");

    //print_err("get_ODS_messages 4\n");
    if (!hdbgf) {
        print_err("can't create file mapping, (%#x)\n",
                GetLastError());
        return -1;
    }

    ODSmem = (PDBG_OUTPUT_DEBUG_STRING_BUFFER)MapViewOfFile(hdbgf, FILE_MAP_READ, 0, 0, 4096);
    //print_err("get_ODS_messages 5\n");

    if (!ODSmem) {
        print_err("can't map shared mem (%#x)\n",
                GetLastError());
        return -1;
    }

    // send Ready message
    SetEvent(hCompleteEvent);

    // inform get_messages() that ODS capturing is started
    SetEvent(hODSreadyEvent);

    //print_err("get_ODS_messages 6\n");

    if(!g_opt.without_driver) {
        pPutMsgBuf = (PDbgPrnHk_PostMessageEx_USER_IN)GlobalAlloc(GMEM_FIXED, sizeof(DbgPrnHk_PostMessageEx_USER_IN)+2048);
        if(!pPutMsgBuf) {
            print_err("can't alloc PostMessageEx buffer (%#x)\n",
                    GetLastError());
            return -1;
        }
        memset(pPutMsgBuf, 0, sizeof(DbgPrnHk_PostMessageEx_USER_IN));
        //pPutMsgBuf->TimeStamp.QuadPart = 0; //already 0
        pPutMsgBuf->ThreadId = (PVOID)(-1);
        //pPutMsgBuf->Irql = 0;             //already 0
        pPutMsgBuf->CpuNumber = 0xff;
        pPutMsgBuf->CallerMode = 1;
    }

    //print_err("get_ODS_messages loop 0\n");

    while(!esc_key) {
        //print_err("get_ODS_messages loop\n");
        // wait for incoming messages
        status = WaitForSingleObject(hNewMsgEvent, INFINITE);
        if(status == WAIT_OBJECT_0) {

            all_copied = FALSE;
            len = 0;

            if(g_opt.without_driver) {
retry:
                WaitForSingleObject(hODS_lock, INFINITE);

retry2:
                // copy message to buffer in unified format
                j = MsgCount2a;

                while(MsgCount2a && (MsgCount2a-1)*sizeof(DbgPrnHk_GetMessages_USER_OUT) >= BufferSize) {
                    ReleaseMutex(hODS_lock);
                    goto retry;
                };

                __try {
                    
                    idx = 0;
                    sptr = &(MsgBuffer2a[j].Msg[0]);

                    while(len < sizeof(ODSmem->Msg)) {

                        if(idx >= (sizeof(MsgBuffer2a[j].Msg)-1)) {
                            all_copied = FALSE;
                            break;
                        }
                        if(!(sptr[idx] = ODSmem->Msg[len])) {
                            all_copied = TRUE;
                            break;
                        }
                        len++;
                        idx++;
                    }
                    if(len >= sizeof(ODSmem->Msg)) {
                        all_copied = TRUE;
                    }
                    sptr[idx] = 0;

                    //strncpy(&(MsgBuffer2a[j].Msg[0]), (const char*)&(ODSmem->Msg[0]), sizeof(ODSmem->Msg));
                    //MsgBuffer2a[j].Msg[sizeof(ODSmem->Msg)-1] = 0;
                    //len = strlen(MsgBuffer2a[j].Msg);
                } __except(EXCEPTION_EXECUTE_HANDLER) {
                }
                QueryPerformanceCounter((PLARGE_INTEGER)&MsgBuffer2a[j].TimeStamp);
                MsgBuffer2a[j].Length = (USHORT)idx;
                MsgBuffer2a[j].ProcessId = (PVOID)(ODSmem->ProcessId);
                MsgBuffer2a[j].ThreadId = (PVOID)(-1);
                MsgBuffer2a[j].Irql = 0;
                MsgBuffer2a[j].CpuNumber = 0xff;
                MsgBuffer2a[j].CallerMode = 1;
                j++;
                MsgCount2a = j;

                if(!all_copied &&
                   ((MsgCount2a+1)*sizeof(DbgPrnHk_GetMessages_USER_OUT) <= BufferSize)) {
                    goto retry2;
                }

                ReleaseMutex(hODS_lock);

                if(!all_copied) {
                    goto retry;
                }
            } else {
                __try {
                    
                    PUCHAR sptr = ODSmem->Msg;
                    idx = 0;
                    while((idx < sizeof(ODSmem->Msg)) && sptr[idx]) {
                        idx++;
                    }
                    pPutMsgBuf->Length = (USHORT)idx;
                    pPutMsgBuf->ProcessId = (PVOID)(ODSmem->ProcessId);
                    memcpy(pPutMsgBuf->Msg, sptr, idx);

                    DbgDump_PostMsgEx(pPutMsgBuf);

                    //strncpy(&(MsgBuffer2a[j].Msg[0]), (const char*)&(ODSmem->Msg[0]), sizeof(ODSmem->Msg));
                    //MsgBuffer2a[j].Msg[sizeof(ODSmem->Msg)-1] = 0;
                    //len = strlen(MsgBuffer2a[j].Msg);
                } __except(EXCEPTION_EXECUTE_HANDLER) {
                }
            }

            SetEvent(hCompleteEvent);
        }
    }
    return 0;
} // end get_ODS_messages()

VOID
store_RAW_message(
    IN PDbgPrnHk_GetMessages_USER_OUT Msg
    )
{
        WaitForSingleObject(hODS_lock, INFINITE);

        // copy message to buffer in unified format

        if(MsgCount2a && (MsgCount2a-1)*sizeof(DbgPrnHk_GetMessages_USER_OUT) >= BufferSize) {
            ReleaseMutex(hODS_lock);
            // drop message on overflow
            return;
        };

        memcpy(&MsgBuffer2a[MsgCount2a], Msg, sizeof(DbgPrnHk_GetMessages_USER_OUT));
        MsgCount2a++;

        ReleaseMutex(hODS_lock);
} // end store_RAW_message()

VOID
store_KD_message(
    IN USHORT Processor,
    IN USHORT ProcessorLevel,
    IN PUCHAR String,
    IN USHORT StringLength
    )
{
    ULONG j;
    ULONG len, idx;
    PCHAR sptr;
    BOOLEAN all_copied;

    all_copied = FALSE;
    len = 0;
retry:
        WaitForSingleObject(hODS_lock, INFINITE);

retry2:
        // copy message to buffer in unified format
        j = MsgCount2a;

        if(MsgCount2a && (MsgCount2a-1)*sizeof(DbgPrnHk_GetMessages_USER_OUT) >= BufferSize) {
            ReleaseMutex(hODS_lock);
            // drop message on overflow
            return;
        };

        __try {
            
            idx = 0;
            sptr = &(MsgBuffer2a[j].Msg[0]);

            while(len < StringLength) {

                if(idx >= (sizeof(MsgBuffer2a[j].Msg)-1)) {
                    all_copied = FALSE;
                    break;
                }
                if(!(sptr[idx] = String[len])) {
                    all_copied = TRUE;
                    break;
                }
                len++;
                idx++;
            }
            if(len >= StringLength) {
                all_copied = TRUE;
            }
            sptr[idx] = 0;

            //strncpy(&(MsgBuffer2a[j].Msg[0]), (const char*)&(ODSmem->Msg[0]), sizeof(ODSmem->Msg));
            //MsgBuffer2a[j].Msg[sizeof(ODSmem->Msg)-1] = 0;
            //len = strlen(MsgBuffer2a[j].Msg);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
        }
        QueryPerformanceCounter((PLARGE_INTEGER)&MsgBuffer2a[j].TimeStamp);
        MsgBuffer2a[j].Length = (USHORT)idx;
        MsgBuffer2a[j].ProcessId = (PVOID)(-1);
        MsgBuffer2a[j].ThreadId = (PVOID)(-1);
        MsgBuffer2a[j].Irql = (UCHAR)ProcessorLevel;
        MsgBuffer2a[j].CpuNumber = (UCHAR)Processor;
        MsgBuffer2a[j].CallerMode = 0;
        j++;
        MsgCount2a = j;

        if(!all_copied &&
           ((MsgCount2a+1)*sizeof(DbgPrnHk_GetMessages_USER_OUT) <= BufferSize)) {
            goto retry2;
        }

        ReleaseMutex(hODS_lock);

        if(!all_copied) {
            goto retry;
        }
} // end store_KD_message()

/*
 Catch strings from KD()
*/
DWORD
__stdcall
get_KD_messages(PVOID ctx)
{

    DWORD       status;
    DWORD       status0;
    PUCHAR      pszExceptCode;
    ULONG       NtsdCurrentProcessor;
    CHAR        DbgPrintBuf[MAX_PATH];
    DWORD       len;

    // inform get_messages() that KD capturing is started
    SetEvent(hKDreadyEvent);

    while(!esc_key) {
        status = DbgKdWaitStateChange(&kd.StateChange, kd.Buffer, 254);

        if(status != ERROR_SUCCESS) {
            print_log("kd: DbgKdWaitStateChange failed: %08lx\n", status);
            esc_key = TRUE;
            return 0;
        }
        len = 0;
        NtsdCurrentProcessor = kd.StateChange.Processor;
        if(kd.StateChange.NewState == DbgKdExceptionStateChange) {

            if (EXCEPTION_CODE == EXCEPTION_BREAKPOINT
                    || EXCEPTION_CODE == EXCEPTION_SINGLE_STEP) {
                pszExceptCode = (PUCHAR)"BreakPoint";
            } else if (EXCEPTION_CODE == EXCEPTION_DATATYPE_MISALIGNMENT) {
                pszExceptCode = (PUCHAR)"Data Misaligned";
            } else if (EXCEPTION_CODE == EXCEPTION_INT_OVERFLOW) {
                pszExceptCode = (PUCHAR)"Integer Overflow";
            } else if (EXCEPTION_CODE == EXCEPTION_ACCESS_VIOLATION) {
                pszExceptCode = (PUCHAR)"Access Violation";
            } else {
                pszExceptCode = (PUCHAR)"Unknown Exception";
            }

            if (!pszExceptCode) {
                status = DBG_EXCEPTION_HANDLED;
            } else {
                print_log("%s - code: %08lx  (", pszExceptCode, EXCEPTION_CODE);
                status = DBG_EXCEPTION_HANDLED;
                if (FIRST_CHANCE) {
                    print_log("first");
                } else {
                    print_log("second");
                }
                print_log(" chance)\n");
            }

#ifdef  i386
            if (EXCEPTION_CODE == EXCEPTION_BREAKPOINT) {
                CONTEXT Registers;
                KSPECIAL_REGISTERS SpecialRegisters;
                if ( DbgKdGetContext(NtsdCurrentProcessor,&Registers) == ERROR_SUCCESS ) {
                    print_log("Breakpoint Occured at:\n");
                    print_log("eip = 0x%08x\n",Registers.Eip);
                    print_log("ebp = 0x%08x\n",Registers.Ebp);
                    print_log("esp = 0x%08x\n",Registers.Esp);
                    Registers.Eip++;
                    DbgKdSetContext(NtsdCurrentProcessor,&Registers);
                }
                if ( DbgKdReadControlSpace(
                        NtsdCurrentProcessor,
                        (PVOID)sizeof(CONTEXT),
                        (PVOID)&SpecialRegisters,
                        sizeof(KSPECIAL_REGISTERS),
                        NULL) == ERROR_SUCCESS ) {
                    print_log("cr3 = 0x%08x\n",SpecialRegisters.Cr3);
                    print_log("cr0 = 0x%08x\n",SpecialRegisters.Cr0);
                }
            }
            kd.ControlSet.TraceFlag = FALSE;
            kd.ControlSet.Dr7 = EXCEPTIONDR7;
#endif
            if(g_opt.input_comdbg_int3) {

                status0 = status;
                status = DBG_CONTINUE;
                if(g_opt.input_comdbg_int3 == 2) {
                    print_log("INT 3 handling: 'B' - break, 'G' - continue execution\n");
                    wait_int3 = TRUE;
                    while(!esc_key && wait_int3) {
                        switch(int3_key) {
                        case 0x30: /*VK_B*/
                            status = status0;
                            wait_int3 = FALSE;
                            break;
                        case 0x22: /*VK_G*/
                            status = DBG_CONTINUE;
                            wait_int3 = FALSE;
                            break;
                        }
                        if(wait_int3) {
                            Sleep(100);
                        }
                    }
                }
                if(status == DBG_CONTINUE) {
                    print_log("Continue execution\n");
                } else {
                    print_log("Pass exception to system\n");
                }
            }
        } else
        if (kd.StateChange.NewState == DbgKdLoadSymbolsStateChange) {
            if (kd.StateChange.u.LoadSymbols.UnloadSymbols) {
                if (kd.StateChange.u.LoadSymbols.PathNameLength == 0 &&
                    kd.StateChange.u.LoadSymbols.BaseOfDll == (PVOID)-1 &&
                    kd.StateChange.u.LoadSymbols.ProcessId == 0
                   ) {
                    ;
                } else {
                    len = sprintf(DbgPrintBuf, "Unloading %s\n",kd.Buffer);
                    store_KD_message((UCHAR)NtsdCurrentProcessor,
                                     kd.StateChange.ProcessorLevel,
                                     (PUCHAR)DbgPrintBuf, (USHORT)len);
                }
            } else {
                len = sprintf(DbgPrintBuf, "Loading Image %s at 0x%lx\n",
                         kd.Buffer,
                         kd.StateChange.u.LoadSymbols.BaseOfDll
                         );
                store_KD_message((UCHAR)NtsdCurrentProcessor,
                                 kd.StateChange.ProcessorLevel,
                                 (PUCHAR)DbgPrintBuf, (USHORT)len);
            }
#ifdef  i386
            kd.ControlSet.TraceFlag = FALSE;
            kd.ControlSet.Dr7 = EXCEPTIONDR7;
#endif
            status = DBG_CONTINUE;
        } else {
            //
            // BUG, BUG - invalid NewState in state change record.
            //
#ifdef  i386
            kd.ControlSet.TraceFlag = FALSE;
            kd.ControlSet.Dr7 = EXCEPTIONDR7;
#endif
            status = DBG_CONTINUE;
        }

        status = DbgKdContinue2(status, kd.ControlSet);
        if (status != ERROR_SUCCESS) {
            print_log("kd: DbgKdContinue failed: %08lx\n", status);
            esc_key = TRUE;
        }
    }
    return 0;
} // end get_KD_messages()
    
/*
 Catch strings from KD()
*/
DWORD
__stdcall
get_SYSLOG_messages(PVOID ctx)
{

    DWORD       status;
    CHAR        DbgPrintBuf[MAX_PATH];
    PCHAR       buffer = NULL;
    int         len;
    struct timeval timo = {0,0};

    ULONG       Mode = (ULONG)ctx; // 0 - syslog, 1 - raw udp

    buffer = (PCHAR)GlobalAlloc(GMEM_FIXED, 64*1024);
    if(!buffer) {
        return -1;
    }

    if(!g_Wsa_init_ok && WSAStartup(0x0001, &g_WsaData)) {
        print_err("Error: can't init network\n");
        return 4;
    }
    g_Wsa_init_ok = TRUE;
    g_opt.input_syslog_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(g_opt.input_syslog_sock == INVALID_SOCKET) {
        print_err("Error: can't init local socket\n");
        return 4;
    }
    sprintf(DbgPrintBuf, "%S", g_opt.input_syslog_HostName);

    g_opt.input_syslog_src.sin_family = AF_INET;
    g_opt.input_syslog_src.sin_port = (USHORT)g_opt.input_syslog_port;
    g_opt.input_syslog_src.sin_addr.s_addr = DbgPrintBuf[0] ? inet_addr(DbgPrintBuf) : htonl(INADDR_ANY);
    if(g_opt.input_syslog_src.sin_addr.s_addr == INADDR_NONE) {
        PHOSTENT hostent;
        hostent = gethostbyname(DbgPrintBuf);
        if(!hostent) {
            print_err("Error: invalid server address: %S\n", DbgPrintBuf);
            return 4;
        }
        g_opt.input_syslog_src.sin_addr.s_addr = *((PULONG)(hostent->h_addr));
        if(!g_opt.input_syslog_src.sin_addr.s_addr) {
            print_err("Error: can't get IP for: %S\n", DbgPrintBuf);
            return 4;
        }
    }

    // set timeout
    timo.tv_sec = 1;
    timo.tv_usec= 0;
    status = setsockopt (g_opt.input_syslog_sock,
       SOL_SOCKET,
       SO_RCVTIMEO,
       (char*)&timo,
       sizeof(struct timeval)
       ); 

    if (status == SOCKET_ERROR) {
        print_err("Error: can't set wait socket timeout\n");
    }

    if(bind(g_opt.input_syslog_sock, (struct sockaddr *)&g_opt.input_syslog_src, sizeof(g_opt.input_syslog_src)) < 0) {
        print_err("Error: can't bind to port\n");
        return 4;
    }

    // inform get_messages() that SYSLOG capturing is started
    SetEvent(hSYSLOGreadyEvent);

    while(!esc_key) {
        len = 64*1024;
        len = recv(g_opt.input_syslog_sock, buffer, len, 0);
        if(len > 0) {
            store_KD_message((UCHAR)-1,
                             -1,
                             (PUCHAR)buffer, (USHORT)len);
        }
    }
    return 0;
} // end get_SYSLOG_messages()
    
BOOL
get_keyboard_byte(
    PVOID pBuf,
    DWORD cbBuf,
    LPDWORD pcbBytesRead
    )
{
    return ReadFile(StdIn,pBuf,cbBuf,pcbBytesRead,NULL);
} // end get_keyboard_byte()

void
dump_machine_info(
    HANDLE fh
    )
{
    CHAR Str[MAX_PATH*3];
    CHAR MHzStr[64];
    CHAR CpuId[MAX_PATH];
    CHAR VendorId[MAX_PATH];
    CHAR RegPath[MAX_PATH];
    ULONG i;
    HKEY hKey;
    ULONG MHz;
    DWORD t;
    ULONG l;
    MEMORYSTATUS          ms  ;

    for(i=0; i<128; i++) {
        sprintf(RegPath, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\%u", i);
        if(RegOpenKey(HKEY_LOCAL_MACHINE, RegPath, &hKey) != ERROR_SUCCESS) {
            break;
        }
        // MHz
        l = sizeof(ULONG);
        if(RegQueryValueEx(hKey, "~MHz", NULL, &t, (PUCHAR)&MHz, &l) != ERROR_SUCCESS) {
            sprintf(MHzStr, "?");
        } else {
            sprintf(MHzStr, "%u", MHz);
        }
        // CPU Type
        l = MAX_PATH-1;
        if(RegQueryValueEx(hKey, "Identifier", NULL, &t, (PUCHAR)CpuId, &l) != ERROR_SUCCESS) {
            sprintf(CpuId, "Unknown type");
        } else {
            CpuId[MAX_PATH-1] = 0;
        }
        // CPU Vendor
        l = MAX_PATH-1;
        if(RegQueryValueEx(hKey, "VendorIdentifier", NULL, &t, (PUCHAR)VendorId, &l) != ERROR_SUCCESS) {
            sprintf(CpuId, "Unknown vendor");
        } else {
            VendorId[MAX_PATH-1] = 0;
        }
        l = sprintf(Str, "CPU-%u: %s, %s, %s MHz\n", i, CpuId, VendorId, MHzStr);
        TeeOutput(fh, Str, l);
        RegCloseKey(hKey);
    }
    ms.dwLength = sizeof (ms) ;
    GlobalMemoryStatus  (&ms) ;
    l = sprintf(Str, "Memory: %u Mb physical, %u Mb virtual\n", (ms.dwTotalPhys)/(1024*1024)+1, (ms.dwTotalVirtual)/(1024*1024)+1);
    TeeOutput(fh, Str, l);
} // end dump_machine_info()

void
dump_privileges(
    HANDLE fh
    )
{
    HANDLE              hToken;
    PTOKEN_PRIVILEGES   tp;
    DWORD               ReturnLength;
    DWORD               i, j;
    CHAR                DbgPrintBuf[MAX_PATH];
    DWORD               len;
    
    TeeOutputConst(fh, "Privilege status:\n");

    // obtain the token, first check the thread and then the process
    if (!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken)){
        if (GetLastError() == ERROR_NO_TOKEN){
            if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
                TeeOutputConst(fh, "Can't open process token\n");
                return;
            }
        } else {
            TeeOutputConst(fh, "Can't open thread token\n");
            return;
        }
    }

    GetTokenInformation(hToken,TokenPrivileges, NULL,0L, &ReturnLength);
    tp = (PTOKEN_PRIVILEGES)GlobalAlloc(GMEM_FIXED,ReturnLength);
    GetTokenInformation(hToken,TokenPrivileges, tp, ReturnLength, &ReturnLength);

    for (i=0; i<tp->PrivilegeCount;i++) {
        //tp->Privileges[i].Attributes & SE_PRIVILEGE_ENABLED SE_PRIVILEGE_ENABLED_BY_DEFAULT;
        TeeOutputConst(fh, "  ");
        len = sizeof(DbgPrintBuf);
        if(LookupPrivilegeName(NULL, &tp->Privileges[i].Luid, DbgPrintBuf, &len)) {
            TeeOutput(fh, DbgPrintBuf, len);
            DbgPrintBuf[len] = 0;
        } else {
            TeeOutputConst(fh, "???");
            len = 3;
        }
        for(j=0; ((len+j+1) & ~8) < 64; j+=8) {
            TeeOutputConst(fh, "\t");
        }
        if(tp->Privileges[i].Attributes & SE_PRIVILEGE_ENABLED) {
            TeeOutputConst(fh, " Enabled");
            if(tp->Privileges[i].Attributes & SE_PRIVILEGE_ENABLED_BY_DEFAULT) {
                TeeOutputConst(fh, " (default)");
            }
        } else {
            TeeOutputConst(fh, " Disabled");
            if(!(tp->Privileges[i].Attributes & SE_PRIVILEGE_ENABLED_BY_DEFAULT)) {
                TeeOutputConst(fh, " (default");
            }
            if(DbgPrintBuf[0]) {
                if(!Privilege(DbgPrintBuf, TRUE)) {
                    TeeOutputConst(fh, ", unavailable");
                } else {
                    Privilege(DbgPrintBuf, FALSE);
                }
            }
            TeeOutputConst(fh, ")");
        }
        TeeOutputConst(fh, "\n");
    }

    TeeOutputConst(fh, "\n");

    if (!CloseHandle(hToken))
         return;

    GlobalFree(tp);
} // end dump_privileges()

/*LONGLONG
translate_timestamp_to_sysperf(
    LONGLONG TimeStamp,
    UCHAR    TimeStampType,
    PDbgPrnHk_GetRdtscCalibration_USER_OUT CalibrationBuffer,
    LONGLONG* RelPerfCounter,
    double*   RdtscCalibration,
    LONGLONG* Frequency
    )*/
#define translate_timestamp_to_sysperf(TimeStamp, TimeStampType) \
{                                                                \
    switch(TimeStampType) {                                      \
    case TimeStampType_SysPerfCounter:                           \
        /*return TimeStamp;*/                                        \
        break;                                                   \
    case TimeStampType_RdtscPerfCounter:                         \
        TimeStamp = (LONGLONG)((double)(TimeStamp - CalibrationBuffer->RdtscTimeStampCalibration0) * (*RdtscCalibration)); \
        TimeStamp += CalibrationBuffer->SysTimeStampCalibration0/* - (*RelPerfCounter)*/; \
        /*return TimeStamp;*/                                        \
        break;                                                   \
    case TimeStampType_SysTime:                                  \
        TimeStamp = (TimeStamp/10000000)*(*Frequency) + (((TimeStamp%10000000) * (*Frequency)) / 10000000); \
        /*TimeStamp -= (*RelPerfCounter);*/                             \
        /*return TimeStamp;*/                                        \
        break;                                                   \
    }                                                            \
}

int
compare_timestamps(
    LONGLONG TimeStamp1,
    UCHAR    TimeStampType1,
    LONGLONG TimeStamp2,
    UCHAR    TimeStampType2,
    PDbgPrnHk_GetRdtscCalibration_USER_OUT CalibrationBuffer,
//    LONGLONG* RelPerfCounter,
    double*   RdtscCalibration,
    LONGLONG* Frequency
    )
{
    if(TimeStampType1 != TimeStampType2) {
        translate_timestamp_to_sysperf(TimeStamp1, TimeStampType1);
        translate_timestamp_to_sysperf(TimeStamp2, TimeStampType2);
    }
    if(TimeStamp1 > TimeStamp2)
        return 1;
    if(TimeStamp1 < TimeStamp2)
        return -1;
    return 0;
} // end compare_timestamps()

HANDLE
open_driver(
    BOOLEAN start_drv,
    BOOLEAN* installed
    )
{
    ULONG k;
    HANDLE h = NULL;
    ULONG returned;
    DbgPrnHk_GetVersion_USER_OUT VerBuffer;

    if(installed) {
        (*installed) = FALSE;
    }
    if(start_drv) {
        // try to install (or re-install) and start kernel-mode DbgPrint capturing driver
        if(g_opt.restart_drv) {
            k = NtServiceStop(NT_DbgPrnHk_SVC_NAME, 5);
        }
        k = NtServiceStart(NT_DbgPrnHk_SVC_NAME);
    }

    k = NtServiceIsRunning(NT_DbgPrnHk_SVC_NAME);
    if(start_drv) {
        if((int)k<0) {
            k = NtServiceInstall(NT_DbgPrnHk_SVC_NAME, PathToExec);
            k = NtServiceStart(NT_DbgPrnHk_SVC_NAME);
            k = NtServiceIsRunning(NT_DbgPrnHk_SVC_NAME);
        }
    }
    if(k == 1) {
        // Ok, started
        if(installed) {
            (*installed) = TRUE;
        }
    } else
    if(k == 0) {
        // installed, but not started
        if(installed) {
            (*installed) = TRUE;
        }
        if(start_drv) {
            k = NtServiceStart(NT_DbgPrnHk_SVC_NAME);
            if(k < 0) {
                // disabled ?
                k = NtServiceSetStartMode(NT_DbgPrnHk_SVC_NAME, 3);
                k = NtServiceStart(NT_DbgPrnHk_SVC_NAME);
            }
        }
    }
    if(k < 0) {
        print_log("Warrning: Can't load or install DbgPrnHk driver\n");
    }

    // Connect to driver
    h = CreateFile(NT_DbgPrnHk_USER_NAME, GENERIC_READ | GENERIC_WRITE,
                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                   NULL,
                   OPEN_EXISTING,
                   FILE_ATTRIBUTE_NORMAL/* | FILE_FLAG_NO_BUFFERING*/,  NULL);
    if(!h || h == (HANDLE)(-1)) {
        print_err("Error: can't open driver\n");
        return NULL;
    }

    // check driver version
    if(!DeviceIoControl(h,IOCTL_DbgPrnHk_GetVersion,
                        NULL,0,
                        &VerBuffer, sizeof(VerBuffer),
                        &returned,NULL)) {
        print_err("Error: incompatible driver version\n");
        //exit(1);
        return NULL;
    }
    if(VerBuffer.Major != PROD_VER_MJ ||
       VerBuffer.Minor != PROD_VER_MN/* ||
       VerBuffer.Sub   != PROD_VER_NSUB*/) {
        print_err("Error: incompatible driver version\n");
        CloseHandle(h);
        return NULL;
    }
    return h;
} // end open_driver()

BOOLEAN
update_drv_conf()
{
    ULONG returned;
    BOOLEAN installed;
    HANDLE h;
    BOOLEAN retval = FALSE;

    h = open_driver(FALSE, &installed);
    if(h) {
        // reconfig driver
        if(!DeviceIoControl(h,IOCTL_DbgPrnHk_ReadRegConf,
                            NULL,0,
                            NULL,0,
                            &returned,NULL)) {
            print_err("Error: cannot update driver settings\n");
        } else {
            retval = TRUE;
            print_log("Driver settings updated\n");
        }
        CloseHandle(h);
    } else {
        if(installed) {
            print_log("Registry settings updated\n");
        } else {
            print_err("Error: driver seems to be not installed\n");
        }
    }
    return retval;
} // end update_drv_conf()

BOOLEAN
get_drv_conf()
{
    ULONG returned;
    BOOLEAN installed;
    HANDLE h;
    BOOLEAN retval = FALSE;

    h = open_driver(FALSE, &installed);
    if(h) {
        // reconfig driver
        if(!DeviceIoControl(h,IOCTL_DbgPrnHk_GetDrvConf,
                            NULL,0,
                            &st,sizeof(st),
                            &returned,NULL)) {
            print_err("Error: cannot read current driver settings\n");
        } else {
            retval = TRUE;
        }
        CloseHandle(h);
    } else {
    }
    return retval;
} // end get_drv_conf()

BOOLEAN
set_drv_conf()
{
    ULONG returned;
    BOOLEAN installed;
    HANDLE h;
    BOOLEAN retval = FALSE;

    h = open_driver(FALSE, &installed);
    if(h) {
        // reconfig driver
        if(!DeviceIoControl(h,IOCTL_DbgPrnHk_SetDrvConf,
                            &st,sizeof(st),
                            NULL,0,
                            &returned,NULL)) {
            print_err("Error: cannot update driver settings\n");
        } else {
            retval = TRUE;
        }
        CloseHandle(h);
    } else {
    }
    return retval;
} // end set_drv_conf()

HANDLE
init_input_stream()
{
    HANDLE h = NULL;
    DWORD InputMode;

    if(g_opt.input_driver) {
        // try to install (or re-install) and start kernel-mode DbgPrint capturing driver
        h = open_driver(TRUE, NULL);
        if(!h) {
            print_log("Working without driver.\n");
            g_opt.without_driver = TRUE;
            g_opt.input_driver = FALSE;
        } else {
            if(g_opt.nowait_msg) {
                get_drv_conf();
                g_max_msg = min(st.QueueSize, st.BufferSize/sizeof(DbgPrnHk_GetMessages_USER_OUT));
            }
        }
    } else
    if(g_opt.input_file) {
        // Open raw message data file
        h = CreateFileW(g_opt.input_FileName, GENERIC_READ,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL/* | FILE_FLAG_NO_BUFFERING*/,  NULL);
        if(!h || h == (HANDLE)(-1)) {
            print_err("Error: can't open input file\n");
            return NULL;
        }
    } else
    if(g_opt.input_stdin) {
        // Setup raw message data 
        if(GetConsoleMode(StdIn, &InputMode)) {
            if(!SetConsoleMode(StdIn, InputMode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT))) {
                print_err("Error: Can't set console input mode\n");
                return NULL;
            }
        }
        h = StdIn;
    } else
    if(g_opt.input_comdbg) {
        DWORD       st;

        kd.Connected = FALSE;
        DbgKdpPrint = store_KD_message;
        DbgKdpGetConsoleByte = get_keyboard_byte;

        if(DbgKdConnectAndInitialize(g_opt.input_comdbg_port, g_opt.input_comdbg_baud) != ERROR_SUCCESS) {
            return NULL;
        }

        print_log("Waiting for Debugger connection on COM%d (%d baud)...\n", g_opt.input_comdbg_port, g_opt.input_comdbg_baud);
        st = DbgKdWaitStateChange(&kd.StateChange, kd.Buffer, 254);
        if(!kd.Connected) {
            kd.Connected = TRUE;
            print_log("Kernel Debugger connection established (%s protocol)\n",
                DbgKdpUse64bit ? "64-bit" : "32-bit");
        }

        if(st != ERROR_SUCCESS) {
            print_log("DbgKdWaitStateChange failed: %08lx\n", st);
            return NULL;
        }
        memset(&kd.VersionBuffer, 0, sizeof(kd.VersionBuffer));
        st = DbgKdGetVersion(&kd.VersionBuffer);
        if(st != ERROR_SUCCESS) {
            print_log("DbgKdGetVersion failed: %08lx\n", st);
        }

#ifdef  i386
        kd.ControlSet.TraceFlag = FALSE;
        kd.ControlSet.Dr7 = EXCEPTIONDR7;
#endif
        st = DbgKdContinue2(DBG_CONTINUE, kd.ControlSet);
        if(st != ERROR_SUCCESS) {
            print_log("DbgKdContinue failed: %08lx\n", st);
            return NULL;
        }
        return (HANDLE)(-2);
    } else
    {
        print_err("Error: no input stream found\n");
        return NULL;
    }
    return h;
} // end init_input_stream()

void
swap_msg_buffers()
{
    PDbgPrnHk_GetMessages_USER_OUT tmp_MsgBuffer2;

    // Get user-mode messages
    WaitForSingleObject(hODS_lock, INFINITE);

    // swap read/write buffers
    tmp_MsgBuffer2 = MsgBuffer2;

    MsgCount2   = MsgCount2a;
    MsgBuffer2  = MsgBuffer2a;

    MsgCount2a  = 0;
    MsgBuffer2a = tmp_MsgBuffer2;

    ReleaseMutex(hODS_lock);
    return;
} // end swap_msg_buffers()

int
read_input_stream(
    HANDLE h,
    PVOID MsgBuffer,
    ULONG BufferSize
    )
{
    ULONG returned;

    returned = 0;
    if(g_opt.input_driver) {
/*
        // Read calibration info
        ReCalibrationBuffer.Recalibrate = TRUE;
        ReCalibrationBuffer.ChangeType = FALSE;
        if(!DeviceIoControl(h,IOCTL_DbgPrnHk_GetRdtscCalibration,
                            &ReCalibrationBuffer, sizeof(ReCalibrationBuffer),
                            &CalibrationBuffer, sizeof(CalibrationBuffer),
                            &returned,NULL)) {
            returned = GetLastError();
            returned = 0;
            RdtscCalibration = 1.0;
        } else {
            RdtscCalibration = (double)(CalibrationBuffer.SysTimeStampCalibration - CalibrationBuffer.SysTimeStampCalibration0) /
                               (double)(CalibrationBuffer.RdtscTimeStampCalibration - CalibrationBuffer.RdtscTimeStampCalibration0);
        }
*/
        // Get kernel messages
        if(!DeviceIoControl(h,IOCTL_DbgPrnHk_GetMessages,
                            MsgBuffer, min(BufferSize, 0x10000),
                            MsgBuffer, min(BufferSize, 0x10000),
                            &returned,NULL)) {
            returned = GetLastError();
            returned = 0;
        }
        if(g_opt.without_driver) {
            swap_msg_buffers();
        }
        if(g_opt.nowait_msg) {
            if(g_max_msg) {
                if(g_max_msg < returned/sizeof(DbgPrnHk_GetMessages_USER_OUT)) {
                    // exit if we have read entire buffer
                    esc_key = TRUE;
                } else {
                    g_max_msg -= returned/sizeof(DbgPrnHk_GetMessages_USER_OUT);
                }
            }
            if(!returned) {
                esc_key = TRUE;
            }
        }

    } else
    if(g_opt.input_file) {
        // Read messages
        if(!ReadFile(h,
                            MsgBuffer, BufferSize,
                            &returned,NULL) ||
           !returned) {
            returned = GetLastError();
            returned = 0;
            esc_key = TRUE;
        }
    } else
    if(g_opt.input_stdin) {
        // Read messages from pipe
        returned = 0;
        if(!ReadFile(h,
//                                    MsgBuffer, min(BufferSize, returned),
                            MsgBuffer, BufferSize,
                            &returned,NULL)) {
            returned = GetLastError();
            returned = 0;
            esc_key = TRUE;
        }
    } else
    if(g_opt.input_comdbg) {
        swap_msg_buffers();
        returned = 0;
    } else
    if(g_opt.input_syslog) {
        swap_msg_buffers();
        returned = 0;
    }
    return returned;
} // end read_input_stream()

void
close_input_stream(
    HANDLE h
    )
{
    if(!g_opt.input_stdin &&
       !g_opt.input_comdbg) {
        CloseHandle(h);
    }
} // end close_input_stream()

/*
 merge user- and kernel-mode logs
*/
ULONG
get_messages()
{
    HANDLE h;
    HANDLE fh;
    HANDLE ih;

    ULONG len;
    ULONG i, j, j2;
//    ULONG k;
//    ULONG r;
    WCHAR a;
    PWCHAR NumPos;
    ULONG returned;
    BOOLEAN new_line;
    BOOLEAN CallerMode;
    CHAR DbgPrintBuf[4096];
    ULONG SleepCount;
    CHAR tab3[] = "\t\t\t";
    ULONG CurLogSize;
    SYSTEMTIME SysTime;

    OSVERSIONINFO OsVer;
    PCHAR PlatformStr;

    DBGPRN_FORMAT_MESSAGE_CTX msgst;
    DBGPRN_TIMESTAMP_CTX      timest;

    HANDLE hODSthread = NULL;
    ULONG thId;
    HANDLE handles[3];

    HANDLE hKDthread = NULL;
    ULONG thId_kd;
    
    HANDLE hSYSLOGthread = NULL;
    ULONG thId_syslog;
    
    TIME_ZONE_INFORMATION TzInfo;
    LONG Bias;

    BOOLEAN last2 = FALSE;
    BOOLEAN upd_i = FALSE;
    PDbgPrnHk_GetMessages_USER_OUT CurMsg = NULL;

    //DbgPrnHk_GetRdtscCalibration_USER_OUT CalibrationBuffer;
    //double RdtscCalibration;

    // gain privileges to install/load driver/service
    // it is not necessary here if we are not running as service
    if(sys_account) {
        if(!Privilege(SE_LOAD_DRIVER_NAME, TRUE)) {
            print_log("Warning: Insufficient privileges\n");
        }
    }

    h = init_input_stream();

    if(g_opt.output_file) {
        // Init log-file name
        wcsncpy(LogFileN, g_opt.LogFile, MAX_PATH-10);
        LogFileN[MAX_PATH-10] = 0;
        len = wcslen(LogFileN);

        NumPos = &LogFileN[len];
        LogFileSuf[0] = 0;
        for(i=len; i>0; i--) {
            a = LogFileN[i-1];
            if(a == '\\') {
                break;
            }
            if(a == '.') {
                NumPos = &LogFileN[i-1];
                wcscpy(LogFileSuf, NumPos);
                break;
            }
        }
    }

    if(g_opt.output_syslog) {
        if(!g_Wsa_init_ok && WSAStartup(0x0001, &g_WsaData)) {
            print_err("Error: can't init network\n");
            return 4;
        }
        g_Wsa_init_ok = TRUE;
        g_opt.output_syslog_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if(g_opt.output_syslog_sock == INVALID_SOCKET) {
            print_err("Error: can't init local socket\n");
            return 4;
        }
        sprintf(DbgPrintBuf, "%S", g_opt.output_syslog_HostName);

        g_opt.output_syslog_dst.sin_family = AF_INET;
        g_opt.output_syslog_dst.sin_port = (USHORT)g_opt.output_syslog_port;
        g_opt.output_syslog_dst.sin_addr.s_addr = inet_addr(DbgPrintBuf);
        if(g_opt.output_syslog_dst.sin_addr.s_addr == INADDR_NONE) {
            PHOSTENT hostent;
            hostent = gethostbyname(DbgPrintBuf);
            if(!hostent) {
                print_err("Error: invalid server address: %S\n", DbgPrintBuf);
                return 4;
            }
            g_opt.output_syslog_dst.sin_addr.s_addr = *((PULONG)(hostent->h_addr));
        }
        if(!g_opt.output_syslog_dst.sin_addr.s_addr) {
            print_err("Error: can't get IP for: %S\n", DbgPrintBuf);
            return 4;
        }
    }

    if(!g_opt.output_none ||
       g_opt.input_comdbg) {
        // alloc buffers
        MsgBuffer = (PDbgPrnHk_GetMessages_USER_OUT)
            GlobalAlloc(GMEM_FIXED, BufferSize);
        if(!MsgBuffer) {
            print_err("Error: can't allocate message buffer\n");
            return 4;
        }
        MsgBuffer2 = (PDbgPrnHk_GetMessages_USER_OUT)
            GlobalAlloc(GMEM_FIXED, BufferSize);
        if(!MsgBuffer2) {
            print_err("Error: can't allocate message buffer (2)\n");
            return 4;
        }
        MsgBuffer2a = (PDbgPrnHk_GetMessages_USER_OUT)
            GlobalAlloc(GMEM_FIXED, BufferSize);
        if(!MsgBuffer2a) {
            print_err("Error: can't allocate message buffer (2)\n");
            return 4;
        }
    }
    i = g_opt.StartLogNum;

    if(!g_opt.input_stdin) {
        if(!g_opt.output_none) {
            help_text2 = (char*)help_text2_std;
        } else {
            help_text2 = (char*)help_text2_proxy;
        }
        if(!g_opt.nowait_msg) {
            print_log(help_text2);
        }
    }

    //print_err("SYS ACCOUINT: %x\n", sys_account);
    if(sys_account) {
        // change service status to RUNNING
        ReportStatusToSCMgr(
            SERVICE_RUNNING,      // service state
            NO_ERROR,             // exit code
            0);                   // wait hint
    }

    handles[2] = NULL;
    //print_err("Init 1...\n");
    if(g_opt.input_driver) {
        hODSreadyEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if(!hODSreadyEvent) {
            print_err("Error: can't create OutputDebugString() event\n");
            return 4;
        }
        //print_err("Init 1.1...\n");
        // start user-mode OutputDebugString capturing thread
        hODSthread = CreateThread(NULL, 0, get_ODS_messages, NULL, 0, &thId);
        if(!hODSthread) {
            print_err("Error: can't create OutputDebugString() thread\n");
            return 4;
        }
        //print_err("Init 1.2... (%x, %x)\n", hODSreadyEvent, hODSthread);
        handles[0] = hODSreadyEvent;
        handles[1] = hODSthread;
        //print_err("Init 1.3...\n");
        if(WaitForMultipleObjects(2, &handles[0], FALSE, 5*1000) != WAIT_OBJECT_0) {
            print_err("Error: can't init OutputDebugString() capturing thread\n");
            return 4;
        }
        //print_err("Init 1 ok\n");
    }
    //print_err("Init 2...\n");
    if(g_opt.input_comdbg) {
        hKDreadyEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if(!hKDreadyEvent) {
            print_err("Error: can't create COM KD event\n");
            return 4;
        }
        // start user-mode OutputDebugString capturing thread
        hKDthread = CreateThread(NULL, 0, get_KD_messages, NULL, 0, &thId_kd);
        if(!hKDthread) {
            print_err("Error: can't create COM KD thread\n");
            return 4;
        }
        handles[0] = hKDreadyEvent;
        handles[1] = hKDthread;
        if(WaitForMultipleObjects(2, &handles[0], FALSE, 5*1000) != WAIT_OBJECT_0) {
            print_err("Error: can't init COM KD capturing thread\n");
            return 4;
        }
    }

    if(g_opt.input_syslog) {
        hSYSLOGreadyEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if(!hSYSLOGreadyEvent) {
            print_err("Error: can't create SYSLOG event\n");
            return 4;
        }
        // start user-mode OutputDebugString capturing thread
        hSYSLOGthread = CreateThread(NULL, 0, get_SYSLOG_messages, NULL, 0, &thId_syslog);
        if(!hSYSLOGthread) {
            print_err("Error: can't create SYSLOG thread\n");
            return 4;
        }
        handles[0] = hSYSLOGreadyEvent;
        handles[1] = hSYSLOGthread;
        if(WaitForMultipleObjects(2, &handles[0], FALSE, 5*1000) != WAIT_OBJECT_0) {
            print_err("Error: can't init SYSLOG capturing thread\n");
            return 4;
        }
    }

    //print_err("Waiting\n");
    // produce logs until break come
    while(!esc_key) {

        if(g_opt.output_none) {
            Sleep(100);
            continue;
        }
        if(g_opt.output_file) {
            // find unused name for log-file
            if(i>0) {
                swprintf(NumPos, L"%d%s", i, LogFileSuf);
            }
            fh = CreateFileW(LogFileN, GENERIC_READ | GENERIC_WRITE,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           NULL,
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL/* | FILE_FLAG_NO_BUFFERING*/,  NULL);
            if(fh && fh != (HANDLE)(-1)) {
                CloseHandle(fh);
                fh = NULL;
                if(!g_opt.overwrite_old) {
                    i++;
                    if(!i) {
                        print_err("Error: can't find unused filename\n");
                        return 2;
                    }
                    continue;
                } else {
                    DeleteFileW(LogFileN);
                    upd_i = TRUE;
                }
            }
            if((i >= g_opt.MaxLogs) && (i-g_opt.MaxLogs >= g_opt.StartLogNum)) {
                if(i-g_opt.MaxLogs > 0) {
                    swprintf(NumPos, L"%d%s", i-g_opt.MaxLogs, LogFileSuf);
                } else {
                    swprintf(NumPos, L"%s", LogFileSuf);
                }
                DeleteFileW(LogFileN);
                if(i > 0) {
                    swprintf(NumPos, L"%d%s", i, LogFileSuf);
                } else {
                    swprintf(NumPos, L"%s", LogFileSuf);
                }
            }
            if(upd_i) {
                i++;
                upd_i = FALSE;
            }
            // create log-file
            fh = CreateFileW(LogFileN, GENERIC_READ | GENERIC_WRITE,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           NULL,
                           CREATE_NEW,
                           FILE_ATTRIBUTE_NORMAL/* | FILE_FLAG_NO_BUFFERING*/,  NULL);
            if(g_opt.use_index) {
                LineNumber = 0;
                swprintf(IdxFileN, L"%s.idx", LogFileN);
                ih = CreateFileW(IdxFileN, GENERIC_READ | GENERIC_WRITE,
                               FILE_SHARE_READ | FILE_SHARE_WRITE,
                               NULL,
                               CREATE_NEW,
                               FILE_ATTRIBUTE_NORMAL/* | FILE_FLAG_NO_BUFFERING*/,  NULL);
                memset(&IdxHeader, 0, sizeof(IdxHeader));

                IdxHeader.log_pid       = g_opt.log_pid      ;
                IdxHeader.log_tid       = g_opt.log_tid      ;
                IdxHeader.log_mode      = g_opt.log_mode     ;
                IdxHeader.log_irql      = g_opt.log_irql     ;
                IdxHeader.log_time_ex   = g_opt.log_time_ext ;
                IdxHeader.log_time_rel  = g_opt.log_time_perf ;
                IdxHeader.log_time_date = g_opt.log_time_date;
                IdxHeader.log_time_time = g_opt.log_time_time;
                             
                IdxHeader.skip_kernel   = g_opt.skip_kernel  ;
                IdxHeader.skip_umode    = g_opt.skip_umode   ;

                IdxHeader.log_time_nano = g_opt.log_time_nano;

                IdxHeader.Major  = IDX_VER_MJ;
                IdxHeader.Minor  = IDX_VER_MN;
                IdxHeader.Sub    = IDX_VER_NSUB;

                IdxHeader.Length = sizeof(IdxHeader);
                IdxHeader.IdxItemSize = sizeof(IdxItem);
            }
        } else {
            fh = StdOut;
        }
        if(!fh || fh == (HANDLE)(-1)) {
            print_err("Error: can't create log-file\n");
            fh = NULL;
            return 3;
        }
        if(g_opt.use_index) {
            if(!ih || ih == (HANDLE)(-1)) {
                print_err("Error: can't create index-file\n");
                ih = NULL;
                return 3;
            }
            WriteFile(ih, &IdxHeader, sizeof(IdxHeader), &len, NULL);
        }
        if(g_opt.prealloc_mode) {
            SetFilePointer(fh, g_opt.MaxLogSize*1024*1024, NULL, FILE_BEGIN);
            SetEndOfFile(fh);
            SetFilePointer(fh, 0, NULL, FILE_BEGIN);
        }

        dbgprint_format_msg_init(&g_opt, &msgst);

        // write log header
        if(g_opt.output_file) {
            print_log("Writing log to %ws\n", LogFileN);
        }
        CurLogSize = 0;
        if(!g_opt.skip_header) {
            if(g_opt.input_driver || g_opt.input_comdbg) {
                if(g_opt.input_comdbg) {
                    len = sprintf(DbgPrintBuf, "Log captured by DbgPrintLog v%d.%d%s\n", PROD_VER_MJ, PROD_VER_MN, PROD_VER_SUB);
                } else {
                    len = sprintf(DbgPrintBuf, "Log generated by DbgPrintLog v%d.%d%s\n", PROD_VER_MJ, PROD_VER_MN, PROD_VER_SUB);
                }
                TeeOutput(fh, DbgPrintBuf, len);

                // init timestamp calibration
                dbgprint_timestamp_init(&g_opt, h, &timest);

                FileTimeToSystemTime((PFILETIME)&timest.FtmSysTime, &SysTime);
                len = sprintf(DbgPrintBuf, "Timestamp: %d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d (%I64d.%4.4d ticks), %s\n",
                    SysTime.wYear,
                    SysTime.wMonth,
                    SysTime.wDay,
                    SysTime.wHour,
                    SysTime.wMinute,
                    SysTime.wSecond,
                    timest.RelPerfCounter/10000,
                    (ULONG)(timest.RelPerfCounter%10000),
                    g_opt.log_time_utc ? "UTC" : "Local"
                    );
                TeeOutput(fh, DbgPrintBuf, len);

                if(g_opt.input_comdbg) {
                    len = 0;
                    len = sprintf(DbgPrintBuf+len, "WinDbg proto %x (%s), flags %x\n",
                        kd.VersionBuffer.ProtocolVersion, DbgKdpUse64bit ? "64-bit" : "32-bit", kd.VersionBuffer.Flags);
                    len+= sprintf(DbgPrintBuf+len, "%s-processor %s system\n",
                        (kd.VersionBuffer.Flags & DBGKD_VERS_FLAG_MP) ? "Multi" : "Uni", 
                        (kd.VersionBuffer.Flags & DBGKD_VERS_FLAG_PTR64) ? "64-bit" : "32-bit");

                    len+= sprintf(DbgPrintBuf+len, "KernelBase %#x\n", kd.VersionBuffer.KernBase);
                    len+= sprintf(DbgPrintBuf+len, "PsLoadedModuleList %#x\n", kd.VersionBuffer.PsLoadedModuleList);
                    if(kd.VersionBuffer.Flags & DBGKD_VERS_FLAG_DATA) {
                        len+= sprintf(DbgPrintBuf+len, "DebuggerDataList %#x\n", kd.VersionBuffer.DebuggerDataList);
                    }
                    len+= sprintf(DbgPrintBuf+len, "MachineType: ");
                    switch(kd.VersionBuffer.MachineType) {
                    case IMAGE_FILE_MACHINE_I386:
                        len+= sprintf(DbgPrintBuf+len, "x86");
                        break;
                    case IMAGE_FILE_MACHINE_IA64:
                        len+= sprintf(DbgPrintBuf+len, "IA64");
                        break;
                    case IMAGE_FILE_MACHINE_AMD64:
                        len+= sprintf(DbgPrintBuf+len, "AMD64");
                        break;
                    default:
                        len+= sprintf(DbgPrintBuf+len, "Unknown");
                        break;
                    }
                    len+= sprintf(DbgPrintBuf+len, " (%#x)\n", kd.VersionBuffer.MachineType);
                    TeeOutput(fh, DbgPrintBuf, len);
                }
                if(!g_opt.input_comdbg) {
                    GetTimeZoneInformation(&TzInfo);
                    Bias = TzInfo.Bias + TzInfo.DaylightBias;
                    len = sprintf(DbgPrintBuf, "Time Zone: %s%2.2d%2.2d, %S\n",
                        Bias >= 0 ? "-" : "+",
                        abs(Bias) / 60,
                        abs(Bias) % 60,
                        TzInfo.StandardName
                        );
                    TeeOutput(fh, DbgPrintBuf, len);
                }
                OsVer.dwOSVersionInfoSize = sizeof(OsVer);
                if(g_opt.input_comdbg) {
                    OsVer.dwBuildNumber = kd.VersionBuffer.MinorVersion;
                    OsVer.dwPlatformId = VER_PLATFORM_WIN32_NT;
                    OsVer.szCSDVersion[0] = 0;
                    switch(kd.VersionBuffer.MinorVersion) {
                    case 1057:
                        OsVer.dwMajorVersion = 3;
                        OsVer.dwMinorVersion = 51;
                        break;
                    case 1381:
                        OsVer.dwMajorVersion = 4;
                        OsVer.dwMinorVersion = 0;
                        break;
                    case 2195:
                        OsVer.dwMajorVersion = 5;
                        OsVer.dwMinorVersion = 0;
                        break;
                    case 2600:
                        OsVer.dwMajorVersion = 5;
                        OsVer.dwMinorVersion = 1;
                        break;
                    case 3790:
                        OsVer.dwMajorVersion = 5;
                        OsVer.dwMinorVersion = 2;
                        break;
                    case 6000:
                        OsVer.dwMajorVersion = 6;
                        OsVer.dwMinorVersion = 0;
                        break;
                    default:
                        OsVer.dwMajorVersion = 0;
                        OsVer.dwMinorVersion = 0;
                        if(kd.VersionBuffer.ProtocolVersion < 4) {
                            // 3.51 ?
                            OsVer.dwMajorVersion = 3;
                            OsVer.dwMinorVersion = 5;
                        }
                        break;
                    }
                } else {
                    GetVersionEx(&OsVer);
                }
                switch(OsVer.dwPlatformId) {
                case VER_PLATFORM_WIN32s:
                    PlatformStr = "Win";
                    break;
                case VER_PLATFORM_WIN32_WINDOWS:
                    PlatformStr = "Win 9x";
                    break;
                case VER_PLATFORM_WIN32_NT:
                    PlatformStr = "NT";
                    break;
                default:
                    PlatformStr = "Unknown";
                }
                len = sprintf(DbgPrintBuf, "OS Version: %d:%s %d.%d (%d)%s%s\n",
                    OsVer.dwPlatformId,
                    PlatformStr,
                    OsVer.dwMajorVersion,
                    OsVer.dwMinorVersion,
                    OsVer.dwBuildNumber,
                    g_opt.input_comdbg ? "" : "\n  ",
                    OsVer.szCSDVersion);
                TeeOutput(fh, DbgPrintBuf, len);

                if(!g_opt.input_comdbg) {
                    TeeOutputConst(fh, "Hostname: ");
                    len = 1023;
                    GetComputerName(DbgPrintBuf, &len);
                    DbgPrintBuf[1023] = 0;
                    TeeOutput(fh, DbgPrintBuf, len);

                    TeeOutputConst(fh, "\nUsername: ");
                    len = 1023;
                    GetUserName(DbgPrintBuf, &len);
                    DbgPrintBuf[1023] = 0;
                    TeeOutput(fh, DbgPrintBuf, len);

                    TeeOutputConst(fh, "\n");
                    dump_machine_info(fh);
                    TeeOutputConst(fh, "\n");
                    dump_privileges(fh);
                    TeeOutputConst(fh, "\n");
                }

                if(!h) {
                    TeeOutputConst(fh, "WARNING: no input stream available\n");
                }
            } else {
                len = sprintf(DbgPrintBuf, "Log translated by DbgPrintLog v%d.%d%s\n", PROD_VER_MJ, PROD_VER_MN, PROD_VER_SUB);
                TeeOutput(fh, DbgPrintBuf, len);
            }

            TeeOutputConst(fh, "Used command line:\n");
            len = sprintf(DbgPrintBuf, "%S\n", g_CmdLine);
            TeeOutput(fh, DbgPrintBuf, len);

            TeeOutputConst(fh, "----- End of Log header -----\n");
        }

        // write to single log-file
        next_log = FALSE;
        while(!esc_key && !next_log) {

            dbgprint_timestamp_resync(&g_opt, h, &timest);
            
            returned = read_input_stream(h, MsgBuffer, BufferSize);
/*
            if(g_opt.skip_kernel) {
                returned = 0;
            }
*/
            if(g_opt.skip_umode) {
                MsgCount2 = 0;
            }

            // drop messages if paused
            if(pause_key) {
                returned = 0;
                MsgCount2 = 0;
            }
            if(!returned && !MsgCount2) {
                SleepCount++;
                if(SleepCount > g_opt.FlushTimeout) {
                    SleepCount = 0;
                    new_line = TRUE;
                    FlushFileBuffered(fh);
                }
                Sleep(100);
                //continue;
            } else {
                SleepCount = 0;
            }
            j = 0;
            j2 = 0;
            // join user- and kernel-mode logs
            while(returned >= sizeof(DbgPrnHk_GetMessages_USER_OUT) || MsgCount2) {
                // assure time ordering
                if(returned >= sizeof(DbgPrnHk_GetMessages_USER_OUT) && MsgCount2) {
/*                    if(MsgBuffer [j] .TimeStamp.QuadPart >
                       MsgBuffer2[j2].TimeStamp.QuadPart) {*/
                      if(compare_timestamps(
                             MsgBuffer [j] .TimeStamp.QuadPart,
                             MsgBuffer [j] .TimeStampType,
                             MsgBuffer2[j2].TimeStamp.QuadPart,
                             MsgBuffer2[j2].TimeStampType,
                             &timest.CalibrationBuffer,
                             /*&RelPerfCounter,*/
                             &timest.RdtscCalibration,
                             (PLONGLONG)&timest.Frequency
                                           )
                          > 0) {
                        CurMsg = &MsgBuffer2[j2];
                        if(!last2) {
                            msgst.new_line = TRUE;
                            last2 = TRUE;
                        }
                    } else {
                        CurMsg = &MsgBuffer[j];
                        if(last2) {
                            msgst.new_line = TRUE;
                            last2 = FALSE;
                        }
                    }
                } else
                if(returned >= sizeof(DbgPrnHk_GetMessages_USER_OUT)) {
                    CurMsg = &MsgBuffer[j];
                    if(last2) {
                        new_line = TRUE;
                        last2 = FALSE;
                    }
                } else {
                    CurMsg = &MsgBuffer2[j2];
                    if(!last2) {
                        msgst.new_line = TRUE;
                        last2 = TRUE;
                    }
                }
                // filter message by origin mode
                CallerMode = CurMsg->CallerMode;
                if(g_opt.skip_kernel && !CallerMode) {
                    CurMsg->Length = 0;
                }
                if(g_opt.skip_umode && CallerMode) {
                    CurMsg->Length = 0;
                }
                // write message to file (buffered)
                if(!g_opt.raw_mode || g_opt.output_syslog) {
                    len = dbgprint_format_msg(&g_opt, CurMsg, DbgPrintBuf, &timest, &msgst);
                } else {
                    len = sizeof(DbgPrnHk_GetMessages_USER_OUT);
                }
                if(len) {
                    if(g_opt.output_syslog) {
                        sendto(g_opt.output_syslog_sock, DbgPrintBuf,
                               min(len, g_opt.output_syslog_mtu), 0, (SOCKADDR*)&g_opt.output_syslog_dst,
                               sizeof(g_opt.output_syslog_dst));
                    }
                    if(msgst.new_line) {
                        LineNumber++;
                        WriteLineNumber(ih, CurLogSize);
                    }
                    if(g_opt.raw_mode) {
                        TeeOutput(fh, (PCHAR)CurMsg, sizeof(DbgPrnHk_GetMessages_USER_OUT));
                    } else {
                        TeeOutput(fh, DbgPrintBuf+msgst.syslog_hrd_len, len-msgst.syslog_hrd_len);
                    }
                    CurLogSize+=len;
                }

                // go to next message
                if(last2) {
                    j2++;
                    MsgCount2--;
                } else {
                    j++;
                    returned -= sizeof(DbgPrnHk_GetMessages_USER_OUT);
                }
            } // end while(returned >= sizeof(DbgPrnHk_GetMessages_USER_OUT) || MsgCount2)
            if(j+j2 < 256) {
                Sleep(20);
            }
            // check user controls
            if(flush_file || g_opt.sync_mode) {
                if(g_opt.output_file && !g_opt.sync_mode) {
                    print_log("flush log\n");
                }
                FlushFileBuffered(fh);
                flush_file = FALSE;
            }
            if(next_log) {
                print_log("next log\n");
            }
            if((CurLogSize+0x10000*2) > g_opt.MaxLogSize*1024*1024 && g_opt.output_file) {
                next_log = TRUE;
                print_log("max log size limit reached -> next log\n");
            }
        } // end of Get Message loop
        FlushFileBuffered(fh);
        CloseHandle(fh);
        fh = NULL;
        if(g_opt.use_index) {
            CloseHandle(ih);
            ih = NULL;
        }
    }

    // cleanup
    if(fh) {
        CloseHandle(fh);
    }
    close_input_stream(h);
    if(g_opt.nowait_msg) {
        print_log("Done\n");
    }
    return 0;
} // end get_messages()

static const char help_text[] = 
    "Usage:\n"
    "\tDbgPrintLog.exe\t[<switches>] [<log-file name>]\n"
    "Switches:\n"
    "    Log-file format:\n"
    "\t-m, --caller_mode\n"
    "\t         write initiator mode (K - kernel, U - user) to log\n"
    "\t-p, --pid, --process_id\n"
    "\t         write ProcessId to log\n"
    "\t-P, --pname, --process_name\n"
    "\t         write Process Name to log\n"
    "\t-t, --tid, --thread_id\n"
    "\t         write ThreadId to log (for kernel-mode only)\n"
    "\t-i, --irql\n"
    "           write IRQL to log (for kernel-mode only)\n"
    "\t--cpu    write CPU num to log (for kernel-mode only)\n"
    "\t--full   same as -m -p -t -i\n"
    "\t--raw    write messages in raw format (doesn't affect syslog stream)\n"
    "\t-T FMT, --timestamp_fields FMT\n"
    "\t         specify absolute time format FMT. FMT string can contain\n"
    "\t         the following switches: D - date, T - time, N - high precision time\n"
    "\t         R - relative time (tick count), U - UTC time\n"
//    "\t+fp PID  log only messages with ProcessId==PID\n"
//    "\t-fp PID  do not log messages with ProcessId==PID\n"
    "\t-fm M    do not log messages from <M> mode\n"
    "\t         <M> can be K - kernel or U - user\n"
    "\t--filter:<action> <condition>\n"
    "\t         perform <action> for all messages matching <condition>\n"
    "\t         prior to putting them to log. For now the only possible action\n"
    "\t         is DENY (or BLOCK, SKIP, DROP). Available <conditions> are:\n"
    "\t         origin=kernel (origin=ring0 or K) - messages from kernel mode\n"
    "\t         origin=user (origin=ring3 or U) - messages from user mode\n"
    "\t-sfp, --stack_frame_ptr\n"
    "\t         write Stack Frame Pointer information to log (for kernel-mode only)\n"
    "    Log-file control:\n"
    "\t-s NUM, --max_log_size NUM\n"
    "\t         set max log-file size to NUM Mbytes (0 < NUM < 2048)\n"
    "\t         128Mb is used by default\n"
    "\t-S NUM, --prealloc_log_size NUM\n"
    "\t         does like -s, but sets initial file size to specified value.\n"
    "\t         Is intended mainly for use by GUI tool.\n"
    "\t-x NUM, --build_index NUM\n"
    "\t         create index file for each log file.\n"
    "\t         Is intended mainly for use by GUI tool.\n"
    "\t-l NUM, --max_log_count NUM, --log_rotate_count NUM\n"
    "\t         keep NUM latest log files\n"
    "\t-ovw, --overwrite_old_log\n"
    "\t         overwrite existing log files\n"
    "\t-n NUM, --initial_log_number NUM\n"
    "\t         set start log number to NUM\n"
    "\t-ft NUM, --flush_timeout NUM\n"
    "\t         flush messages buffer after NUM seconds of inactivity\n"
    "\t-sm, --sync_mode\n"
    "\t         synchronous mode. all messages are flushed immediately\n"
//    "\t-cf      copy log to both STDOUT and file\n"
    "\t-wd DIR, --working_directory DIR\n"
    "\t         specify working directory\n"
    "    Output data stream control:\n"
    "\t--out:<output> [<parameters>]\n"
    "\t         multiple --out: options ara allowed\n"
    "\t         available values for <output> are:\n"
    "\t  stdout   write log to STDOUT instead of file\n"
//    "\t  both     write log to both STDOUT and file\n"
    "\t  file     write log file (default)\n"
    "\t  syslog HOST[:PORT]\n"
    "\t           send messages via UDP syslog protocol to\n"
    "\t           speciified PORT of HOST. PORT is defaulted to 514\n"
    "\t  none     do not write logs anyware\n"
    "\t           just forward OutputDebugString to driver\n"
    "\t-o, --stdout\n"
    "\t         same as --out:stdout\n"
    "\t-cf      same as --out:file --out:stdout\n"
    "\t--no_out same as --out:none\n"
    "\t--syslog:mtu MTU\n"
    "\t         set maximum syslog packet size to MTU bytes (1024 by default)\n"
    "    Input data stream control:\n"
    "\t--in:<input> [<parameters>]\n"
    "\t         available values for <input> are:\n"
    "\t  drv      read message stream from the driver (default)\n"
    "\t  stdin    read message stream from STDIN\n"
    "\t  file FILENAME\n"
    "\t         read message stream from FILENAME\n"
    "\t  comdbg   read serial port (COM) over WinDbg protocol\n"
    "\t  syslog [HOST][:PORT]\n"
    "\t           get messages via UDP syslog protocol at\n"
    "\t           speciified PORT of HOST. PORT is defaulted to 514,\n"
    "\t           HOST is defaulted to ANY interface of local host\n"
    "\t--stdin  same as --in:stdin\n"
    "\t--in_file FILENAME\n"
    "\t         same as --in:file FILENAME\n"
    "\t--in_drv same as --in:drv\n"
    "\t--comdbg:port NUM\n"
    "\t         use serial port NUM\n"
    "\t         by default used COM1 if " ENV_VALUE_COM_PORT_NAME " env. variable\n"
    "\t         not defined\n"
    "\t--comdbg:baud NUM\n"
    "\t         use baudrate NUM (e.g. 9600, 19200, 57600, 115200)\n"
    "\t         by default used 115200 if " ENV_VALUE_COM_BAUD_NAME " env. variable\n"
    "\t         not defined\n"
    "\t--comdbg:int3 NUM\n"
    "\t         INT3 handling: 0 - do not handle, 1 - skip, 2 - prompt\n"
    "    Service control:\n"
    "\t--svc:install MOD\n"
    "\t         install or uninstall as service. MOD specify startup mode:\n"
    "\t         A - automatic, M - manual, U - uninstall. 'A' is used by default\n"
    "\t         if MOD is omited.\n"
    "\t--run:MOD\n"
    "\t         Specify DbgPrintLog.exe run mode:\n"
    "\t         U, user - run as plain user even under SYSTEM account\n"
    "\t         S, svc  - run as service regardless of user account\n"
    "\t         by default Service mode is assumed if runnyng under SYSTEM account\n"
    "    Driver run mode:\n"
    "\t--drv:install MOD\n"
    "\t         Specify driver startup mode:\n"
    "\t         1 - very first (boot), B - boot, S - system,\n"
    "\t         A - automatic, M - manual, U - uninstall\n"
    "\t         'M' is used by default if MOD is omited.\n"
    "\t--drv:noload, --no_drv\n"
    "\t         do not use driver for message routing\n"
    "\t--drv:reload, -rd\n"
    "\t         restart and reinstall driver before start\n"
    "\t--drv:reconfig, -rc\n"
    "\t         make driver to read config changes from Registry\n"
    "\t--drv:opt OPTION_NAME VALUE\n"
    "\t or --drv_opt OPTION_NAME VALUE\n"
    "\t         Specify driver startup option. Valid OPTION_NAMEs are\n"
    "\t  CheckIrql                0|1\n"
    "\t  BufferSize               NUM (in KBytes, integral multiple of 2)\n"
    "\t  DoNotPassMessagesDown    0|1\n"
    "\t  StopOnBufferOverflow     0|1|2 (2 invokes Kernel Debugger on overflow)\n"
    "\t  TimeStampType            0|1|2\n"
    "\t  AggregateMessages        0|1\n"
    "\t  DumpStackFramePtr        0|1\n"
    "\t         Read documentation for each option description\n"
    "    Misc options:\n"
    "\t--status display information about installed and active components\n"
    "\t--nowait do not wait for new messages, just dump already stored in driver and exit\n"
//    "\t-chcfg   Make config changes only and exit. Do not start log capturing\n"
    "\t-h, -?, /?, /h, --help\n"
    "\t         display this help message\n"
//    "\t-h OPT   display help for driver option OPT\n"
    ;

void
usage(void)
{
    HANDLE stdOuth = GetStdHandle(STD_OUTPUT_HANDLE);
    ULONG WrittenBytes;

    if(stdOuth && stdOuth != ((HANDLE)(-1))) {
        WriteFile(stdOuth, help_text,  strlen(help_text),  &WrittenBytes, NULL);
        WriteFile(stdOuth, "----------------\n",  17,  &WrittenBytes, NULL);
        WriteFile(stdOuth, help_text2, strlen(help_text2), &WrittenBytes, NULL);
    } else {
        MessageBox(NULL, help_text, "Usage", MB_OK);
    }
} // end usage()

BOOL
ReportStatusToSCMgr(
    DWORD dwCurrentState,
    DWORD dwWin32ExitCode,
    DWORD dwWaitHint
    )
{
    static DWORD dwCheckPoint = 1;
    BOOL fResult = TRUE;

    if (dwCurrentState == SERVICE_START_PENDING) {
        ssStatus.dwControlsAccepted = 0;
    } else {
        ssStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE;
    }

    ssStatus.dwCurrentState = dwCurrentState;
    ssStatus.dwWin32ExitCode = dwWin32ExitCode;
    ssStatus.dwWaitHint = dwWaitHint;

    if ( ( dwCurrentState == SERVICE_RUNNING ) ||
         ( dwCurrentState == SERVICE_STOPPED ) ) {
        ssStatus.dwCheckPoint = 0;
    } else {
        ssStatus.dwCheckPoint = dwCheckPoint++;
    }

    // Report the status of the service to the service control manager.
    if (!(fResult = SetServiceStatus( sshStatusHandle, &ssStatus))) {
        AddToMessageLog(TEXT("SetServiceStatus"));
    }
    return fResult;
} // end ReportStatusToSCMgr()

VOID
AddToMessageLog(
    LPTSTR lpszMsg
    )
{
    CHAR    szMsg[256];
    HANDLE  hEventSource;
    LPSTR   lpszStrings[2];
    ULONG   dwErr;

    dwErr = GetLastError();

    print_err(szMsg, "%s error: %d", NT_DbgPrnHk_Client_SVC_NAME, dwErr);
    // Use event logging to log the error.
    hEventSource = RegisterEventSource(NULL, NT_DbgPrnHk_Client_SVC_NAME);

    sprintf(szMsg, "%s error: %d", NT_DbgPrnHk_Client_SVC_NAME, dwErr);
    lpszStrings[0] = szMsg;
    lpszStrings[1] = lpszMsg;

    if (!hEventSource)
        return;

    ReportEvent(hEventSource, // handle of event source
        EVENTLOG_ERROR_TYPE,  // event type
        0,                    // event category
        0,                    // event ID
        NULL,                 // current user's SID
        2,                    // strings in lpszStrings
        0,                    // no bytes of raw data
        (const char **)lpszStrings,          // array of error strings
        NULL);                // no raw data

    (VOID)DeregisterEventSource(hEventSource);

} // end AddToMessageLog()

void
WINAPI
service_ctrl(
    DWORD dwCtrlCode)
{
    ULONG sst;
    // Handle the requested control code.
    switch(dwCtrlCode)
    {
    // Stop the service.

    // SERVICE_STOP_PENDING should be reported before
    // setting the Stop Event - hServerStopEvent - in
    // ServiceStop().  This avoids a race condition
    // which may result in a 1053 - The Service did not respond...
    // error.
    case SERVICE_CONTROL_STOP:
        ReportStatusToSCMgr(SERVICE_STOP_PENDING, NO_ERROR, 10000);
        esc_key = TRUE;
        break;

    case SERVICE_CONTROL_PAUSE:
        ReportStatusToSCMgr(SERVICE_PAUSED, NO_ERROR, 0);
        pause_key = TRUE;
        break;

    case SERVICE_CONTROL_CONTINUE:
        ReportStatusToSCMgr(SERVICE_RUNNING, NO_ERROR, 0);
        pause_key = FALSE;
        break;

    default:
    case SERVICE_CONTROL_INTERROGATE:
        if(esc_key) {
            sst = SERVICE_STOP_PENDING;
        } else
        if(pause_key) {
            sst = SERVICE_PAUSED;
        } else {
            sst = SERVICE_RUNNING;
        }
        ReportStatusToSCMgr(sst, NO_ERROR, 0);
        break;
    }
} // service_ctrl()

void
WINAPI
service_main(
    DWORD dwArgc,
    LPTSTR *lpszArgv)
{
    // register our service control handler:
    sshStatusHandle = RegisterServiceCtrlHandler( NT_DbgPrnHk_Client_SVC_NAME, service_ctrl);
    print_log("***** DbgPrintLog service start********");
    if (!sshStatusHandle) {
        print_err("***** No service control handle ********");
        goto cleanup;
    }

    // SERVICE_STATUS members that don't change in example
    ssStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    ssStatus.dwServiceSpecificExitCode = 0;


    // report the status to the service control manager.
    if (!ReportStatusToSCMgr(
        SERVICE_START_PENDING, // service state
        NO_ERROR,              // exit code
        10000)) {                // wait hint
        print_err("***** ReportStatusToSCMgr() err ********");
        goto cleanup;
    }

    // get messages from user- and kernel-mode and store them to log-file
    get_messages();

cleanup:

    print_err("***** cleaning up... ********");
    // try to report the stopped status to the service control manager.
    if (sshStatusHandle)
        (VOID)ReportStatusToSCMgr(
                            SERVICE_STOPPED,
                            NO_ERROR,
                            0);

    return;
} // end service_main()

BOOLEAN
deinstall_very_first()
{
    int k;
    HKEY hKey;
    PULONG v;
    ULONG t;
    int i;
    BOOLEAN updated = FALSE;

    // save load order in registry
    if(RegOpenKeyW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\GroupOrderList", &hKey) == ERROR_SUCCESS) {
        k = 0;
        RegQueryValueExW(hKey, L"System Bus Extender", NULL, &t, NULL, (PULONG)&k);
        if(k) {
            v = (PULONG)GlobalAlloc(GMEM_FIXED, k);
            RegQueryValueExW(hKey, L"System Bus Extender", NULL, &t, (PUCHAR)v, (PULONG)&k);
            i = 1;
            while(i*(int)sizeof(ULONG) < k) {
                if(v[i] == NT_DBGPRNHK_DRV_TAG) {
                    v[0]--;
                    memmove(&(v[i]), &(v[i+1]), k-(i+1)*sizeof(ULONG));
                    k -= sizeof(ULONG);
                    updated = TRUE;
                } else {
                    i++;
                }
            }
            if(updated) {
                if(RegSetValueExW(hKey, L"System Bus Extender", NULL, REG_BINARY, (PUCHAR)v, k) != ERROR_SUCCESS) {
                    print_log("Error: Can't store driver load order in registry\n");
                }
            }
            GlobalFree(v);
        }
        RegCloseKey(hKey);
    } else {
        print_log("Error: Can't store driver load order in registry\n");
        return FALSE;
    }
    return TRUE;
} // end deinstall_very_first()

BOOLEAN
remove_driver()
{
    int k;
    HANDLE h;
    ULONG returned;

    deinstall_very_first();

    k = NtServiceIsRunning(NT_DbgPrnHk_SVC_NAME);
    if(k<0) {
        NtServiceSetStartMode(NT_DbgPrnHk_SVC_NAME, 4);
        return TRUE;
    }
    if(k == 1) {
        // Started, connect to driver
        h = CreateFile(NT_DbgPrnHk_USER_NAME, GENERIC_READ | GENERIC_WRITE,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL/* | FILE_FLAG_NO_BUFFERING*/,  NULL);
        if(!h || h == (HANDLE)(-1)) {
            print_log("Error: can't open driver\n");
        } else {
            // check if driver can be unloaded
            if(!DeviceIoControl(h,IOCTL_DbgPrnHk_CanUnload,
                                NULL,0,
                                NULL,0,
                                &returned,NULL)) {
                print_log("Warning: DbgPrnHk driver cannot be unloaded, will be disabled\n");
                NtServiceSetStartMode(NT_DbgPrnHk_SVC_NAME, 4);
                CloseHandle(h);
                return FALSE;
            } else {
                k = NtServiceStop(NT_DbgPrnHk_SVC_NAME, 1);
            }
        }
    } else
    if(k == 0) {
        NtServiceSetStartMode(NT_DbgPrnHk_SVC_NAME, 4);
    }

    k = NtServiceRemove(NT_DbgPrnHk_SVC_NAME);
    if(k == 1) {
        print_log("DbgPrnHk driver is uninstalled\n");
        return TRUE;
    }
    return FALSE;
} // end remove_driver()

BOOLEAN
AmIStarted()
{
    HANDLE h;
    ULONG ErrCode;

    h = CreateEvent(NULL, FALSE, FALSE, "DbgPrintLogEvent1");
    if(!h)
        return FALSE;
    ErrCode = GetLastError();
    if(ErrCode == ERROR_ALREADY_EXISTS)
        return TRUE;
    return FALSE;
} // end AmIStarted()

void main () {
//    int argc;
//    WCHAR** argv;
    WCHAR*  CmdLine = NULL;
    WCHAR*  CmdLine2;

    LONG i=0;
//    LONG j;
    LONG k;
    ULONG t;
    ULONG thId;
    HKEY hKey;
    PCHAR sPtr;
    PWCHAR tmpWDir = NULL;

    print_log("DbgPrintLog v%d.%d%s (c) by Alter\nHome site http://www.alter.org.ua\n",
        PROD_VER_MJ, PROD_VER_MN, PROD_VER_SUB);
    fflush(stdout);

    if(GetVersion() & 0x80000000) {
        print_err("For Windows NT family only.\n");
        exit(-2);
    }

    // init with default settings
    #define DBGPRINT_OPT_RAW_INIT
    #include "dbgprint_opt_list.h"

    {
        PUCHAR EnvValue;
        ULONG i;
        if (EnvValue = (PUCHAR)getenv(ENV_VALUE_COM_BAUD_NAME)) {
            g_opt.input_comdbg_baud = atoi((const char*)EnvValue);
        }
        if (EnvValue = (PUCHAR)getenv(ENV_VALUE_COM_PORT_NAME)) {
            i=0;
            while(EnvValue[i] && (EnvValue[i] < '0' || EnvValue[i] > '9')) {
                i++;
            }
            if(EnvValue[i]) {
                g_opt.input_comdbg_port = atoi((const char*)EnvValue+i);
            }
        }
    }

//    __asm int 3;

    // check if we are running as service
    sys_account = IsSystemAccount();

    // get current directory
    k = GetCurrentDirectoryW(0, NULL);
    if(k) {
        g_opt.WDir = (PWCHAR)GlobalAlloc(GMEM_FIXED, (k+1)*sizeof(WCHAR));
        g_opt.WDir[0] = 0;
        k = GetCurrentDirectoryW(k, g_opt.WDir);
        g_opt.WDir[k] = 0;
    }
retry_sys_acc:
    if(sys_account) {
        // if we are running as service get command line and working directory from registry

/*        k = MAX_PATH*3;
        CmdLine = (PWCHAR)GlobalAlloc(GMEM_FIXED, k);
        if(!CmdLine) {
            exit(-3);
        }*/

        CmdLine = GetCommandLineW();
        if(CmdLine) {
            k = DbgPrint_CmdLine_to_OptStruct(CmdLine, FALSE, (PDBGDUMP_OPT)&g_opt);
            if(k == 1 && g_opt.user_run) {
                sys_account = IsSystemAccount();
                if(!sys_account) {
                    goto retry_sys_acc;
                }
            }
            // re-init with default settings
            #define DBGPRINT_OPT_RAW_INIT
            #include "dbgprint_opt_list.h"
        }

        if(RegOpenKey(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\" NT_DbgPrnHk_Client_SVC_NAME, &hKey) != ERROR_SUCCESS) {
            print_err("Can't open '%s' registry key\n", "System\\CurrentControlSet\\Services\\" NT_DbgPrnHk_Client_SVC_NAME);
            exit(-4);
        }
        k = 0;
        if(RegQueryValueExW(hKey, NT_DbgPrnHk_Reg_CmdLineW, NULL, &t, NULL, (PULONG)&k) == ERROR_SUCCESS) {
            CmdLine = (PWCHAR)GlobalAlloc(GMEM_FIXED, k+sizeof(WCHAR));
        }
        if(!k || ! CmdLine ||
           RegQueryValueExW(hKey, NT_DbgPrnHk_Reg_CmdLineW, NULL, &t, (PUCHAR)CmdLine, (PULONG)&k) != ERROR_SUCCESS) {
            if(CmdLine)
                GlobalFree(CmdLine);
            CmdLine = GetCommandLineW();
        }
        k = 0;
        if(RegQueryValueExW(hKey, NT_DbgPrnHk_Reg_WDirW, NULL, &t, NULL, (PULONG)&k) == ERROR_SUCCESS) {
            tmpWDir = (PWCHAR)GlobalAlloc(GMEM_FIXED, k+sizeof(WCHAR));
        }
        if(k && tmpWDir && RegQueryValueExW(hKey, NT_DbgPrnHk_Reg_WDirW, NULL, &t, (PUCHAR)tmpWDir, (PULONG)&k) == ERROR_SUCCESS) {
            g_opt.WDir = tmpWDir;
        } else {
            if(tmpWDir)
                GlobalFree(tmpWDir);
        }
        RegCloseKey(hKey);
    } else {
        CmdLine = GetCommandLineW();
        if(!CmdLine) {
            print_err("Can't get command line arguments\n");
            exit(-1);
        }
    }
    if(CmdLine) {
        g_CmdLine = CmdLine;
    }

    GetModuleFileName(NULL, PathToExec, sizeof(PathToExec));
    sPtr = strrchr(PathToExec, '\\');
    if(sPtr) {
        (*sPtr) = 0;
    }

    k = DbgPrint_CmdLine_to_OptStruct(CmdLine, FALSE, (PDBGDUMP_OPT)&g_opt);
    if(k != 1) {
        if(!k)
            usage();
        exit(k);
    }


    StdIn  = GetStdHandle(STD_INPUT_HANDLE);
    StdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    // gain privileges to install/load driver/service
    if(!Privilege(SE_LOAD_DRIVER_NAME, TRUE)) {
        print_log("Warning: Insufficient privileges\n");
    }

//    if(sys_account) {
//        continue;
//    }

/*
    // do nothing if this was service/driver deinstall
    if(deinstall) {
        exit(0);
    }
*/
    // set current directory
    SetCurrentDirectoryW(g_opt.WDir);

    if(!sys_account && g_opt.display_status) {
        ULONG StartMode_drv;
        ULONG StartMode_svc;

        print_log("DbgPrnHk driver:     ");
        k = NtServiceGetStartMode(NT_DbgPrnHk_SVC_NAME, &StartMode_drv);
        if(k == 0) {
            k = NtServiceIsRunning(NT_DbgPrnHk_SVC_NAME);
            if(k == 1) {
                print_log("Active      (%d)", StartMode_drv);
                if(get_drv_conf()) {
                    print_log("\n");
                    dp_dump_st(&st);
                }
            } else
            if(k == 0) {
                print_log("Stopped     (%d)", StartMode_drv);
            } else {
                print_log("Unavailable (%d)", StartMode_drv);
            }
        } else {
            print_log("Unavailable");
        }
        print_log("\n");
        print_log("DbgPrintLog service: ");
        k = NtServiceGetStartMode(NT_DbgPrnHk_Client_SVC_NAME, &StartMode_svc);
        if(k == 0) {
            k = NtServiceIsRunning(NT_DbgPrnHk_Client_SVC_NAME);
            if(k == 1) {
                print_log("Active      (%d)", StartMode_svc);
            } else
            if(k == 0) {
                print_log("Stopped     (%d)", StartMode_svc);
            } else {
                print_log("Unavailable (%d)", StartMode_svc);
            }
        } else {
            print_log("Unavailable");
        }
        print_log("\n");
        print_err("Client instance:     %sfound\n", AmIStarted() ? "" : "not ");
        exit(0);
    }

    if(!sys_account && g_opt.input_driver) {
        // ensure that driver is installed

        if(g_opt.install_svc && g_opt.svc_mode != SERVICE_DISABLED) {
            g_opt.install_drv = TRUE;
        }

        if(g_opt.install_drv) {
            // install/deinstall or update startup mode
            if(g_opt.drv_mode == SERVICE_DISABLED) {
                remove_driver();
            } else {
                k = NtServiceIsRunning(NT_DbgPrnHk_SVC_NAME);
                if(k<0) {
                    remove_driver();
                    k = NtServiceInstall(NT_DbgPrnHk_SVC_NAME, PathToExec, TRUE, g_opt.drv_mode);
                    if(k < 0) {
                        print_err("Error: Can't install DbgPrnHk driver\n");
                        exit(-1);
                    }
                } else {
                    // update .SYS
                    NtServiceInstall(NT_DbgPrnHk_SVC_NAME, PathToExec, TRUE, g_opt.drv_mode);
                    // update start mode
                    NtServiceSetStartMode(NT_DbgPrnHk_SVC_NAME, g_opt.drv_mode);
                }
                // save load options in registry
                if(RegOpenKeyW(HKEY_LOCAL_MACHINE, NT_DBGPRNHK_REG_PATH, &hKey) != ERROR_SUCCESS) {
                    print_err("Error: Can't store driver parameters in registry\n");
                    exit(-4);
                }
                k = sizeof(ULONG);
                i = g_opt.drv_mode; // load mode
                if(RegSetValueExW(hKey, L"Start", NULL, REG_DWORD, (PUCHAR)&i, k) != ERROR_SUCCESS) {
                    print_err("Error: Can't store driver parameters in registry\n");
                    exit(-4);
                }
                if(g_opt.drv_very_first) {
                    PULONG v = NULL;

                    // make clean
                    deinstall_very_first();

                    // make install
                    // ;)
                    k = sizeof(ULONG);
                    i = NT_DBGPRNHK_DRV_TAG; // our Tag
                    if(RegSetValueExW(hKey, NT_DBGPRNHK_REG_TAG_VAL, NULL, REG_DWORD, (PUCHAR)&i, k) != ERROR_SUCCESS) {
                        print_err("Error: Can't store driver parameters in registry\n");
                        exit(-4);
                    }
                    k = sizeof(L"System Bus Extender");
                    if(RegSetValueExW(hKey, NT_DBGPRNHK_REG_GROUP_VAL, NULL, REG_SZ, (PUCHAR) L"System Bus Extender", k) != ERROR_SUCCESS) {
                        print_err("Error: Can't store driver parameters in registry\n");
                        exit(-4);
                    }
        /*
                    // do not set TSTAMP_TYPE directly to override explicitly set value (if any)
                    for(j=0; DbgPrintHook_Options[j].OptionName; j++) {
                        if(!wcsicmp(DbgPrintHook_Options[j].OptionName, NT_DBGPRNHK_REG_TSTAMP_TYPE_VAL)) {
                            DbgPrintHook_Options[j].Value.OptDword = TimeStampType_RdtscPerfCounter;
                            DbgPrintHook_Options[j].UpdateFlag = TRUE;
                            drv_opt = TRUE;
                            break;
                        }
                    }
        */
                    g_opt.TimeStampType = TimeStampType_RdtscPerfCounter;
                    g_opt.TimeStampType_upd = TRUE;
                    g_opt.drv_opt = TRUE;

                    RegCloseKey(hKey);
                    // save load order in registry
                    if(RegOpenKeyW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\GroupOrderList", &hKey) != ERROR_SUCCESS) {
                        print_err("Error: Can't store driver load order in registry\n");
                        exit(-4);
                    }
                    k = 0;
                    RegQueryValueExW(hKey, L"System Bus Extender", NULL, &t, NULL, (PULONG)&k);
                    if(!k)
                        k+=sizeof(ULONG);
                    v = (PULONG)GlobalAlloc(GMEM_FIXED, k+sizeof(ULONG));
                    memset(v, 0, k+sizeof(ULONG));
                    RegQueryValueExW(hKey, L"System Bus Extender", NULL, &t, (PUCHAR)v, (PULONG)&k);
                    if(!k)
                        k+=sizeof(ULONG);
                    if(v[1] != NT_DBGPRNHK_DRV_TAG) {
                        v[0]++;
                        memmove(&(v[2]), &(v[1]), k-sizeof(ULONG));
                        v[1] = NT_DBGPRNHK_DRV_TAG;
                        if(RegSetValueExW(hKey, L"System Bus Extender", NULL, REG_BINARY, (PUCHAR)v, k+sizeof(ULONG)) != ERROR_SUCCESS) {
                            print_err("Error: Can't store driver load order in registry\n");
                            exit(-4);
                        }
                    }
                    GlobalFree(v);
                } else {
                    RegDeleteValueW(hKey, NT_DBGPRNHK_REG_TAG_VAL);
                    RegDeleteValueW(hKey, NT_DBGPRNHK_REG_GROUP_VAL);
                    deinstall_very_first();
                }
                RegCloseKey(hKey);
            }
        }
        if(g_opt.drv_opt) {
            if(RegOpenKeyW(HKEY_LOCAL_MACHINE, NT_DBGPRNHK_REG_PATH, &hKey) != ERROR_SUCCESS) {
                print_err("Error: Can't store driver parameters in registry\n");
                exit(-4);
            }
            // write driver settings to registry
            #define DBGPRINT_DRV_OPT_RAW_WRITE_REG
            #include "dbgprint_opt_list.h"

    /*
            for(j=0; DbgPrintHook_Options[j].OptionName; j++) {
                if(!DbgPrintHook_Options[j].UpdateFlag)
                    continue;
                k = sizeof(ULONG);
                i = DbgPrintHook_Options[j].Value.OptDword;
                if(RegSetValueExW(hKey, DbgPrintHook_Options[j].OptionName, NULL, REG_DWORD, (PUCHAR)&i, k) != ERROR_SUCCESS) {
                    print_log("Error: Can't store driver parameters in registry\n");
                    exit(-4);
                }
            }
    */
            RegCloseKey(hKey);
        }
        if(g_opt.install_svc) {
            if(g_opt.svc_mode == SERVICE_DISABLED) {
                k = NtServiceStop(NT_DbgPrnHk_Client_SVC_NAME, 10);
                k = NtServiceRemove(NT_DbgPrnHk_Client_SVC_NAME);
                if(k == 1) {
                    print_log("DbgPrintLog service is uninstalled\n");
                }
            } else {
                PWCHAR ImagePath;

                // try to install (or re-install) DbgPrint capturing service
                k = NtServiceStop(NT_DbgPrnHk_Client_SVC_NAME, 10);
                k = NtServiceRemove(NT_DbgPrnHk_Client_SVC_NAME);
                k = NtServiceInstall(NT_DbgPrnHk_Client_SVC_NAME, PathToExec, FALSE, g_opt.svc_mode);
                if(k < 0) {
                    print_err("Error: Can't install DbgPrintLog service\n");
                    exit(-1);
                }
                // save command line in registry
                if(RegOpenKey(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\" NT_DbgPrnHk_Client_SVC_NAME, &hKey) != ERROR_SUCCESS) {
                    print_err("Error: Can't store parameters in registry\n");
                    exit(-4);
                }
                CmdLine2 = GetCommandLineW();
                if(!CmdLine2) {
                    print_err("Can't get command line arguments\n");
                    exit(-1);
                }
                k = (wcslen(CmdLine2)+1)*sizeof(WCHAR);
                if(RegSetValueExW(hKey, NT_DbgPrnHk_Reg_CmdLineW, NULL, REG_SZ, (PUCHAR)CmdLine2, k) != ERROR_SUCCESS) {
                    print_err("Error: Can't store parameter '%S' in registry\n", NT_DbgPrnHk_Reg_CmdLineW);
                    exit(-4);
                }
                // save working directory
                k = (wcslen(g_opt.WDir)+1)*sizeof(WCHAR);
                if(RegSetValueExW(hKey, NT_DbgPrnHk_Reg_WDirW, NULL, REG_SZ, (PUCHAR)g_opt.WDir, k) != ERROR_SUCCESS) {
                    print_err("Error: Can't store parameter '%S' in registry\n", NT_DbgPrnHk_Reg_WDirW);
                    exit(-4);
                }
                // update image path with --run:MOD
                k = (strlen("System32\\" NT_DbgPrnHk_Client_SVC_NAME ".exe --run:S")+1)*sizeof(WCHAR);
                ImagePath = (PWCHAR)GlobalAlloc(GMEM_FIXED, k);
                swprintf(ImagePath, L"%S", "System32\\" NT_DbgPrnHk_Client_SVC_NAME ".exe --run:S");
                if(RegSetValueExW(hKey, NT_DbgPrnHk_Reg_ImagePathW, NULL, REG_SZ, (PUCHAR)ImagePath, k) != ERROR_SUCCESS) {
                    print_err("Error: Can't store parameter '%S' in registry\n", NT_DbgPrnHk_Reg_ImagePathW);
                    exit(-4);
                }
                GlobalFree(ImagePath);
                RegCloseKey(hKey);
            }
        }

        if((g_opt.install_drv || g_opt.drv_opt) && g_opt.install_svc) {
            print_log("Changes will take effect after reboot or driver and service restart\n");
            print_log("Use \n"
                      "  net stop  DbgPrintLog\n"
                      "  net stop  DbgPrnHk\n"
                      "  net start DbgPrnHk\n"
                      "  net start DbgPrintLog\n"
                      "to apply changes immediately\n");
            exit(0);
        } else
        if(g_opt.install_svc) {
            print_log("Changes will take effect after reboot or service restart\n");
            print_log("Use \n"
                      "  net stop  DbgPrintLog\n"
                      "  net start DbgPrintLog\n"
                      "to apply changes immediately\n");
            exit(0);
        } else
        if(g_opt.install_drv || g_opt.drv_opt) {
            print_log("Changes will take effect after reboot or driver restart\n");
            print_log("Use \n"
                      "  net stop  DbgPrnHk\n"
                      "  net start DbgPrnHk\n"
                      "to apply changes immediately\n");
            exit(0);
        }
    }

    if(g_opt.change_config) {
        update_drv_conf();
        exit(0);
    }

    if(!g_opt.input_driver) {
        // avoid multiple instances
        if(!g_opt.input_driver && AmIStarted()) {
            print_err("Multiple instances not allowed.\n");
            exit(-2);
        }

        // init lock for switching buffers
        hODS_lock = CreateMutex(NULL, FALSE, NULL);
        if(!hODS_lock || hODS_lock == (HANDLE)(-1)) {
            print_err("can't init mutex, (%#x)\n",
                GetLastError());
            exit(-1);
        }
    }
    if(sys_account) {
        // init as service
        if (!StartServiceCtrlDispatcher(dispatchTable)) {
            AddToMessageLog("StartServiceCtrlDispatcher failed.");
            exit(-1);
        }
    } else {
        // start keyboard event handling thread
        CreateThread(NULL, 0, get_kbd, NULL, 0, &thId);
        // get messages from user- and kernel-mode and store them to log-file
        get_messages();
    }

    exit(0);
} // end main()

