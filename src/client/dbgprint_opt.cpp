
#include <windows.h>
#include <stdio.h>
#include "DbgPrnHk.h"
#include "DbgPrintLog.h"
#include "dbgprint_opt.h"
//#include "dbgprn_options.h"

#define pOpt     opt

int
DbgPrint_CmdLine_to_OptStruct(
    IN PWCHAR CmdLine,
    IN UCHAR  Reinit,
    OUT PDBGDUMP_OPT opt_struct
    )
{
    int argc;
    WCHAR** argv;
    LONG i=0;
    LONG j;
    WCHAR a;
    PDBGDUMP_OPT_RAW opt = (PDBGDUMP_OPT_RAW)opt_struct;
    PWCHAR eq_s;
    PWCHAR p_val;
    PWCHAR p_nam;
    WCHAR saved_a;
    ULONG d;
    PWCHAR p;

    switch(Reinit) {
    case DbgPrint_OptStruct_Reinit_Drv:
        #define DBGPRINT_DRV_OPT_RAW_INIT
        #include "dbgprint_opt_list.h"
        break;
    case DbgPrint_OptStruct_Reinit_All:
        #define DBGPRINT_OPT_RAW_INIT
        #include "dbgprint_opt_list.h"
        break;
    }

    if(CmdLine) {
        argv = CommandLineToArgvW(CmdLine, &argc);
    } else {
        argv = NULL;
        argc = 0;
    }
    for(i=1; i<argc; i++) {
        if(!argv[i] || argv[i][0] != '-') {
            if(!wcscmp(argv[i], L"/h") ||
               !wcscmp(argv[i], L"/?")) {
                goto show_help;
            }
            opt->LogFile = argv[i];
            continue;
        }
        if(!wcscmp(argv[i], L"-h") ||
           !wcscmp(argv[i], L"-?") ||
//           !wcscmp(argv[i], L"/h") ||
//           !wcscmp(argv[i], L"/?") ||
           !wcscmp(argv[i], L"--help")) {
show_help:
//            usage();
//            exit(0);
            return 0;
        } else
        if(!wcscmp(argv[i], L"--status")) {
//            usage();
//            exit(0);
            opt->display_status = TRUE;
            //return 0;
        } else
        if(!wcscmp(argv[i], L"--run:U") || 
           !wcscmp(argv[i], L"--run:user") ||
           !wcscmp(argv[i], L"--run:usr")) {
//            usage();
//            exit(0);
            opt->user_run = 1;
            //return 0;
        } else
        if(!wcscmp(argv[i], L"--run:S") || 
           !wcscmp(argv[i], L"--run:svc")) {
//            usage();
//            exit(0);
            opt->user_run = 2;
            //return 0;
        } else
        if(!wcscmp(argv[i], L"--nowait") ||
           !wcscmp(argv[i], L"--no_wait") ||
           !wcscmp(argv[i], L"--no-wait") ||
           !wcscmp(argv[i], L"--run:N") ||
           !wcscmp(argv[i], L"--run:nowait")) {
//            usage();
//            exit(0);
            opt->nowait_msg = TRUE;
            //return 0;
        } else
        if(!wcscmp(argv[i], L"-sm") ||
           !wcscmp(argv[i], L"--sync_mode")) {
            opt->sync_mode = TRUE;
        } else
        if(!wcscmp(argv[i], L"-wd") ||
           !wcscmp(argv[i], L"--working_directory")) {
            i++;
            if(i >= argc) {
                print_log("%S requires pathname\n", argv[i-1]);
                return -2;
            }
            if(opt->WDir) {
                GlobalFree(opt->WDir);
            }
            j = wcslen(argv[i])+1;
            opt->WDir = (PWCHAR)GlobalAlloc(GMEM_FIXED, j*sizeof(WCHAR));
            memcpy(opt->WDir, argv[i], j*sizeof(WCHAR));
            opt->WDir[j-1] = 0;
        } else
        if(!wcscmp(argv[i], L"-drvopt") ||
           !wcscmp(argv[i], L"--drvopt") ||
           !wcscmp(argv[i], L"--drv_opt") ||
           !wcscmp(argv[i], L"--opt:drv") ||
           !wcscmp(argv[i], L"--drv:opt")) {
            i++;
            eq_s = wcschr(argv[i], '=');
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

            if(i >= argc) {
                print_log("%S requires parameter\n", argv[i-1]);
                return -2;
            }

            // recognize driver options and fill structure
            #define DBGPRINT_DRV_OPT_RAW_RECOGNIZE
            #include "dbgprint_opt_list.h"
            /* else */
            if(!wcscmp(argv[i], L"*")) {
                // Use all defaults
                #define DBGPRINT_DRV_OPT_RAW_INIT_WITH_APPLY
                // reinit values
                #define DBGPRINT_DRV_OPT_RAW_INIT
                #include "dbgprint_opt_list.h"
            } else {
                print_log("Invalid driver option %S\n", argv[i]);
                return -2;
            }
            
            opt->drv_opt = TRUE;
        } else
        if(!wcscmp(argv[i], L"-drv") ||
           !wcscmp(argv[i], L"--drv:inst") ||
           !wcscmp(argv[i], L"--drv:install")) {
            i++;
            if(i < argc) {
                if(!wcscmp(argv[i], L"1")) {
                    opt->drv_mode = SERVICE_BOOT_START;
                    opt->drv_very_first = TRUE;
                } else
                if(!wcscmp(argv[i], L"B")) {
                    opt->drv_mode = SERVICE_BOOT_START;
                } else
                if(!wcscmp(argv[i], L"S")) {
                    opt->drv_mode = SERVICE_SYSTEM_START;
                } else
                if(!wcscmp(argv[i], L"A")) {
                    opt->drv_mode = SERVICE_AUTO_START;
                } else
                if(!wcscmp(argv[i], L"M")) {
                    opt->drv_mode = SERVICE_DEMAND_START;
                } else
                if(!wcscmp(argv[i], L"U")) {
/*
                    if(sys_account) {
                        continue;
                    }
                    remove_driver();
*/
                    //opt->deinstall_drv = TRUE;
                    opt->drv_mode = SERVICE_DISABLED;
                    //continue;
                } else {
                    if(argv[i][0] != '-') {
                        print_log("Invalid driver startup mode %S\n", argv[i]);
                        return -2;
                    }
                    // revert to initial state
                    i--;
                }
            }
//            if(sys_account) {
//                continue;
//            }
            opt->install_drv = TRUE;
        } else
        if(!wcscmp(argv[i], L"--drv:uninst") ||
           !wcscmp(argv[i], L"--drv:deinst") ||
           !wcscmp(argv[i], L"--drv:uninstall") ||
           !wcscmp(argv[i], L"--drv:deinstall")) {

            opt->drv_mode = SERVICE_DISABLED;
            opt->install_drv = TRUE;
        } else
        if(!wcscmp(argv[i], L"-svc") ||
           !wcscmp(argv[i], L"--svc:inst") ||
           !wcscmp(argv[i], L"--svc:install")) {
            i++;
            if(i < argc) {
                if(!wcscmp(argv[i], L"U")) {
                    opt->svc_mode = SERVICE_DISABLED;
                } else
                if(!wcscmp(argv[i], L"A")) {
                    opt->svc_mode = SERVICE_AUTO_START;
                } else
                if(!wcscmp(argv[i], L"M")) {
                    opt->svc_mode = SERVICE_DEMAND_START;
                } else {
                    // revert to initial state
                    i--;
                }
            }
            opt->install_svc = TRUE;
        } else
        if(!wcscmp(argv[i], L"--svc:uninst") ||
           !wcscmp(argv[i], L"--svc:deinst") ||
           !wcscmp(argv[i], L"--svc:uninstall") ||
           !wcscmp(argv[i], L"--svc:deinstall")) {

            opt->svc_mode = SERVICE_DISABLED;
            opt->install_svc = TRUE;
        } else
/*
        if(!wcscmp(argv[i], L"-run_mode")) {
            i++;
            if(i < argc) {
                if(!wcscmp(argv[i], L"S")) {
                    force_run_mode = TRUE;
                    run_mode_svc = TRUE;
                } else
                if(!wcscmp(argv[i], L"A")) {
                    force_run_mode = TRUE;
                    run_mode_svc = FALSE;
                } else
                {
                    return -2;
                }
            }
        } else
*/
        if(!wcscmp(argv[i], L"-no_drv") ||
           !wcscmp(argv[i], L"--no_drv") ||
           !wcscmp(argv[i], L"--no_driver") ||
           !wcscmp(argv[i], L"--in:nodrv") ||
           !wcscmp(argv[i], L"--in:nodriver") ||
           !wcscmp(argv[i], L"--no_driver")) {
            opt->without_driver = TRUE;
        } else
        if(!wcsicmp(argv[i], L"-s") ||
           !wcsicmp(argv[i], L"--max_log_size")) {
            if(argv[i][1] == 'S') {
prealloc_log_size:
                opt->prealloc_mode = TRUE;
            }
            i++;
            if(i >= argc) {
                print_log("%S requires parameter\n", argv[i-1]);
                return -2;
            }
            swscanf(argv[i], L"%d", &opt->MaxLogSize);
            if(!opt->MaxLogSize || opt->MaxLogSize > 2047) {
                print_log("Invalid %S value: %S\n", argv[i-1], argv[i]);
                return -2;
            }
        } else
        if(!wcscmp(argv[i], L"--prealloc_log_size")) {
            goto prealloc_log_size;
        } else
        if(!wcscmp(argv[i], L"-x") ||
           !wcscmp(argv[i], L"--build_index")) {
            opt->use_index = TRUE;
        } else
        if(!wcscmp(argv[i], L"-l") ||
           !wcscmp(argv[i], L"--max_log_count") ||
           !wcscmp(argv[i], L"--log_rotate_count")) {
            i++;
            if(i >= argc) {
                print_log("%S requires parameter\n", argv[i-1]);
                return -2;
            }
            swscanf(argv[i], L"%d", &opt->MaxLogs);
        } else
        if(!wcscmp(argv[i], L"-n") ||
           !wcscmp(argv[i], L"--initial_log_number")) {
            i++;
            if(i >= argc) {
                print_log("%S requires parameter\n", argv[i-1]);
                return -2;
            }
            swscanf(argv[i], L"%d", &opt->StartLogNum);
        } else
        if(!wcscmp(argv[i], L"-ovw") ||
           !wcscmp(argv[i], L"--overwrite_old_log")) {
            opt->overwrite_old = TRUE;
        } else
        if(!wcscmp(argv[i], L"-ft") ||
           !wcscmp(argv[i], L"--flush_timeout")) {
            i++;
            if(i >= argc) {
                print_log("%S requires parameter\n", argv[i-1]);
                return -2;
            }
            swscanf(argv[i], L"%d", &opt->FlushTimeout);
        } else
        if(!wcscmp(argv[i], L"-cf")/* ||
           !wcscmp(argv[i], L"--out:both")*/) {
            opt->output_file = TRUE;
            opt->output_stdout = TRUE;
            opt->output_none = FALSE;
            opt->output_specified = TRUE;
        } else
        if(!wcscmp(argv[i], L"--out:both")) {
            print_log("Warning: --out:both is deprecated, use --out:file --out:stdout instead\n");
            opt->output_file = TRUE;
            opt->output_stdout = TRUE;
            opt->output_none = FALSE;
            opt->output_specified = TRUE;
        } else
        if(!wcscmp(argv[i], L"--out:file")) {
            opt->output_file = TRUE;
            opt->output_none = FALSE;
            opt->output_specified = TRUE;
        } else
        if(!wcscmp(argv[i], L"--stdout") ||
           !wcscmp(argv[i], L"-o") ||
           !wcscmp(argv[i], L"--out:stdout")) {
            opt->output_stdout = TRUE;
            opt->output_none = FALSE;
            opt->output_specified = TRUE;
        } else
        if(!wcscmp(argv[i], L"--no_out") ||
           !wcscmp(argv[i], L"--out_none") ||
           !wcscmp(argv[i], L"--out:none")) {
            opt->output_file = FALSE;
            opt->output_stdout = FALSE;
            opt->output_syslog = FALSE;
            opt->output_none = TRUE;
            opt->output_specified = TRUE;
        } else
        if(!wcscmp(argv[i], L"--out:syslog")) {
            i++;
            if(i >= argc) {
                print_log("%S requires target hostname\n", argv[i-1]);
                return -1;
            }
            if(opt->output_syslog_HostName) {
                GlobalFree(opt->output_syslog_HostName);
            }
            j = wcslen(argv[i])+1;
            opt->output_syslog_HostName = (PWCHAR)GlobalAlloc(GMEM_FIXED, j*sizeof(WCHAR));
            memcpy(opt->output_syslog_HostName, argv[i], j*sizeof(WCHAR));
            opt->output_syslog_HostName[j-1] = 0;

            p = wcschr(opt->output_syslog_HostName, ':');
            if(p) {
                if(swscanf(p+1, L"%d", &(opt->output_syslog_port)) != 1) {
                    print_log("Error: invalid port specified: %S\n", p);
                    return -1;
                }
                if(!opt->output_syslog_port || opt->output_syslog_port >= 65535) {
                    print_log("Error: invalid port specified: %S, must be between 1 and 65534\n", p);
                    return -1;
                }
                (*p) = 0;
            }

            opt->output_syslog = TRUE;
            opt->output_none = FALSE;
            opt->output_specified = TRUE;
        } else
        if(!wcscmp(argv[i], L"--in:syslog")) {
            i++;
            if(i >= argc) {
                continue;
            }
            if(opt->input_syslog_HostName) {
                GlobalFree(opt->input_syslog_HostName);
            }
            j = wcslen(argv[i])+1;
            opt->input_syslog_HostName = (PWCHAR)GlobalAlloc(GMEM_FIXED, j*sizeof(WCHAR));
            memcpy(opt->input_syslog_HostName, argv[i], j*sizeof(WCHAR));
            opt->input_syslog_HostName[j-1] = 0;

            p = wcschr(opt->input_syslog_HostName, ':');
            if(p) {
                if(swscanf(p+1, L"%d", &(opt->input_syslog_port)) != 1) {
                    print_log("Error: invalid port specified: %S\n", p);
                    return -1;
                }
                (*p) = 0;
            } else {
                p = &opt->input_syslog_HostName[0];
                // try to distinguiesh between port and host specification
                if(swscanf(p, L"%d", &(opt->input_syslog_port)) == 1) {
                    // ok, it is PORT
                    (*p) = 0;
                } else {
                   // also ok, it is bind address
                }
            }

            if(!opt->input_syslog_port || opt->input_syslog_port >= 65535) {
                print_log("Error: invalid port specified: %S, must be between 1 and 65534\n", p);
                return -1;
            }

            opt->input_syslog = TRUE;
            opt->input_driver = FALSE;
            opt->input_file   = FALSE;
            opt->input_stdin  = FALSE;
            opt->input_comdbg = FALSE;
            opt->without_driver = TRUE;
            opt->log_time_perf = TRUE;
            //opt->output_none = FALSE;
            //opt->output_specified = TRUE;
        } else
        if(!wcscmp(argv[i], L"--syslog:mtu") /*||
           !wcscmp(argv[i], L"--in_syslog:mtu") ||
           !wcscmp(argv[i], L"--out_syslog:mtu")*/) {
            i++;
            if(i >= argc) {
                print_log("%S requires parameter\n", argv[i-1]);
                return -2;
            }
            if(swscanf(argv[i], L"%d", &opt->output_syslog_mtu) != 1) {
                print_log("Error: invalid port specified: %S\n", p);
                return -1;
            }
            if(opt->output_syslog_mtu < 64 || opt->output_syslog_mtu > 16384) {
                print_log("Error: invalid MTU specified: %S, must be between 64 and 16384\n", argv[i]);
                return -1;
            }
            opt->input_syslog_mtu = opt->output_syslog_mtu;
        } else
        if(!wcscmp(argv[i], L"--in_drv") ||
           !wcscmp(argv[i], L"--in:drv")) {
            opt->input_driver = TRUE;
            opt->input_file   = FALSE;
            opt->input_stdin  = FALSE;
            opt->input_comdbg = FALSE;
            opt->input_syslog = FALSE;
        } else
        if(!wcscmp(argv[i], L"--stdin") ||
           !wcscmp(argv[i], L"--in:stdin")) {
            opt->input_driver = FALSE;
            opt->input_file   = FALSE;
            opt->input_stdin  = TRUE;
            opt->input_comdbg = FALSE;
            opt->without_driver = FALSE;
            opt->log_time_perf = TRUE;
            opt->input_syslog = FALSE;
        } else
        if(!wcscmp(argv[i], L"-in_file") ||
           !wcscmp(argv[i], L"--in_file") ||
           !wcscmp(argv[i], L"--in:file")) {
            i++;
            if(i >= argc) {
                print_log("%S requires filename\n", argv[i-1]);
                return -1;
            }
            if(opt->input_FileName) {
                GlobalFree(opt->input_FileName);
            }
            j = wcslen(argv[i])+1;
            opt->input_FileName = (PWCHAR)GlobalAlloc(GMEM_FIXED, j*sizeof(WCHAR));
            memcpy(opt->input_FileName, argv[i], j*sizeof(WCHAR));
            opt->input_FileName[j-1] = 0;

            opt->input_driver = FALSE;
            opt->input_file   = TRUE;
            opt->input_stdin  = FALSE;
            opt->input_comdbg = FALSE;
            opt->without_driver = FALSE;
            opt->log_time_perf = TRUE;
            opt->input_syslog = FALSE;
        } else
        if(!wcscmp(argv[i], L"-in_comdbg") ||
           !wcscmp(argv[i], L"--in_comdbg") ||
           !wcscmp(argv[i], L"--in:comdbg")) {

            opt->input_driver = FALSE;
            opt->input_file   = FALSE;
            opt->input_stdin  = FALSE;
            opt->input_comdbg = TRUE;
            opt->without_driver = TRUE;
            opt->log_time_perf = TRUE;
            opt->input_syslog = FALSE;
        } else
        if(!wcscmp(argv[i], L"--comdbg:port") ||
           !wcscmp(argv[i], L"--comdbg_port")) {
            i++;
            if(i >= argc) {
                print_log("%S requires parameter\n", argv[i-1]);
                return -2;
            }
            j=0;
            p = argv[i];
            while(p[j] && (p[j] < '0' || p[j] > '9')) {
                j++;
            }
            if(swscanf(p+j, L"%d", &opt->input_comdbg_port) != 1) {
                print_log("Error: invalid COM port specified: %S\n", p);
                return -1;
            }
            if(!opt->input_comdbg_port || opt->output_syslog_port > 1024) {
                print_log("Error: invalid COM port number specified: %S\n", p);
                return -1;
            }
        } else
        if(!wcscmp(argv[i], L"--comdbg:baud") ||
           !wcscmp(argv[i], L"--comdbg:rate") ||
           !wcscmp(argv[i], L"--comdbg_baud") ||
           !wcscmp(argv[i], L"--comdbg_rate")) {
            i++;
            if(i >= argc) {
                print_log("%S requires parameter\n", argv[i-1]);
                return -2;
            }
            if(swscanf(argv[i], L"%d", &opt->input_comdbg_baud) != 1) {
                print_log("Error: invalid COM port baud rate specified: %S\n", argv[i]);
                return -1;
            }
            if(!opt->input_comdbg_baud) {
                print_log("Error: invalid COM port baud rate specified: %S\n", argv[i]);
                return -1;
            }
        } else
        if(!wcscmp(argv[i], L"--no_header")/* ||
           !wcscmp(argv[i], L"--header:off")*/) {
            opt->skip_header = TRUE;
        } else

//        #define ARG_OPT_UNICODE
        #include "fmt_output_opt.h"

        if(!wcscmp(argv[i], L"-rd") ||
           !wcscmp(argv[i], L"--drv:reload")) {
            opt->restart_drv = TRUE;
        } else
        if(!wcscmp(argv[i], L"-rc") ||
           !wcscmp(argv[i], L"--drv:reconfig")) {
            opt->change_config = TRUE;

        } else {
            //print_log("Unrecognized option: %S\n", argv[i]);
            return 0;
        }
    }
    if(!opt->output_specified) {
        opt->output_file = TRUE;
    }
    return 1;
} // end DbgPrint_CmdLine_to_OptStruct()

VOID
DbgPrint_Cleanup_OptStruct(
    OUT PDBGDUMP_OPT opt_struct
    )
{
    PDBGDUMP_OPT_RAW opt = (PDBGDUMP_OPT_RAW)opt_struct;

} // end DbgPrint_Cleanup_OptStruct()