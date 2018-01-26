
#include "arg_opt_unicode.h"

    if(!arg_strcmp(argv[i], arg_TEXT("-full")) ||
       !arg_strcmp(argv[i], arg_TEXT("--full"))) {
        opt->log_irql = TRUE;
        opt->log_pid = TRUE;
        opt->log_tid = TRUE;
        opt->log_mode = TRUE;
    } else
    if(!arg_strcmp(argv[i], arg_TEXT("-fm")) ||
       !arg_strcmp(argv[i], arg_TEXT("--filter:block")) ||
       !arg_strcmp(argv[i], arg_TEXT("--filter:deny")) ||
       !arg_strcmp(argv[i], arg_TEXT("--filter:skip")) ||
       !arg_strcmp(argv[i], arg_TEXT("--filter:drop")) ||
       FALSE) {
        i++;
        if(i >= argc) {
            print_log("%s requires parameter\n", argv[i-1]);
            return -2;
        }
        if(!arg_stricmp(argv[i], arg_TEXT("K")) ||
           !arg_stricmp(argv[i], arg_TEXT("origin=kernel")) ||
           !arg_stricmp(argv[i], arg_TEXT("origin=ring0"))) {
            opt->skip_kernel = TRUE;
        } else
        if(!arg_stricmp(argv[i], arg_TEXT("U")) ||
           !arg_stricmp(argv[i], arg_TEXT("origin=user")) ||
           !arg_stricmp(argv[i], arg_TEXT("origin=ring3"))) {
            opt->skip_umode = TRUE;
        } else {
            print_log("Invalid %s mode: %s\n", argv[i-1], argv[i]);
            return -2;
        }
    } else
    if(!arg_strcmp(argv[i], arg_TEXT("-T")) ||
       !arg_strcmp(argv[i], arg_TEXT("--timestamp_fields"))) {
        i++;
        if(i >= argc) {
            print_log("%s requires parameter\n", argv[i-1]);
            return -2;
        }
        opt->log_time_ext = FALSE;
        opt->log_time_perf = FALSE;
        for(j=0; a = argv[i][j]; j++) {
            switch(a) {
            case 'D':
            case 'd':
                opt->log_time_date = TRUE;
                opt->log_time_ext = TRUE;
                break;
            case 'T':
            case 't':
                opt->log_time_time = TRUE;
                opt->log_time_ext = TRUE;
                break;
            case 'N':
            case 'n':
                opt->log_time_nano = TRUE;
                opt->log_time_ext = TRUE;
                break;
            case 'R':
            case 'r':
                opt->log_time_perf = TRUE;
                break;
            case 'U':
            case 'u':
                opt->log_time_utc = TRUE;
                break;
            default:
                print_log("Invalid %s mode: %s\n", argv[i-1], argv[i]);
                return -2;
            }
        }
    } else
    if(!arg_strcmp(argv[i], arg_TEXT("-i")) ||
       !arg_strcmp(argv[i], arg_TEXT("--irql"))) {
        opt->log_irql = TRUE;
    } else
    if(!arg_strcmp(argv[i], arg_TEXT("-p")) ||
       !arg_strcmp(argv[i], arg_TEXT("--pid")) ||
       !arg_strcmp(argv[i], arg_TEXT("--process_id"))) {
        opt->log_pid = TRUE;
    } else
    if(!arg_strcmp(argv[i], arg_TEXT("-P")) ||
       !arg_strcmp(argv[i], arg_TEXT("--pname")) ||
       !arg_strcmp(argv[i], arg_TEXT("--process_name"))) {
        opt->log_pname = TRUE;
    } else
    if(!arg_strcmp(argv[i], arg_TEXT("-t")) ||
       !arg_strcmp(argv[i], arg_TEXT("--tid")) ||
       !arg_strcmp(argv[i], arg_TEXT("--thread_id"))) {
        opt->log_tid = TRUE;
    } else
    if(!arg_strcmp(argv[i], arg_TEXT("-m")) ||
       !arg_strcmp(argv[i], arg_TEXT("--caller_mode"))) {
        opt->log_mode = TRUE;
    } else
    if(!arg_strcmp(argv[i], arg_TEXT("-cpu")) ||
       !arg_strcmp(argv[i], arg_TEXT("--cpu"))) {
        opt->log_cpu = TRUE;
    } else
    if(!arg_strcmp(argv[i], arg_TEXT("-sfp")) ||
       !arg_strcmp(argv[i], arg_TEXT("--stack_frame_ptr"))) {
        opt->log_sfp = TRUE;
    } else
    if(!arg_strcmp(argv[i], arg_TEXT("--raw"))) {
        opt->raw_mode = TRUE;
        opt->skip_header = TRUE;
    } else

