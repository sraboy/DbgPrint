#include "stdafx.h"

static const PCHAR char2txt[] = {
    "00",    "01",    "02",    "03",    "04",    "05",    "06",    "07",    "08",    "09",
    "10",    "11",    "12",    "13",    "14",    "15",    "16",    "17",    "18",    "19",
    "20",    "21",    "22",    "23",    "24",    "25",    "26",    "27",    "28",    "29",
    "30",    "31",    "32",    "33",    "34",    "35",    "36",    "37",    "38",    "39"
};

static const PCHAR num2mon[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Nov", "Oct", "Dec" };

ULONG
dbgprint_format_msg(
    PDBGDUMP_OPT_RAW opt,
    PDbgPrnHk_GetMessages_USER_OUT CurMsg,
    PCHAR buf, // must be >= 4k
    PDBGPRN_TIMESTAMP_CTX      timest,
    PDBGPRN_FORMAT_MESSAGE_CTX msgst
    )
{
    SYSTEMTIME SysTime;
    ULONGLONG LocFileTime;
    ULONGLONG rTimeStamp;
    ULONGLONG dTime;
    ULONG len=0;
    ULONG l;
    ULONG d;
    BOOLEAN append_crlf = FALSE;
    PWCHAR pname = NULL;
    BOOLEAN free_pname = FALSE;

    // write message to file (buffered)
    msgst->syslog_hrd_len=0;
    if(!CurMsg->Length) {
        return 0;
    }
    if(opt->log_pname) {
        if(CurMsg->ProcessId) {
            pname = FindProcessNameW((ULONG)(CurMsg->ProcessId));
            if(pname) {
                free_pname = TRUE;
            }
        } else {
            pname = L"System";
        }
    }
    if(opt->output_syslog || !opt->log_time_perf) {
        switch(CurMsg->TimeStampType) {
        case TimeStampType_SysPerfCounter:
            rTimeStamp = CurMsg->TimeStamp.QuadPart - timest->RelPerfCounter;
            if(opt->log_time_ext) {
                dTime = (rTimeStamp/timest->Frequency)*10000000 + (((rTimeStamp%timest->Frequency) * 10000000) / timest->Frequency);
            }
            break;
        case TimeStampType_RdtscPerfCounter:
            rTimeStamp = (LONGLONG)((double)(CurMsg->TimeStamp.QuadPart - timest->CalibrationBuffer.RdtscTimeStampCalibration0)
                * timest->RdtscCalibration);
            rTimeStamp += timest->CalibrationBuffer.SysTimeStampCalibration0 - timest->RelPerfCounter;
            if(opt->log_time_ext) {
                dTime = (rTimeStamp/timest->Frequency)*10000000 + (((rTimeStamp%timest->Frequency) * 10000000) / timest->Frequency);
            }
            break;
        case TimeStampType_SysTime:
            dTime = CurMsg->TimeStamp.QuadPart;
            if(opt->log_time_perf) {
                rTimeStamp = ((dTime-timest->UtcSysTime)/10000000)*timest->Frequency +
                    ((((dTime-timest->UtcSysTime)%10000000) * timest->Frequency) / 10000000);
            }
            break;
        }
        // dTime - nano-seconds;
        //r = (ULONG)(dTime % 100);
        //dTime /= 100;
        // dTime - nano-seconds*100;

        if(opt->output_syslog || opt->log_time_ext) {
            if(opt->log_time_utc) {
                //*((PLONGLONG)&LocFileTime) = CurMsg->TimeStamp.QuadPart;
                LocFileTime = dTime + timest->UtcSysTime;
            } else {
                LocFileTime = dTime + timest->FtmSysTime;
                //FileTimeToLocalFileTime((PFILETIME)&dTime, (PFILETIME)&LocFileTime);
                //FileTimeToSystemTime((PFILETIME)&LocFileTime, &SysTime);
            }
            FileTimeToSystemTime((PFILETIME)&LocFileTime, &SysTime);
        }
    }
    if(opt->output_syslog) {
        msgst->new_line = TRUE;
        len += sprintf(buf+len, "<%d7>", CurMsg->CallerMode);
        memcpy(buf+len, num2mon[SysTime.wMonth-1], 3);
        len += 3;
        len += sprintf(buf+len, " %s%d", (SysTime.wDay < 10) ? " " : "", SysTime.wDay);
        len += sprintf(buf+len, " %2.2d:%2.2d:%2.2d ",
            SysTime.wHour,
            SysTime.wMinute,
            SysTime.wSecond
            );

        l = 1023;
        GetComputerName(buf+len, &l);
        len += l;
        if(opt->log_pname && pname) {
            len += sprintf(buf+len, " %S:", pname);
        } else {
            len += sprintf(buf+len, " PID-%d:", CurMsg->ProcessId);
        }
        msgst->syslog_hrd_len = len;
    }
    if(msgst->new_line) {
//        LineNumber++;
//        WriteLineNumber(ih, CurLogSize);
        if(opt->log_mode) {
            memcpy(buf+len, CurMsg->CallerMode ? "U " : "K ", 2);
            len+=2;
        }
        if(opt->log_cpu) {
            if(CurMsg->CpuNumber == 0xff) {
                memcpy(buf+len, "CPU-?? ", sizeof("CPU-?? ")-1);
                len += sizeof("CPU-?? ")-1;
            } else {
                len += sprintf(buf+len, "CPU-%2.2x ",
                    CurMsg->CpuNumber);
            }
        }
        if(opt->log_irql) {
            len += sprintf(buf+len, "%2.2x ",
                CurMsg->Irql);
        }
        if(opt->log_pid) {
            if(opt->log_pname && pname) {
                len += sprintf(buf+len, "%S: ", pname);
            } else {
                len += sprintf(buf+len, "???: ");
            }
            len += sprintf(buf+len, "%8.8x ",
                CurMsg->ProcessId);
        }
        if(opt->log_tid) {
            if(CurMsg->ThreadId != (PVOID)(-1)) {
                len += sprintf(buf+len, "%8.8x ",
                    CurMsg->ThreadId);
            } else {
                memcpy(buf+len, "???????? ", sizeof("???????? ")-1);
                len += sizeof("???????? ")-1;
            }
        }
        if(opt->log_sfp) {
            if(CurMsg->WithSF) {
                len += sprintf(buf+len, "@%8.8x ",
                    CurMsg->StackFramePtr);
            }
        }
        if(opt->log_time_perf) {
            len += sprintf(buf+len, "%I64d.%4.4d\t",
                CurMsg->TimeStamp.QuadPart/10000,
                (ULONG)(CurMsg->TimeStamp.QuadPart%10000));
        } else {
            
            if(opt->log_time_ext) {

                buf[len] = '[';
                len++;
                if(opt->log_time_date) {
                    len += sprintf(buf+len, "%d/%2.2d/%2.2d",
                        SysTime.wYear,
                        SysTime.wMonth,
                        SysTime.wDay
                        );
                }

                if(opt->log_time_nano) {
                    len += sprintf(buf+len, "%2.2d:%2.2d:%2.2d.%3.3d%4.4d",
                        SysTime.wHour,
                        SysTime.wMinute,
                        SysTime.wSecond,
                        SysTime.wMilliseconds,
                        dTime % 10000
                        );
                } else
                if(opt->log_time_time) {
                    len += sprintf(buf+len, "%2.2d:%2.2d:%2.2d.%3.3d%1.1d",
                        SysTime.wHour,
                        SysTime.wMinute,
                        SysTime.wSecond,
                        SysTime.wMilliseconds,
                        (dTime % 10000) / 1000
                        );
                }
                memcpy(buf+len, "] ", 2);
                len+=2;
            }
            if(opt->log_time_perf) {
                //rTimeStamp = CurMsg->TimeStamp.QuadPart;
                rTimeStamp += timest->RelPerfCounter;
                len += sprintf(buf+len, " (%I64d.%4.4d ticks)",
                    rTimeStamp/10000,
                    (ULONG)(rTimeStamp%10000)
                    );
            }
        }
        msgst->new_line = FALSE;
    }
    if(CurMsg->WithSF) {
        if(CurMsg->Length >= sizeof(ULONG)) {
            l = min(CurMsg->Length-sizeof(ULONG), sizeof(CurMsg->Msg));
        } else {
            l = 0;
        }
        d = sizeof(ULONG);
    } else {
        l = min(CurMsg->Length, sizeof(CurMsg->Msg));
        d = 0;
    }
    if(l && CurMsg->Msg[d+l-1] == '\n') {
        msgst->new_line = TRUE;
    } else
    if(l > 1 && CurMsg->Msg[d+l-1] == '\r' && CurMsg->Msg[d+l-2] == '\n') {
        msgst->new_line = TRUE;
    } else
    if(l > 1 && CurMsg->Msg[d+l-1] == '\n' && CurMsg->Msg[d+l-2] == '\r') {
        msgst->new_line = TRUE;
    } else
    if(l && CurMsg->Msg[d+l-1] == '\r') {
        msgst->new_line = TRUE;
    } else
    if(CurMsg->ThreadId != msgst->prev_ThreadId ||
       CurMsg->ProcessId != msgst->prev_ProcessId) {
        msgst->new_line = TRUE;
	append_crlf = TRUE;
    }
    msgst->prev_ThreadId  = CurMsg->ThreadId;
    msgst->prev_ProcessId = CurMsg->ProcessId;
    // copy message to buffer
    if(CurMsg->DataType == DbgDataType_Text) {
        memcpy(buf+len, CurMsg->Msg+d, l);
    } else
    if(CurMsg->DataType == DbgDataType_Bin) {
        // decode binary message
        ULONG i;
        for(i=0; i<l; i++) {
            ULONG c;
            c = (ULONG)(*(((PUCHAR)(CurMsg->Msg+d))+i));
            len += sprintf(buf+len, "%2.2x ", c);
            if ((i & 0x0f) == 0x0f) {
                buf[len] = '\n';
                len++;
            }
        }
        append_crlf = TRUE;
    } else
    if(CurMsg->DataType == DbgDataType_Dup) {
        // duplicated messages
        if(CurMsg->MsgDup.DupCounter) {
            len += sprintf(buf+len, "Last message repeated %u times\n", CurMsg->MsgDup.DupCounter+1);
        } else {
            len += sprintf(buf+len, "Last message repeated once again\n");
        }
    } else {
    }
    len+=l;
    if(append_crlf && !opt->output_syslog) {
        buf[len] = '\n';
	len++;
    }
    if(free_pname) {
        GlobalFree(pname);
    }

    return len;
} // end dbgprint_formet_msg()


void
dbgprint_format_msg_init(
    PDBGDUMP_OPT_RAW opt,
    PDBGPRN_FORMAT_MESSAGE_CTX msgst
    )
{
    msgst->prev_ThreadId  = (HANDLE)0xfffffffe;
    msgst->prev_ProcessId = (HANDLE)0xfffffffe;
    msgst->new_line = TRUE;

} // end dbgprint_format_msg_init()

void
synchronize_counters(
    LONGLONG* UtcSysTime,
    LONGLONG* RelPerfCounter
    )
{
    LONGLONG ft0 = 0;

    // Spin waiting for a change in system time. Get the matching
    // performance counter value for that time.
    //
    GetSystemTimeAsFileTime((PFILETIME)&ft0);
    do {
        GetSystemTimeAsFileTime((PFILETIME)UtcSysTime);
        QueryPerformanceCounter((PLARGE_INTEGER)RelPerfCounter);
    }
    while (*UtcSysTime == ft0);

} // end synchronize_counters()

void
dbgprint_timestamp_resync(
    PDBGDUMP_OPT_RAW           opt,
    HANDLE                     h_drv,
    PDBGPRN_TIMESTAMP_CTX      timest
    )
{
    DbgPrnHk_GetRdtscCalibration_USER_IN  ReCalibrationBuffer;
    ULONG returned;

    if(opt->input_driver) {
        // Read calibration info
        ReCalibrationBuffer.Recalibrate = TRUE;
        ReCalibrationBuffer.ChangeType = FALSE;
        if(!DeviceIoControl(h_drv,IOCTL_DbgPrnHk_GetRdtscCalibration,
                            &ReCalibrationBuffer, sizeof(ReCalibrationBuffer),
                            &timest->CalibrationBuffer, sizeof(timest->CalibrationBuffer),
                            &returned,NULL)) {
            returned = GetLastError();
            returned = 0;
            timest->RdtscCalibration = 1.0;
        } else {
            timest->RdtscCalibration =
                (double)(timest->CalibrationBuffer.SysTimeStampCalibration - timest->CalibrationBuffer.SysTimeStampCalibration0) /
                (double)(timest->CalibrationBuffer.RdtscTimeStampCalibration - timest->CalibrationBuffer.RdtscTimeStampCalibration0);
        }
    }
}

void
dbgprint_timestamp_init(
    PDBGDUMP_OPT_RAW           opt,
    HANDLE                     h_drv,
    PDBGPRN_TIMESTAMP_CTX      timest
    )
{
    QueryPerformanceFrequency((PLARGE_INTEGER)&timest->Frequency);

    synchronize_counters((PLONGLONG)&timest->UtcSysTime, (PLONGLONG)&timest->RelPerfCounter);

    if(opt->log_time_utc) {
        timest->FtmSysTime = timest->UtcSysTime;
    } else {
        //GetLocalTime(&SysTime);
        //SystemTimeToFileTime(&SysTime, (PFILETIME)&FtmSysTime);
        FileTimeToLocalFileTime((PFILETIME)&timest->UtcSysTime, (PFILETIME)&timest->FtmSysTime);
    }

    dbgprint_timestamp_resync(opt, h_drv, timest);

} // end dbgprint_format_msg_init()
