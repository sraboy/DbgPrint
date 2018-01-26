#include <windows.h>
#include <stdio.h>

void usage() {
    printf(
           "Echo to DebugConsole v1.0 (c) by Alexander A. Telyatnikov (Alter), 2004\n"
           "  Home site http://www.alter.org.ua\n"
           "Usage:\n"
           "  EchoDbg [-<switches>] <mesage to print>\n"
           "Switches:\n"
           "  -i                  copy StdIn to DbgConsole\n"
           "  -o                  write message to both StdOut and DbgConsole\n"
           "  -?,-h               display this help\n"
           "Examples:\n"
           "  EchoDbg \"Start test 1\"\n"
           "  EchoDbg -o \"Start test 1\"\n"
           "  cat Message.txt | EchoDbg -i\n"
          );
    exit(0);
}

HANDLE StdIn  = NULL;

BOOLEAN use_stdout = 0;
BOOLEAN use_stdin = 0;

void main (void) {
    int i;

    WCHAR*  CmdLine;
    PWCHAR* argv;
    int argc;
    DWORD InputMode;
    ULONG read_bytes;
    BOOLEAN was_r;

    CHAR LogBuffer[4096];

    if(GetVersion() & 0x80000000) {
        printf("For Windows NT family only.\n");
        exit(-2);
    }

    StdIn  = GetStdHandle(STD_INPUT_HANDLE);

    CmdLine = GetCommandLineW();
    argv = CommandLineToArgvW(CmdLine, &argc);

    if(argc < 2)
        usage();
    for(i=1; i<argc; i++) {
        if(!argv[i])
            continue;
        if(argv[i][0] != '-') {
            continue;
        }
        if(!wcscmp(argv[i], L"-h") ||
           !wcscmp(argv[i], L"-?") ||
           !wcscmp(argv[i], L"--help")) {
            usage();
            exit(0);
        } else
        if(!wcscmp(argv[i], L"-o")) {
            use_stdout = TRUE;
        } else
        if(!wcscmp(argv[i], L"-i")) {
            use_stdin = TRUE;
        } else
        {}
    }
    if(!use_stdin) {
        for(i=1; i<argc; i++) {
            if(!argv[i])
                continue;
            if(argv[i][0] == '-') {
                continue;
            }
            LogBuffer[0] = 0;
            _snprintf(LogBuffer, 4096, "%S\n", argv[i]);
            LogBuffer[4095] = 0;
            OutputDebugString(LogBuffer);
            if(use_stdout) {
                printf(LogBuffer);
            }
        }
    } else {
        if(GetConsoleMode(StdIn, &InputMode)) {
            if(!SetConsoleMode(StdIn, InputMode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT))) {
                printf("Can't set console input mode\n");
                exit(-3);
            }
        }
        i = 0;
        LogBuffer[0] = 0;
        read_bytes = 0;
        was_r = FALSE;
        while(ReadFile(StdIn, &LogBuffer[i], 1, &read_bytes, NULL)) {
            if(read_bytes) {
                LogBuffer[i+1] = 0;
                //printf("%x|%s\n", LogBuffer[i], LogBuffer);
                if(LogBuffer[i] == '\r') {
                    //printf("was r\n");
                    was_r = TRUE;
                } else
                if((LogBuffer[i] == '\n') && was_r) {
                    //printf("was r + n\n");
                    // do nothing
                } else {
                    was_r = FALSE;
                }
                if((LogBuffer[i] == '\n') && was_r) {
                    //printf("was r + n\n");
                    // do nothing
                } else
                if((LogBuffer[i] == '\n') || (LogBuffer[i] == '\r') || !LogBuffer[i] || (i >= 4094)) {
                    if((LogBuffer[i] == '\r') && i && (LogBuffer[i-1] != '\n')) {
                        LogBuffer[i] = '\n';
                    }
                    LogBuffer[i+1] = 0;
                    //printf("1:%x|%s\n", LogBuffer[i], LogBuffer);
                    OutputDebugString(LogBuffer);
                    if(use_stdout) {
                        printf("\n");
                    }
                    LogBuffer[0] = 0;
                    i = 0;
                } else {
                    if(use_stdout) {
                        printf(LogBuffer+i);
                    }
                    i++;
                }
                read_bytes = 0;
            }
        }
        LogBuffer[i] = 0;
        //printf("2:%x|%s\n", LogBuffer[i], LogBuffer);
        OutputDebugString(LogBuffer);
        if(use_stdout) {
            printf("\n");
        }
    }
    exit(0);
}
