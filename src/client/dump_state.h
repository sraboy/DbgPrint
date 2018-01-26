void
dp_dump_st(
    PDBGPRNHK_INTERNAL_STATE st
    )
{
    print_log("Ver: %d.%d(%d)\n", st->Ver.Major, st->Ver.Minor, st->Ver.Sub);

    print_log("MsgBuffer              = %#x\n", st->MsgBuffer);
    print_log("ReadPosition           = %#I64x (%I64u)\n", st->ReadPosition.QuadPart,  st->ReadPosition.QuadPart);
    print_log("WritePosition          = %#I64x (%I64u)\n", st->WritePosition.QuadPart, st->WritePosition.QuadPart);
    print_log("BufferSize             = %#x bytes (%d Kb)\n", st->BufferSize, st->BufferSize / (1024));
    print_log("BufferSizeMask         = %#x (for item indexes)\n", st->BufferSizeMask);
    print_log("QueueSize              = %#x\n", st->QueueSize);
    print_log("MaxQueueSize           = %#x\n", st->MaxQueueSize);
    print_log("CheckIrql              = %s\n", st->CheckIrql ? "Yes" : "No");
    print_log("DoNotPassMessagesDown  = ");
    switch(st->DoNotPassMessagesDown) {
    case DoNotPassMessages_Off:
        print_log("Off (0)\n");
        break;
    case DoNotPassMessages_On:
        print_log("On (1)\n");
        break;
    case DoNotPassMessages_NoBuffer:
        print_log("Ignore (2)\n");
        break;
    default:
        print_log("unknown (%x)\n", st->DoNotPassMessagesDown);
        break;
    }
    print_log("StopOnBufferOverflow   = ");
    switch(st->StopOnBufferOverflow) {
    case BufferOverflow_Continue:
        print_log("Continue\n");
        break;
    case BufferOverflow_Stop:
        print_log("Stop\n");
        break;
    case BufferOverflow_CallDebugger:
        print_log("CallDebugger\n");
        break;
    default:
        print_log("unknown (%x)\n", st->StopOnBufferOverflow);
        break;
    }
    print_log("LoggingPaused          = %s\n", st->LoggingPaused ? "Yes" : "No");
    print_log("TimeStampType          = %d, ", st->TimeStampType);
    switch(st->TimeStampType) {
    case TimeStampType_SysPerfCounter:
        print_log("KeQueryPerformanceCounter()\n");
        break;
    case TimeStampType_RdtscPerfCounter:
        print_log("Rdtsc CPU instruction\n");
        break;
    case TimeStampType_SysTime:
        print_log("KeQuerySystemTime()\n");
        break;
    default:
        print_log("unknown\n");
        break;
    }
    print_log("BugCheckRegistered     = %s\n", st->BugCheckRegistered ? "Yes" : "No");
    print_log("KdDebuggerEnabled      = %s\n", st->KdDebuggerEnabled ? "Yes" : "No");
    print_log("KdDebuggerNotPresent   = %s\n", st->KdDebuggerNotPresent ? "Yes" : "No");
} // end dp_dump_st()

