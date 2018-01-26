// dbghk_exts.cpp : Defines the entry point for the DLL application.
//


#include "stdafx.h"

#include "DbgPrnHk.h"
#include "..\client\dbgprint_opt.h"
#include "..\sdk\formatmsg\fmt_output.h"
#include "..\inc\tools.h"

#define malloc(sz)  GlobalAlloc(GMEM_FIXED, sz)
#define free(addr)  GlobalFree(addr)

#ifdef DBG
#define TRACE(x)   dprintf x
#else
#define TRACE(x) {;}
#endif //DBG

WINDBG_EXTENSION_APIS   ExtensionApis;

// this is EntryPoint in WinDbg environment
// however, it will be called after DllMain
VOID
WinDbgExtensionDllInit(
    PWINDBG_EXTENSION_APIS PassedApis,
    USHORT Major,
    USHORT Minor
    )
{
    memcpy(&ExtensionApis,PassedApis,sizeof(WINDBG_EXTENSION_APIS));
} // end WinDbgExtensionDllInit()

EXT_API_VERSION ExtapiVersion = { 5,5,0,0 };

LPEXT_API_VERSION
ExtensionApiVersion(VOID) {
    return &ExtapiVersion;
} // end ExtensionApiVersion()

DBGDUMP_OPT_RAW g_opt;

DWORD dwAddr_st = 0;
PDBGPRNHK_INTERNAL_STATE st = NULL;
ULONG SavedBufferSize = 0;
PDbgPrnHk_GetMessages_USER_OUT MsgBuffer = NULL;
PULONG MsgBuffer_PageMap = NULL;

ULONG saved_s;
ULONG saved_n;
//BOOLEAN g_raw_mode = FALSE;

BOOL
WINAPI
DllMain(
    IN HANDLE DllHandle,
    IN DWORD  Reason,
    IN LPVOID Reserved
    )
{

    BOOL b;

    UNREFERENCED_PARAMETER(Reserved);

    b = TRUE;

    switch(Reason) {

    case DLL_PROCESS_ATTACH:

        dwAddr_st = 0;
        st = NULL;
        MsgBuffer = NULL;
        MsgBuffer_PageMap = NULL;
        memset(&g_opt, 0, sizeof(g_opt));
        saved_s = 0;
        saved_n = 20;
        break;

    case DLL_THREAD_ATTACH:

        break;

    case DLL_PROCESS_DETACH:

        if(st) {
            free(st);
            st = NULL;
        }
        if(MsgBuffer) {
            free(MsgBuffer);
            MsgBuffer = NULL;
        }
        if(MsgBuffer_PageMap) {
            free(MsgBuffer_PageMap);
            MsgBuffer_PageMap = NULL;
        }
        break;

    case DLL_THREAD_DETACH:

        break;
    }

    return (b);
} // end DllMain()

PCHAR*
CommandLineToArgvA(
    PCHAR CmdLine,
    int* _argc
    )
{
    PCHAR* argv;
    PCHAR  _argv;
    ULONG   len;
    ULONG   argc;
    CHAR    a;
    ULONG   i, j;

    BOOLEAN  in_QM;
    BOOLEAN  in_TEXT;
    BOOLEAN  in_SPACE;

    len = strlen(CmdLine);
    i = ((len+2)/2)*sizeof(PVOID) + sizeof(PVOID);

    argv = (PCHAR*)malloc(
        i + (len+2)*sizeof(CHAR));

    _argv = (PCHAR)(((PUCHAR)argv)+i);

    argc = 0;
    argv[argc] = _argv;
    in_QM = FALSE;
    in_TEXT = FALSE;
    in_SPACE = TRUE;
    i = 0;
    j = 0;

    while( a = CmdLine[i] ) {
        if(in_QM) {
            if(a == '\"') {
                in_QM = FALSE;
            } else {
                _argv[j] = a;
                j++;
            }
        } else {
            switch(a) {
            case '\"':
                in_QM = TRUE;
                in_TEXT = TRUE;
                if(in_SPACE) {
                    argv[argc] = _argv+j;
                    argc++;
                }
                in_SPACE = FALSE;
                break;
            case ' ':
            case '\t':
            case '\n':
            case '\r':
                if(in_TEXT) {
                    _argv[j] = '\0';
                    j++;
                }
                in_TEXT = FALSE;
                in_SPACE = TRUE;
                break;
            default:
                in_TEXT = TRUE;
                if(in_SPACE) {
                    argv[argc] = _argv+j;
                    argc++;
                }
                _argv[j] = a;
                j++;
                in_SPACE = FALSE;
                break;
            }
        }
        i++;
    }
    _argv[j] = '\0';
    argv[argc] = NULL;

    (*_argc) = argc;
    return argv;
} // end CommandLineToArgvA()

void
dpvmsg_help()
{
    dprintf("View DbgPrint Dump saved messages\n");
    dprintf("Usage:\n");
    dprintf("  ls [-h] [--info] [-a] [<output format switches>] [<1st message> [<last message> | -n <NUM messages>]]\n");
    dprintf("    -h           show this help messages\n");
    dprintf("    -a           show all messages and summary info\n");
    dprintf("    -e           show messages from entire buffer, including those are passed\n");
    dprintf("                   up to service and potentially saved to disk \n");
    dprintf("    --info       show summary info only (default)\n");
    dprintf("    --start NUM  show starting from NUM message in buffer\n");
    dprintf("    --count NUM  show not more than NUM messages\n");
    dprintf("    --last NUM   show last NUM messages\n");
    dprintf("    --next       show next NUM messages, where NUM is set by previous --count\n");
    dprintf("Format switches:\n");
    dprintf("    -m           show show initiator mode (K - kernel, U - user)\n");
    dprintf("    -p           show Process ID\n");
    dprintf("    -t           show Thread ID\n");
    dprintf("    -i           show IRQL\n");
    dprintf("    -cpu         show CPU number\n");
    dprintf("    --full       same as -m -p -t -i\n");
    dprintf("    -T           show timestamp\n");
    dprintf("    -fm M        do not show messages from <M> mode (<M> can be K or U)");

    return;
} // end dpvmsg_help()
    
void
dpsmsg_help()
{
    dprintf("Save DbgPrint Dump saved messages to file\n");
    dprintf("Usage:\n");
    dprintf("  save [-h] [-a] [<1st message> [<last message> | -n <NUM messages>]] -f <FILENAME>\n");
    dprintf("    -h           show this help messages\n");
    dprintf("    --file NAME  name of the file for saving messages\n");
    dprintf("    -a           save all messages\n");
    dprintf("    -e           save messages from entire buffer, including those are passed\n");
    dprintf("                   up to service and potentially saved to disk \n");
    dprintf("    --start NUM  save starting from NUM message in buffer\n");
    dprintf("    --count NUM  save not more than NUM messages\n");
    dprintf("    --last NUM   save last NUM messages\n");
    return;
} // end dpsmsg_help()
    
void
lsig_help()
{
    dprintf("Locate DbgPrint Dump message buffer in physical memory\n");
    dprintf("Usage:\n");
    dprintf("  lsig --max MAXMEM   look for signature below MAXMEM Mb\n");
    dprintf("  lsig ADDR           check for signature at ADDR virtual address\n");
    dprintf("  lsig -p ADDR        check for signature at ADDR physical address\n");
    dprintf("  lsig -h             show this help messages\n");
    dprintf("  lsig                looks for signature through 1Gb\n");
    return;
} // end lsig_help()
    
void
drvopt_help()
{
    dprintf("Modify DbgPrint Dump options on remote machine\n");
    dprintf("Usage:\n");
    dprintf("  drvopt OPTION_NAME VALUE\n"
            "    Specify driver startup option. Valid OPTION_NAMEs are\n"
            "      CheckIrql\n"
            "      DoNotPassMessagesDown\n"
            "      StopOnBufferOverflow\n"
            "      TimeStampType\n"
            "      AggregateMessages\n"
            "    Read documentation for each option description\n"
            );
    dprintf("  drvopt\n"
            "    Display current options\n");
    return;
} // end drvopt_help()
    
void
display_help()
{
    dprintf("Display or save to file contents of video buffer from remote machine\n");
    dprintf("Usage:\n");
    dprintf("  display <options>\n");
    dprintf("    --file FNAME    name of image file for saving\n");
    dprintf("    -t, --text      save CGA/EGA/VGA text buffer at 0xB8000\n");
    dprintf("    -e, --ega16     save EGA/VGA 16-color buffer at 0xA0000\n");
    dprintf("    -b, --bin       save text buffer in raw binary form (char+color)\n");
    dprintf("    -h              show this help messages\n");
    return;
} // end display_help()
    
void
mdump_help()
{
    dprintf("Save to file contents of physical memory range from remote machine\n");
    dprintf("Usage:\n");
    dprintf("  mdump <options>\n");
    dprintf("    --file FNAME    name of image file for saving\n");
    dprintf("    -b, --base      staring (base) physical address\n");
    dprintf("    -e, --end       end physical address\n");
    dprintf("    -m, --mb        assume base/end addresses are in Mbytes\n");
    dprintf("    -h              show this help messages\n");
    return;
} // end mdump_help()
    
void
pause_help()
{
    dprintf("Pause or Resume Logging on remote machine\n");
    dprintf("Usage:\n");
    dprintf("  pause               toggle current state\n");
    dprintf("  pause +             pause loging, further messages shall be dropped\n");
    dprintf("  pause -             resume loging\n");
    return;
} // end pause_help()

#define print_log    dprintf

#include "..\client\dump_state.h"

/*
void
dp_dump_st(
    PDBGPRNHK_INTERNAL_STATE st
    )
{
} // end dp_dump_st()
*/

#define TeeOutput(h, msg, len) \
                    dprintf("%.*s", len, msg);

BOOLEAN
dp_check_buffer_page(
    PDBGPRNHK_INTERNAL_STATE st,
    PDbgPrnHk_GetMessages_USER_OUT MsgBuffer,
    PULONG MsgBuffer_PageMap,
    ULONG offset
    )
{
    ULONG i = offset;
    ULONG b;

    if(!EnvGetBit(MsgBuffer_PageMap, i/PAGE_SIZE)) {
        if(!(i % (1024*1024/16))) {
            if(!(i % (1024*1024*4))) {
                dprintf("\n");
            }
            dprintf(".");
        }
        b = ReadMemory(((ULONG)st->MsgBuffer)+i,
                                  (PUCHAR)(MsgBuffer)+i,
                                  PAGE_SIZE,
                                  NULL);
        if ( !b ) {
            dprintf("Cannot read DbgPrnHk internal message buffer at offset %#x\n", i);
            return FALSE;
        }
        EnvSetBit(MsgBuffer_PageMap, i/PAGE_SIZE);
    }
    return TRUE;
}

HANDLE
dp_open_file_for_save(
    PCHAR fName,
    PCHAR saving_msg,
    PCHAR cant_open_msg
    )
{
    HANDLE h = NULL;
    ULONG returned;
    CHAR buf[3] = "";

    h = CreateFile(fName, 0,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL/* | FILE_FLAG_NO_BUFFERING*/,  NULL);
    if(h && h != (HANDLE)(-1)) {
        CloseHandle(h);
        h = NULL;

        dprintf("File %s already exists\n", fName);
        returned = GetInputLine("Overwrite (y/n)", &buf[0], 3);
        if(buf[0] != 'y' && buf[0] != 'Y') {
            dprintf("Please enter different name\n");
            goto exit;
        }
    }
    dprintf(saving_msg /*"Saving messages to %s\n"*/, fName);
    h = CreateFile(fName, GENERIC_READ | GENERIC_WRITE,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL,
                       CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL/* | FILE_FLAG_NO_BUFFERING*/,  NULL);
    if(!h || h == (HANDLE)(-1)) {
        dprintf(cant_open_msg /*"Cannot create message file\n"*/);
        h = NULL;
        goto exit;
    }
exit:
    return h;
} // end dp_open_file_for_save()

void
dp_dump_msg(
    PDBGPRNHK_INTERNAL_STATE st,
    PDbgPrnHk_GetMessages_USER_OUT MsgBuffer,
    PULONG MsgBuffer_PageMap,
    ULONG s, // may be negative
    ULONG n,
    PCHAR fName
    )
{
    ULONG i;
    PDbgPrnHk_GetMessages_USER_OUT CurMsg;
    HANDLE h = NULL;
    BOOLEAN write_failed = FALSE;
    CHAR buf[3] = "";
    CHAR DbgPrintBuf[PAGE_SIZE*2];
    ULONG CurLogSize=0;
    ULONG len;

    DBGPRN_FORMAT_MESSAGE_CTX msgst;
    DBGPRN_TIMESTAMP_CTX      timest;

    TRACE(("dp_dump_msg:\n"));

    if(fName) {
        h = dp_open_file_for_save(fName, "Saving messages to %s\n", "Cannot create message file\n");
        if(!h) {
            goto exit;
        }
    }

    dprintf("Dumping %d (%x) messages starting from %d (%x) pos in buffer\n",
            n, n,
            (ULONG)s & st->BufferSizeMask, (ULONG)s & st->BufferSizeMask
           );
    if(fName) {
        dprintf("  to %s\n", fName);
    }
    dprintf("use Ctrl-Break to abort\n");

    dbgprint_format_msg_init(&g_opt, &msgst);

    for(i=0; i<n; i++) {

        CurMsg = &MsgBuffer[(s+i) & st->BufferSizeMask];

        if(!dp_check_buffer_page(st, MsgBuffer, MsgBuffer_PageMap, (ULONG)CurMsg - (ULONG)MsgBuffer)) {
            goto exit;
        }

        if(!g_opt.raw_mode) {
            len = dbgprint_format_msg(&g_opt, CurMsg, DbgPrintBuf, &timest, &msgst);
        } else {
            len = sizeof(DbgPrnHk_GetMessages_USER_OUT);
        }
        if(len) {
            if(h) {
                if(!WriteFile(h, g_opt.raw_mode ? (PCHAR)CurMsg : (PCHAR)DbgPrintBuf, len, &len, NULL)) {
                    write_failed = TRUE;
                }
            } else {
                dprintf("%.*s", len, DbgPrintBuf);
            }
        }
        if(CheckControlC()) {
            dprintf("\nStopped by Ctrl-C\n");
            goto exit;
        }
    }
    if(h) {
        if(write_failed) {
            dprintf("\n*ERROR! write failed.\n  %d messages were saved to file\n");
        } else {
            dprintf("\nAll messages saved successfuly\n");
        }
    }
exit:
    if(h) {
        CloseHandle(h);
    }
    TRACE(("dp_dump_msg: exit\n"));
    return;
} // end dp_dump_msg()

int
dp_load_st()
{
    DWORD dwAddr = 0;
    DWORD dwAddr2 = 0;
    BOOL b;

    if(!dwAddr_st) {
        dprintf("Connecting to DbgPrnHk, please wait\n");
        dwAddr = GetExpression("DbgPrnHk!DbgPrnHk_State");
        if ( !dwAddr ) {
            dwAddr = GetExpression("DBGPRNHK.SYS!DbgPrnHk_State");
            if ( !dwAddr ) {
                dwAddr2 = GetExpression("DbgPrnHk!st");
            }
        }
        if ( !dwAddr && !dwAddr2) {
            dprintf("*ERROR! Cannot find DbgPrnHk!DbgPrnHk_State export\n");
            dprintf("  If you are sure that log capturing driver was active\n");
            dprintf("  try !dbgprn.lsig command to scan physical memory for latest logs\n");
            dprintf("  Note, this operation may take same time as full memory dump.\n");
            goto exit;
        }
    } else {
        dwAddr2 = dwAddr_st;
    }

    if(!st) {
        dprintf("Reading DbgPrnHk internal state buffer, please wait\n");
        st = (PDBGPRNHK_INTERNAL_STATE)malloc(sizeof(DBGPRNHK_INTERNAL_STATE));
        if(!st) {
            dprintf("Cannot alloc memory for DbgPrnHk internal state buffer\n");
            goto exit;
        }
        if(dwAddr) {
            b = ReadMemory((ULONG)dwAddr,
                                      &dwAddr2,
                                      sizeof(dwAddr2),
                                      NULL);
            if ( !b ) {
                free(st);
                st = NULL;
                dprintf("Cannot read pointer to DbgPrnHk internal state\n");
                goto exit;
            }
            dwAddr_st = dwAddr2;
        }
        if(!dwAddr2) {
            dprintf("NULL pointer to DbgPrnHk internal state\n");
            goto exit;
        }
        b = ReadMemory((ULONG)dwAddr2,
                                  st,
                                  sizeof(DBGPRNHK_INTERNAL_STATE),
                                  NULL);
        if ( !b ) {
            free(st);
            st = NULL;
            dprintf("Cannot read DbgPrnHk internal state\n");
            goto exit;
        }
    }
    SavedBufferSize = st->BufferSize;
    return 1;
exit:
    return 0;
} // end dp_load_st()

int
dp_upload_st()
{
    BOOL b;
    if(!dwAddr_st) {
        dprintf("DbgPrnHk internal state not loaded\n");
        return 0;
    }
    if(!st) {
        dprintf("local copy of DbgPrnHk internal state not allocated\n");
        return 0;
    }
    st->BufferSize = SavedBufferSize;
    b = WriteMemory((ULONG)dwAddr_st,
                              st,
                              sizeof(DBGPRNHK_INTERNAL_STATE),
                              NULL);
    if ( !b ) {
        dprintf("Cannot update DbgPrnHk internal state\n");
        return 0;
    }
    return 1;
} // end dp_upload_st()

int
dp_x_msg(
    PCHAR args,
    ULONG Action
    )
{
    BOOL b;
    PCHAR* argv = NULL;
    int argc;
    int i, j;
    BOOLEAN View;

    BOOLEAN   start_item_set = FALSE;
    ULONGLONG start_item = 0;
    BOOLEAN   end_item_set = FALSE;
    ULONGLONG end_item = -1;
    ULONG n=0;
    ULONG s=0;
    PCHAR fName = NULL;
    ULONG flags = 0;

    BOOLEAN all_mode = FALSE;
    BOOLEAN all_avl_mode = FALSE;
    BOOLEAN dump_mode = FALSE;
    BOOLEAN info_mode = FALSE;
    BOOLEAN del_msg = FALSE;

    PDBGDUMP_OPT_RAW opt = &g_opt;
    UCHAR a;
    ULONG retval = -1;

    g_opt.raw_mode = FALSE;

    switch(Action) {
    case 0:
        View = FALSE;
        break;
    case 1:
        View = TRUE;
        break;
    case 2:
        View = TRUE;
        del_msg = TRUE;
        break;
    }

    TRACE(("dp_x_msg:\n"));

#define print_log    dprintf

    // Evaluate the argument string to get the address of
    // the string to dump.

    argv = CommandLineToArgvA((PCHAR)args, &argc);
    if(!argv) {
        dprintf("Cannot parse commandline\n");
        goto exit;
    }

    if(!dp_load_st()) {
        goto exit;
    }

    if(del_msg) {
        dprintf("Deleting messages from buffer\n");
        st->ReadPosition.QuadPart = st->WritePosition.QuadPart;
        st->QueueSize = 0;
        if(dp_upload_st()) {
            dprintf("Ok\n");
            retval = 1;
        }
        goto exit;
    }

    end_item = st->WritePosition.QuadPart;

    if(!argc) {
        if(View) {
            dump_mode = TRUE;
        } else {
            dprintf("Target file name required\n");
            goto exit;
        }
    }

    for(i=0; i<argc; i++) {
        if(!argv[i]) {
            continue;
        }
        if(argv[i][0] != '-') {
            if(!start_item_set) {
                sscanf(argv[i], "%I64d", &start_item);
                start_item_set = TRUE;
            } else
            if(!end_item_set) {
                sscanf(argv[i], "%I64d", &end_item);
                end_item_set = TRUE;
            } else {
                dprintf("Invalid parameter %s\n", argv[i]);
                goto exit;
            }
            continue;
        }
        if(!strcmp(argv[i], "-h") ||
           !strcmp(argv[i], "-?") ||
           !strcmp(argv[i], "//h") ||
           !strcmp(argv[i], "//?") ||
           !strcmp(argv[i], "--help")) {
            if(View) {
                dpvmsg_help();
            } else {
                dpsmsg_help();
            }
            goto exit;
        } else
        if(!strcmp(argv[i], "-a")) {
            all_mode = TRUE;
            dump_mode = TRUE;
        } else
        if(!strcmp(argv[i], "--all_avl") ||
           !strcmp(argv[i], "-e") ||
           !strcmp(argv[i], "--entire_buffer")) {
            all_mode = TRUE;
            all_avl_mode = TRUE;
            dump_mode = TRUE;
        } else
        if(!strcmp(argv[i], "--info")) {
            if(!View) {
                goto unrec;
            }
            info_mode = TRUE;
            dump_mode = FALSE;
        } else
        if(!strcmp(argv[i], "--raw")) {
            if(View) {
                goto unrec;
            }
            g_opt.raw_mode = TRUE;
        } else
        if(!strcmp(argv[i], "--file")) {
            if(View) {
                goto unrec;
            }
            i++;
            if(i >= argc) {
                dprintf("Missing filename after %s\n", argv[i-1]);
                goto exit;
            }
            fName = argv[i];
        } else
        if(!strcmp(argv[i], "--count")) {
            i++;
            if(i >= argc) {
                dprintf("Missing message count after %s\n", argv[i-1]);
                goto exit;
            }
            sscanf(argv[i], "%d", &n);
        } else
        if(!strcmp(argv[i], "--start")) {
            i++;
            if(i >= argc) {
                dprintf("Missing start message number after %s\n", argv[i-1]);
                goto exit;
            }
            sscanf(argv[i], "%d", &s);
        } else
        if(!strcmp(argv[i], "--last")) {
            i++;
            if(i >= argc) {
                dprintf("Missing message count after %s\n", argv[i-1]);
                goto exit;
            }
            sscanf(argv[i], "%d", &n);
            if((st->WritePosition.QuadPart - st->ReadPosition.QuadPart) < n) {
                n = (ULONG)(st->WritePosition.QuadPart - st->ReadPosition.QuadPart);
                s = 0;
            } else {
                s = (ULONG)(st->WritePosition.QuadPart - st->ReadPosition.QuadPart - n);
            }
        } else
        if(!strcmp(argv[i], "--next")) {
            s = saved_s+saved_n;
            n = saved_n;
        } else

        #include "..\client\fmt_output_opt.h"
        
        {
unrec:
            dprintf("Unrecognized option %s\n", argv[i]);
            retval = -2;
            goto exit;
        }
    }
    if(!dump_mode && !info_mode) {
        dump_mode = TRUE;
    }
    start_item = st->ReadPosition.QuadPart + s;
    if(n) {
        end_item = start_item + n;
    }

    if(start_item < (ULONGLONG)st->ReadPosition.QuadPart) {
        start_item = st->ReadPosition.QuadPart;
    }
    if(end_item >= (ULONGLONG)st->WritePosition.QuadPart) {
        end_item = st->WritePosition.QuadPart;
    }

    saved_s = (ULONG)(end_item - st->ReadPosition.QuadPart);
    saved_n = (ULONG)(end_item - start_item);

    s = (ULONG)start_item & st->BufferSizeMask;
    n = (ULONG)(end_item - start_item);
    if(info_mode && View) {
        dp_dump_st(st);
    }
    if(!MsgBuffer) {
        MsgBuffer = (PDbgPrnHk_GetMessages_USER_OUT)malloc(st->BufferSize);
        if(!MsgBuffer) {
            dprintf("Cannot alloc memory for DbgPrnHk internal message buffer\n");
            goto exit;
        }
    }
    if(!MsgBuffer_PageMap) {
        MsgBuffer_PageMap = (PULONG)malloc(st->BufferSize/PAGE_SIZE/8 + 1);
        if(!MsgBuffer_PageMap) {
            dprintf("Cannot alloc memory for MsgBuffer_PageMap\n");
            goto exit;
        }
        memset(MsgBuffer_PageMap, 0, st->BufferSize/PAGE_SIZE/8 + 1);
    }
    if(all_mode) {
        dprintf("Downloading message buffer (%dMb), please wait.\n", st->BufferSize/(1024*1024));
        dprintf("  or use Ctrl-Break to abort\n");
        for(i=0; (ULONG)i<st->BufferSize; i+=PAGE_SIZE) {
            if(!(i % (1024*1024/16))) {
                dprintf(".");
            }
            if(!(i % (1024*1024))) {
                dprintf("%dMb\n", i / (1024*1024));
            }
            if(!EnvGetBit(MsgBuffer_PageMap, i/PAGE_SIZE)) {
                b = ReadMemory(((ULONG)st->MsgBuffer)+i,
                                          (PUCHAR)(MsgBuffer)+i,
                                          PAGE_SIZE,
                                          NULL);
                if ( !b ) {
cleanup:
                    free(MsgBuffer);
                    MsgBuffer = NULL;
                    free(MsgBuffer_PageMap);
                    MsgBuffer_PageMap = NULL;
                    dprintf("Cannot read DbgPrnHk internal message buffer at offset %#x\n", i);
                    goto exit;
                }
                EnvSetBit(MsgBuffer_PageMap, i/PAGE_SIZE);
            }
            if(CheckControlC()) {
                dprintf("Stopped by Ctrl-C\n");
                goto cleanup;
            }
        }
    }
    if(all_avl_mode) {
        //n = (ULONG)(st->WritePosition.QuadPart - st->ReadPosition.QuadPart);
        n = st->BufferSize / sizeof(DbgPrnHk_GetMessages_USER_OUT);
        if(st->WritePosition.QuadPart < n) {
            n = st->WritePosition.LowPart & st->BufferSizeMask;
            s = 0;
        } else {
            s = (st->WritePosition.LowPart+n+1) & st->BufferSizeMask;
        }
    }
    if(n > st->BufferSizeMask) {
        // fixup DbgPrnHk driver bug :(
        n = st->BufferSizeMask+1;
        s = st->WritePosition.LowPart & st->BufferSizeMask;
    }
    if(dump_mode) {
        dp_dump_msg(st, MsgBuffer, MsgBuffer_PageMap, s, n, fName);
    }

    retval = 1;
exit:
    if(argv) {
        free(argv);
    }
    TRACE(("dp_x_msg: exit %d\n", retval));
    return retval;
} // end dp_x_msg()

int
dp_drvopt(
    PCHAR args
    )
{
    PCHAR* argv = NULL;
    int argc;
    int i;

    PDBGDUMP_OPT_RAW opt = &g_opt;
    ULONG retval = -1;

    PCHAR eq_s;
    PCHAR p_val;
    PCHAR p_nam;
    CHAR saved_a;
    ULONG d;

    TRACE(("dp_drvopt:\n"));

#define pOpt opt

    // Evaluate the argument string to get the address of
    // the string to dump.

    argv = CommandLineToArgvA((PCHAR)args, &argc);
    if(!argv || !argc) {
        if(!dp_load_st()) {
            goto exit;
        }
        dp_dump_st(st);
        //dprintf("Cannot parse commandline\n");
        retval = 1;
        goto exit;
    }

    if(!dp_load_st()) {
        goto exit;
    }

    for(i=0; i<argc; i++) {
        if(!argv[i]) {
            continue;
        }
        if(!strcmp(argv[i], "-h") ||
           !strcmp(argv[i], "-?") ||
           !strcmp(argv[i], "//h") ||
           !strcmp(argv[i], "//?") ||
           !strcmp(argv[i], "--help")) {
            drvopt_help();
            goto exit;
        } else
            eq_s = strchr(argv[i], '=');
            if(eq_s) {
                saved_a = eq_s[0];
                eq_s[0] = 0;
                p_val = eq_s+1;
                p_nam = argv[i];
                d = 0;
                //eq_s[0] = 0;
            } else {
                saved_a = 0;
                p_val = argv[i+1];
                p_nam = argv[i];
                d = 1;
            }
        // recognize driver options and fill structure
        #define DBGPRINT_DRV_OPT_RAW_RECOGNIZE
        #include "dbgprint_opt_list.h"
        /* else */
        {
            dprintf("Unrecognized option %s\n", argv[i]);
            retval = -2;
            goto exit;
        }
    }

    #include "..\driver\update_st.h"
    dp_dump_st(st);

    if(dp_upload_st()) {
        dprintf("Ok\n");
        retval = 1;
    }
#undef pOpt

exit:
    return retval;
} // end dp_drvopt()

int
dp_pause(
    PCHAR args
    )
{
    PCHAR* argv = NULL;
    int argc;
    int i;

    ULONG retval = -1;

    TRACE(("dp_pause:\n"));

    argv = CommandLineToArgvA((PCHAR)args, &argc);
    if(!dp_load_st()) {
        goto exit;
    }

    if(!argv || !argc) {
        st->LoggingPaused = !st->LoggingPaused;
        argc = 0;
    }

    for(i=0; i<argc; i++) {
        if(!argv[i]) {
            continue;
        }
        if(!strcmp(argv[i], "-h") ||
           !strcmp(argv[i], "-?") ||
           !strcmp(argv[i], "//h") ||
           !strcmp(argv[i], "//?") ||
           !strcmp(argv[i], "--help")) {
            pause_help();
            goto exit;
        } else
        if(!strcmp(argv[i], "-") ||
           !strcmp(argv[i], "0") ||
           !strcmp(argv[i], "off")) {
            st->LoggingPaused = 0;
        } else
        if(!strcmp(argv[i], "+") ||
           !strcmp(argv[i], "1") ||
           !strcmp(argv[i], "on")) {
            st->LoggingPaused = 1;
        } else
        {
//unrec:
            dprintf("Unrecognized option %s\n", argv[i]);
            retval = -2;
            goto exit;
        }
    }
    if(dp_upload_st()) {
        if(st->LoggingPaused) {
            dprintf("Logging paused\n");
        } else {
            dprintf("Logging resumed\n");
        }
        retval = 1;
    }
exit:
    return retval;
} // end dp_pause()

int
dp_display(
    PCHAR args
    )
{
    PCHAR* argv = NULL;
    int argc;
    int i;

    ULONG retval = -1;
    ULONG buff_addr = 0xb8000;
    ULONG buff_len = 0x8000;
    BOOLEAN text_mode=TRUE;
    BOOLEAN to_file=FALSE;
    PUCHAR buffer=NULL;
    PCHAR fName = NULL;
    HANDLE h=NULL;
    ULONG returned;
    UCHAR a;
    int x,y;

    TRACE(("dp_display:\n"));

    argv = CommandLineToArgvA((PCHAR)args, &argc);

    for(i=0; i<argc; i++) {
        if(!argv[i]) {
            continue;
        }
        if(!strcmp(argv[i], "-h") ||
           !strcmp(argv[i], "-?") ||
           !strcmp(argv[i], "//h") ||
           !strcmp(argv[i], "//?") ||
           !strcmp(argv[i], "--help")) {
            display_help();
            goto exit;
        } else
        if(!strcmp(argv[i], "-t") ||
           !strcmp(argv[i], "--text")) {
            buff_addr = 0xb8000;
            buff_len = 0x8000;
            text_mode=TRUE;
        } else
        if(!strcmp(argv[i], "-b") ||
           !strcmp(argv[i], "--bin")) {
            buff_addr = 0xb8000;
            buff_len = 0x8000;
            text_mode=FALSE;
        } else
        if(!strcmp(argv[i], "-e") ||
           !strcmp(argv[i], "--ega16")) {
            buff_addr = 0xa0000;
            buff_len = 0x10000;
            text_mode=FALSE;
        } else
        if(!strcmp(argv[i], "-f") ||
           !strcmp(argv[i], "--file")) {
            if((argc-i) < 2) {
                dprintf("Filename required\n");
                goto exit;
            }
            i++;
            fName=argv[i];
        } else
        {
//unrec:
            dprintf("Unrecognized option %s\n", argv[i]);
            retval = -2;
            goto exit;
        }
    }

    buffer = (PUCHAR)malloc(buff_len*2);
    if(!buffer) {
        dprintf("Can't allocate temporary buffer\n");
        goto exit;
    }
    ReadPhysical(buff_addr, buffer, buff_len, &returned);
    if(returned != buff_len) {
        dprintf("Video buffer %#x: Read %#x bytes instead of %#x\n", buff_addr, returned, buff_len);
        goto exit;
    }
    i=0;
    if(text_mode) {
        for(y=0; y<35; y++) {
            for(x=0; x<80; x++) {
                a = buffer[(y*80+x)*2];
                buffer[i]=a;
                i++;
            }
            buffer[i]='\n';
            i++;
        }
        buffer[i]=0;
        i++;
    }

    if(fName) {
        h = dp_open_file_for_save(fName, "Saving video-buffer to %s\n", "Cannot create video dump file\n");
        if(!h) {
            goto exit;
        }
        if(text_mode) {
            if(!WriteFile(h, buffer, i-1, &returned, NULL)) {
                dprintf("Can't save to file\n");
            } else {
                dprintf("Saved\n");
            }
        } else {
            if(!WriteFile(h, buffer, buff_len, &returned, NULL)) {
                dprintf("Can't save to file\n");
            } else {
                dprintf("Saved\n");
            }
        }
        CloseHandle(h);
        h = NULL;
    } else
    if(text_mode) {
        dprintf("\n");
        dprintf((CCHAR*)text_mode);
        dprintf("\n");
    } else {
        dprintf("Can't display video buffer, use --f\n", buff_len, returned);
    }

exit:
    if(buffer) {
        free(buffer);
        buffer = NULL;
    }
    if(h) {
        CloseHandle(h);
        h = NULL;
    }
    return retval;
} // end dp_display()

/*++

Routine Description:

--*/
DECLARE_API(lsig) 

{
    PCHAR* argv;
    int argc;
    DWORD dwAddr = 0;
    BOOL b;
    UCHAR buffer[PAGE_SIZE];
    ULONG i,j;
    ULONG returned;
    PDBGPRNHK_SIGNATURE_PAGE SigPage;
    ULONG MaxMem = 1024*1024/4; // 1Gb (in Pages)
    BOOLEAN ph_addr = FALSE;
    BOOLEAN found = FALSE;
    BOOLEAN scanned = FALSE;

    TRACE(("lsig:\n"));

    argv = CommandLineToArgvA((PCHAR)args, &argc);
    for(i=0; (int)i<argc; i++) {
        if(!argv[i]) {
            continue;
        }
        if(!strcmp(argv[i], "-h") ||
           !strcmp(argv[i], "-?") ||
           !strcmp(argv[i], "--help")) {
            free(argv);
            lsig_help();
            return;
        } else
        if(!strcmp(argv[i], "-max") ||
           !strcmp(argv[i], "--max")) {
            if((argc-i) < 2) {
                dprintf("No MAXMEM value specified\n");
                free(argv);
                return;
            }
            i++;
            sscanf(argv[i], "%d", &MaxMem);
            dprintf("MaxMem = %dMb\n", MaxMem);
            MaxMem *= 1024/4;
        } else
        if(!strcmp(argv[i], "-p") ||
           !strcmp(argv[i], "--phys")) {
            ph_addr = TRUE;
        } else {
            sscanf(argv[i], "%x", &dwAddr);
        }
    }
    free(argv);
    SigPage = (PDBGPRNHK_SIGNATURE_PAGE)&buffer;
    if(dwAddr) {
        if(ph_addr) {
            ReadPhysical(dwAddr, buffer, PAGE_SIZE, &returned);
            b = (returned == PAGE_SIZE);
        } else {
            b = ReadMemory(dwAddr,
                           buffer,
                           PAGE_SIZE,
                           NULL);
        }
        if ( !b ) {
            dprintf("Cannot read signature page from specified %s address (%#x)\n",
                ph_addr ? "phys." : "virt.", dwAddr);
            return;
        }
        for(i=0; i<sizeof(SigPage->Signature); i+=sizeof(DBGPRNHK_SIGNATURE)) {
            if(memcmp(&(SigPage->Signature[i]), DBGPRNHK_SIGNATURE, sizeof(DBGPRNHK_SIGNATURE))) {
                dprintf("Signature not found at specified %s address (%#x)\n", ph_addr ? "physical" : "virtual", dwAddr);
                return;
            }
        }
        dwAddr_st = (ULONG)(SigPage->st);
    } else {
        dprintf("Looking for Signature page 0-%dMb, please wait\n", MaxMem*4/1024);
        scanned = TRUE;
        for(j=0; j<MaxMem; j++) {
            if(!(j % 16)) {
                dprintf(".");
            }
            if(!(j % 126)) {
                dprintf("%dMb\n", j/128);
            }
            if(CheckControlC()) {
                dprintf("\nStopped by Ctrl-C\n");
                return;
            }
            returned = 0;
    /*        for(i=0; i<sizeof(SigPage->Signature); i+=sizeof(DBGPRNHK_SIGNATURE)) {
                if(memset(&(SigPage->Signature[i]), DBGPRNHK_SIGNATURE, sizeof(DBGPRNHK_SIGNATURE))) {
                    break;
                }
            }
    */
            ReadPhysical(j*PAGE_SIZE, buffer, sizeof(DBGPRNHK_SIGNATURE), &returned);
            if(returned != sizeof(DBGPRNHK_SIGNATURE)) {
    /*
                if(j > 1024/4) {
                    dprintf("\nCannot find signature page, scanned %dMb\n", (j*4)/1024);
                    return;
                }
    */
                continue;
            }
            returned = 0;
            ReadPhysical(j*PAGE_SIZE, buffer, PAGE_SIZE, &returned);
            if(returned != PAGE_SIZE) {
    /*
                if(j > 1024/4) {
                    dprintf("\nCannot find signature page, scanned %dMb\n", (j*4)/1024);
                    return;
                }
    */
                continue;
            }
            for(i=0; i<sizeof(SigPage->Signature); i+=sizeof(DBGPRNHK_SIGNATURE)) {
                if(memcmp(&(SigPage->Signature[i]), DBGPRNHK_SIGNATURE, sizeof(DBGPRNHK_SIGNATURE))) {
                    break;
                }
            }
            if(i != sizeof(SigPage->Signature)) {
                continue;
            }
            dwAddr_st = (ULONG)(SigPage->st);
            found = TRUE;
            break;
        }
    }
    if(found) {
        if(scanned) {
            dprintf("\n");
            dwAddr = j*PAGE_SIZE;
        }
        dprintf("Signature page found at %#x (%s)\n", ph_addr ? "physical" : "virtual", dwAddr);
        if(dp_load_st()) {
            if(st->Ver.Major > 0 || st->Ver.Minor >= 8) {
                dprintf("Signature state flags %#x\n", SigPage->state);
                dprintf("Internal state virtual address %#x (physical %#x)\n", dwAddr_st, SigPage->ph_st.LowPart);
            } else {
                dprintf("Internal state virtual address %#x\n", dwAddr_st);
            }
        } else {
            dprintf("Expected Internal state virtual address %#x (load failed)\n", dwAddr_st);
        }
    } else
    if(scanned) {
        dprintf("\nCannot find signature page, scanned %dMb\n", (j*4)/1024);
    }
    return;
} // end DECLARE_API(lsig) 

int
dp_mdump(
    PCHAR args
    )
{
    PCHAR* argv = NULL;
    int argc;
    int i;

    ULONG retval = -1;
    DWORD dwAddr = 0;
    ULONG MaxMem = 1024*1024/4; // 1Mb (in Pages)
    BOOLEAN text_mode=TRUE;
    BOOLEAN to_file=FALSE;
    PUCHAR buffer=NULL;
    PCHAR fName = NULL;
    HANDLE h=NULL;
    ULONG returned;
    ULONG mb = 1;
    ULONG j;

    TRACE(("dp_mdump:\n"));

    argv = CommandLineToArgvA((PCHAR)args, &argc);

    for(i=0; i<argc; i++) {
        if(!argv[i]) {
            continue;
        }
        if(!strcmp(argv[i], "-h") ||
           !strcmp(argv[i], "-?") ||
           !strcmp(argv[i], "//h") ||
           !strcmp(argv[i], "//?") ||
           !strcmp(argv[i], "--help")) {
            mdump_help();
            goto exit;
        } else
        if(!strcmp(argv[i], "-s") ||
           !strcmp(argv[i], "--start") ||
           !strcmp(argv[i], "-b") ||
           !strcmp(argv[i], "--base")) {
            if((argc-i) < 2) {
                dprintf("No BASE PHYSICAL ADDRESS value specified\n");
                goto exit;
            }
            i++;
            sscanf(argv[i], "%d", &dwAddr);
            dprintf("Start from %dMb\n", dwAddr);
            if(mb == 1024/4) {
                dwAddr *= 1024/4;
            } else {
                dwAddr /= PAGE_SIZE;
            }
            goto exit;
        } else
        if(!strcmp(argv[i], "-m") ||
           !strcmp(argv[i], "--mb")) {
            mb = 1024/4;
            goto exit;
        } else
        if(!strcmp(argv[i], "-e") ||
           !strcmp(argv[i], "--end")) {
            if((argc-i) < 2) {
                dprintf("No MAXMEM value specified\n");
                goto exit;
            }
            i++;
            sscanf(argv[i], "%d", &MaxMem);
            dprintf("End address %dMb\n", MaxMem);
            if(mb == 1024/4) {
                MaxMem *= 1024/4;
            } else {
                MaxMem = (MaxMem+PAGE_SIZE-1) / PAGE_SIZE;
            }
            goto exit;
        } else
        if(!strcmp(argv[i], "-f") ||
           !strcmp(argv[i], "--file")) {
            if((argc-i) < 2) {
                dprintf("Filename required\n");
                goto exit;
            }
            i++;
            fName=argv[i];
            if(!fName) {
                dprintf("Filename required\n");
                goto exit;
            }
        } else
        {
//unrec:
            dprintf("Unrecognized option %s\n", argv[i]);
            retval = -2;
            goto exit;
        }
    }

    buffer = (PUCHAR)malloc(PAGE_SIZE);
    if(!buffer) {
        dprintf("Can't allocate temporary buffer\n");
        goto exit;
    }
    if(!fName) {
        dprintf("Filename required\n");
        goto exit;
    }
    if(!MaxMem) {
        if(!st) {
            dp_load_st();
        }
        if(st && st->MaxPage) {
            MaxMem = st->MaxPage;
            dprintf("using MaxMem (%#x) as end address\n", MaxMem*PAGE_SIZE);
        } else {
            dprintf("end address not specified\n");
            goto exit;
        }
    }

    h = dp_open_file_for_save(fName, "Saving video-buffer to %s\n", "Cannot create video dump file\n");
    if(!h) {
        goto exit;
    }
    dprintf("Dumping %#x - %#x\n", dwAddr*PAGE_SIZE, MaxMem*PAGE_SIZE);
    for(j=dwAddr; j<MaxMem; j++) {
        if(!(j % 16)) {
            dprintf(".");
        }
        if(!(j % 126)) {
            dprintf("%dMb\n", j/128);
        }
        if(CheckControlC()) {
            dprintf("\nStopped by Ctrl-C\n");
            goto exit;
        }
        returned = 0;
        ReadPhysical(j*PAGE_SIZE, buffer, PAGE_SIZE, &returned);
        if(returned != PAGE_SIZE) {
            dprintf("Can't read physical memory at %#x\n", j*PAGE_SIZE);
            memset(buffer, 0xff, PAGE_SIZE);
        }
        if(!WriteFile(h, buffer, i-1, &returned, NULL)) {
            dprintf("Can't write to file\n");
            goto exit;
        }
    }
    dprintf("\nSaved to file\n");

exit:
    if(buffer) {
        free(buffer);
        buffer = NULL;
    }
    if(h) {
        CloseHandle(h);
        h = NULL;
    }
    if(argv) {
        free(argv);
    }
    return retval;
} // end dp_mdump()

DECLARE_API(ls) 
{
    TRACE(("ls:\n"));
    dp_x_msg((PCHAR)args, TRUE);
    TRACE(("ls: exit\n"));
} // end DECLARE_API(ls) 

DECLARE_API(list) 
{
    TRACE(("list:\n"));
    dp_x_msg((PCHAR)args, TRUE);
    TRACE(("list: exit\n"));
} // end DECLARE_API(list) 

DECLARE_API(save) 
{
    TRACE(("save:\n"));
    dp_x_msg((PCHAR)args, FALSE);
    TRACE(("save: exit\n"));
} // end DECLARE_API(save)

DECLARE_API(xsave) 
{
    TRACE(("xsave:\n"));
    dp_x_msg((PCHAR)args, FALSE);
    TRACE(("xsave: exit\n"));
} // end DECLARE_API(save)

DECLARE_API(del) 
{
    TRACE(("del:\n"));
    dp_x_msg((PCHAR)args, 2);
    TRACE(("del: exit\n"));
} // end DECLARE_API(save)

DECLARE_API(drvopt) 
{
    TRACE(("drvopt:\n"));
    dp_drvopt((PCHAR)args);
    TRACE(("drvopt: exit\n"));
} // end DECLARE_API(save)

DECLARE_API(pause) 
{
    TRACE(("pause:\n"));
    dp_pause((PCHAR)args);
    TRACE(("pause: exit\n"));
} // end DECLARE_API(pause)

DECLARE_API(display) 
{
    TRACE(("display:\n"));
    dp_display((PCHAR)args);
    TRACE(("display: exit\n"));
} // end DECLARE_API(display)

DECLARE_API(mdump) 
{
    TRACE(("mdump:\n"));
    dp_mdump((PCHAR)args);
    TRACE(("mdump: exit\n"));
} // end DECLARE_API(mdump)

DECLARE_API(help)
{
    dprintf("Available commands:\n");
    dprintf("  ls|list       view debug messages.\n");
    dprintf("  save          write debug messages to file\n");
    dprintf("  del           delete debug messages from buffer\n");
    dprintf("  lsig          locate message buffer by signature\n");
    dprintf("  drvopt        modify or view remote DbgPrnHk options\n");
    dprintf("  pause         pause/resume logging\n");
    dprintf("  display       view or save remote display buffer\n");
    dprintf("  mdump         write physical memory block to file\n");
    dprintf("Use <command> /? to read detailed info about <command>\n");
} // end DECLARE_API(help)
