/*

Copyleft (L) 2004-2016  AlterWare Beast Inc.   >;->

Module Name:        DbgPrnHk.cpp
Abstract:           
Authors:            Alter
Environment:        kernel mode only

 */

#include "DbgPrnHk.h"
//#include "bin_patch.h"
#include "regtools.h"

#include "..\sdk\CrossNt\CrossNt.h"
#include "binhook.h"

PDRIVER_OBJECT DriverObject;
PDEVICE_OBJECT DbgPrnHkDevice = NULL;
PULONG         p_Kd_WIN2000_Mask = NULL;

UNICODE_STRING      unicodeDeviceNameDos;

CHAR Copyright[] = "DbgPrint hook driver v" PROD_VER_MJ_STR "." PROD_VER_MN_STR PROD_VER_SUB "\n"
                   "Copyright (c) 2004-2016 by Alexander A. Telyatnikov\n"
                   "home site: http://alter.org.ua\n";

//PCHAR Buffer = NULL;

/*
PDbgPrnHk_GetMessages_USER_OUT MsgBuffer = NULL;
LARGE_INTEGER ReadPosition = {0, 0};
LARGE_INTEGER WritePosition = {0, 0};
ULONG BufferSize = 1024*1024;
ULONG BufferSizeMask = (1024*1024/sizeof(DbgPrnHk_GetMessages_USER_OUT))-1;
ULONG QueueSize = 0;
ULONG MaxQueueSize = 0;
BOOLEAN CheckIrql = TRUE;
BOOLEAN DoNotPassMessagesDown = FALSE;
BOOLEAN StopOnBufferOverflow = FALSE;
BOOLEAN LoggingPaused = FALSE;
*/

PDBGPRNHK_SIGNATURE_PAGE SigPage = NULL;

DBGPRNHK_INTERNAL_STATE st;

extern "C" {
PDBGPRNHK_INTERNAL_STATE DbgPrnHk_State = &st;
};
/*
VOID
NTAPI
DbgBreakPoint(
    VOID
    );
*/
__int64       RdtscTimeStampCalibration0 = 0;
LARGE_INTEGER SysTimeStampCalibration0   = {0,0};
__int64       RdtscTimeStampCalibration  = 0;
LARGE_INTEGER SysTimeStampCalibration    = {0,0};

PHYSICAL_ADDRESS ph1mb = {{0xFFFF, 0}};

typedef
ULONG
(*PDBG_PRINT_API) (
    PCH Format,
    ...
    );

typedef
NTSTATUS
(*PDEBUG_PRINT_API) (
    IN PSTRING Output
    );

typedef
NTSTATUS
(*PDEBUG_PRINTEX_API) (
    IN PSTRING Output,
    IN ULONG Arg1,
    IN ULONG Arg2
    );

typedef
NTSTATUS
(*PDEBUG_DISPATCH_2003_R2_API) (
    IN PCH Prefix,
    IN ULONG ComponentId,
    IN ULONG Level,
    IN PCH Format,
    IN va_list arglist,
    IN BOOLEAN ContinueExecution
    );

typedef
NTSTATUS
(__fastcall *PDEBUG_DISPATCH_VISTA_API) (
    IN PCH Prefix,
    IN ULONG Arg1,
    IN ULONG ComponentId,
    IN ULONG Level,
    IN PCH Format,
    IN va_list arglist,
    IN BOOLEAN ContinueExecution
    );

typedef
NTSTATUS
(*PDEBUG_DISPATCH_VISTA_API_STD) (
    IN PCH Prefix,
    IN ULONG Arg1,
    IN ULONG ComponentId,
    IN ULONG Level,
    IN PCH Format,
    IN va_list arglist,
    IN BOOLEAN ContinueExecution
    );

PDBG_PRINT_API              OrigDbgPrint = NULL;
PDEBUG_PRINT_API            OrigDebugPrint = NULL;
PDEBUG_PRINTEX_API          OrigDebugPrintEx = NULL;
//PDEBUG_DISPATCH_2003_R2_API OrigDebugDispatch = NULL;
PVOID                       callOrigDebugDispatch = NULL;
//PULONG                      DebugDispatchNextIp = NULL;
PDEBUG_DISPATCH_VISTA_API      OrigDebugDispatchVista = NULL;
PDEBUG_DISPATCH_VISTA_API_STD  OrigDebugDispatchVista_i2 = NULL;
ULONG                       VistaInvariant = 0;
PVOID                       hHook = NULL;

PUCHAR                      PatchAddress = NULL;
PVOID                       pvDbgPrintExWithPrefix = NULL;

ULONG                       EnterCount = 0;
BOOLEAN                     g_LockedBufferSize = FALSE;

ULONG                       g_CpuGen = 0;

ULONG                       DebugDispatchVistaParam;

KBUGCHECK_CALLBACK_RECORD   BugCheckCallbackRecord;

SYSTEM_BASIC_INFORMATION    SysInfo;

VOID
FreeSigPage();

NTSTATUS
HookDebugPrint (
    IN PSTRING Output
    );

NTSTATUS
HookDebugPrintEx (
    IN PSTRING Output,
    IN ULONG Arg1,
    IN ULONG Arg2
    );

ULONG
__fastcall
HookDebugDispatchVista(
    IN PCH Prefix,
    IN ULONG Arg1,
    IN ULONG ComponentId,
    IN ULONG Level,
    IN PCH Format,
    IN va_list arglist,
    IN BOOLEAN ContinueExecution
    );

ULONG
HookDebugDispatchVista_std(
    IN ULONG Arg1,
    IN ULONG ComponentId,
    IN ULONG Level,
    IN PCH Format,
    IN va_list arglist,
    IN BOOLEAN ContinueExecution
    );

VOID
PrintToBuffer(
    PDbgPrnHk_PostMessageEx_USER_IN MsgBuffer, /* optional */
    PCHAR Text,
    ULONG Length,
    ULONG Flags
    );

ULONG
ReadFromBuffer(
    IN OUT PDbgPrnHk_GetMessages_USER_OUT UserBuffer,
    IN ULONG Length
    );

NTSTATUS
KernelGetSysInfo(
    PSYSTEM_BASIC_INFORMATION SysInfo
    );
/*
PVOID
KernelGetModuleBase3(
    PDRIVER_OBJECT DriverObject,
    PCHAR  pModuleName
    );

PVOID
KernelGetModuleBase(
    PCHAR  pModuleName
    );

PVOID
KernelGetProcAddress(
    PVOID ModuleBase,
    PCHAR pFunctionName
    );
*/
NTSTATUS
DbgPrnHkReadRegistry();

NTSTATUS
DbgPrnHkUpdateST(
    PDBGPRNHK_INTERNAL_STATE st,
    PDBGPRNHK_INTERNAL_STATE opt
    );

VOID
DbgPrnHkBugCheckHandler(
    IN PVOID Buffer,
    IN ULONG Length
    );

NTSTATUS
DbgPrnHkGetCPUFreq();

__declspec (naked)
__int64
__fastcall
QueryRdtscPerformanceCounter()
{
  _asm {
    rdtsc
    ret
  }
} // end QueryRdtscPerformanceCounter()


__declspec (naked)
DWORD
__fastcall
PatchDword(
    DWORD* ptr,// ECX
    DWORD  val // EDX
    )
{
  _asm {

    // save flags and disable interrupts for current CPU
    pushfd
    cli

    // Clear WriteProtection bit in CR0:
    push ebx
    push ecx

    mov  ebx, cr0
    push ebx
    and  ebx, ~0x10000  // reset WriteProtect bit
    mov  cr0, ebx

    lock xchg [ecx],edx
    mov  eax,edx

    // Restore WriteProtection bit in CR0:
    pop ebx
    mov cr0, ebx

    pop ecx
    pop ebx

    // restore flags (e.g. interrupt flag)
    popfd

    ret
  }
} // end PatchDword()

/*
    DriverObject - Driver object.

    Returns:    NTSTATUS
 */

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT driverObject,
    IN PUNICODE_STRING RegistryPath
    )
{
    CCHAR       deviceNameBuffer[MAXIMUM_FILENAME_LENGTH];
    CCHAR       deviceNameBufferDos[MAXIMUM_FILENAME_LENGTH];
    ANSI_STRING         deviceName;
    ANSI_STRING         deviceNameDos;
    UNICODE_STRING      unicodeDeviceName;
    ULONG               i,j;
    NTSTATUS            status;
    PVOID               ModuleBase;
//    BOOL bRet;
    ULONG               count_50_e8;
    PUCHAR              f_ptr;
    BOOLEAN             found = FALSE;
    PHYSICAL_ADDRESS    ph_addr;
    BOOLEAN             ContiguousMemory = FALSE;
    BOOLEAN             saved_LoggingPaused;
    PULONG              buf_descr;
    // prevent compiler from unused string removal ;)
    DbgPrint(Copyright);
    //__asm int 3;
    //HalDisplayString((PUCHAR)"DbgPrnHk entry\n");

    // Pre-init driver state
    memset(&st, 0, sizeof(st));

    for(i=0; i<8; i++) {
        ph1mb.LowPart = (1 << (20+i)) - 1;
        SigPage = (PDBGPRNHK_SIGNATURE_PAGE)
            MmAllocateContiguousMemory(sizeof(DBGPRNHK_SIGNATURE_PAGE), ph1mb);
        if(SigPage) {
            ContiguousMemory = TRUE;
            break;
        }
    }
    if(!SigPage) {
        SigPage = (PDBGPRNHK_SIGNATURE_PAGE)
            ExAllocatePool(NonPagedPool, sizeof(DBGPRNHK_SIGNATURE_PAGE));
    }
    if(!SigPage || (ULONG)SigPage & (PAGE_SIZE-1)) {
        KdPrint(("DbgPrnHkInitialize: can't allocate signature page\n"));
        FreeSigPage();
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    memset(SigPage, 0, sizeof(DBGPRNHK_SIGNATURE));
    for(i=0; i<sizeof(SigPage->Signature); i+=sizeof(DBGPRNHK_SIGNATURE)) {
        memcpy(&(SigPage->Signature[i]), DBGPRNHK_SIGNATURE, sizeof(DBGPRNHK_SIGNATURE));
    }
    SigPage->st = &st;
    SigPage->state = 0;
    SigPage->ph_st = MmGetPhysicalAddress(SigPage->st);
    memset(&SigPage->ph_buf,       0, sizeof(SigPage->ph_buf));
    memset(&SigPage->ph_buf_descr, 0, sizeof(SigPage->ph_buf_descr));
    SigPage->buf_descr = NULL;

    ph_addr = MmGetPhysicalAddress(SigPage);
    DbgPrint("DbgPrnHkInitialize: Allocated signature page at %#x (phys %x)\n", SigPage, ph_addr.LowPart);
    DbgPrint("     use ***    !dbgprn.lsig %8.8x    *** in KD or WinDbg\n", SigPage);

    // Init driver state

    st.Ver.Major = PROD_VER_MJ;
    st.Ver.Minor = PROD_VER_MN;
    st.Ver.Sub   = PROD_VER_NSUB;

    st.MsgBuffer = NULL;
    st.ReadPosition.QuadPart = 0;
    st.WritePosition.QuadPart = 0;
    st.BufferSize = 1024*1024;
    st.BufferSizeMask = (st.BufferSize/sizeof(DbgPrnHk_GetMessages_USER_OUT))-1;
    st.QueueSize = 0;
    st.MaxQueueSize = 0;
    st.CheckIrql = TRUE;
    st.DoNotPassMessagesDown = DoNotPassMessages_Off;
    st.StopOnBufferOverflow = BufferOverflow_Continue;
    st.LoggingPaused = FALSE;
    st.TimeStampType = TimeStampType_SysPerfCounter;
    st.BugCheckRegistered = FALSE;
    st.MaxPage = 0;
    st.AggregateMessages = FALSE;
    st.SuppressDuplicates = FALSE;
    st.DumpToHalDisplay = FALSE;
    st.DumpStackFramePtr = FALSE;

    st.KdDebuggerEnabled = KdDebuggerEnabled;
    st.KdDebuggerNotPresent = KdDebuggerNotPresent;

    st.ContiguousMemory = ContiguousMemory;

    // Init driver object & K
    DriverObject = driverObject;
    KdPrint(("DbgPrnHkInitialize: call CrNtInit(%#x, %#x)\n", DriverObject, RegistryPath));
    if(!NT_SUCCESS(status = CrNtInit(DriverObject, RegistryPath))) {
        KdPrint(("DbgPrnHkInitialize: CrNtInit failed with status %#x\n", status));
        //HalDisplayString((PUCHAR)"DbgPrnHkInitialize: CrNtInit failed\n");
        FreeSigPage();
        return status;
    }
    g_CpuGen = CrNtGetCPUGen();
    KdPrint(("DbgPrnHkInitialize: OS ver %x.%x (%d) CPU Gen %d\n", MajorVersion, MinorVersion, BuildNumber & 0xffff, g_CpuGen));

    if(KernelGetSysInfo(&SysInfo) == STATUS_SUCCESS) {
        // check if we have enough physical memory
        if(SysInfo.NumberOfPhysicalPages < (1024*1024/PAGE_SIZE)) {
            DbgPrint("DbgPrnHkInitialize: bogus number of system pages: %x\n", SysInfo.NumberOfPhysicalPages);
            if(MajorVersion >= 5) {
                if(MinorVersion >= 2) {
                    SysInfo.NumberOfPhysicalPages = 64*1024*1024/PAGE_SIZE; //96Mb
                } else
                if(MinorVersion >= 1) {
                    SysInfo.NumberOfPhysicalPages = 48*1024*1024/PAGE_SIZE; //96Mb
                } else {
                    SysInfo.NumberOfPhysicalPages = 32*1024*1024/PAGE_SIZE; //32Mb
                }
            } else
            if(MajorVersion >= 4) {
                SysInfo.NumberOfPhysicalPages = 16*1024*1024/PAGE_SIZE; //16Mb
            } else 
            {
                SysInfo.NumberOfPhysicalPages = 12*1024*1024/PAGE_SIZE; //12Mb
            }
        } else {
            DbgPrint("DbgPrnHkInitialize: number of system pages: %x\n", SysInfo.NumberOfPhysicalPages);
        }
        st.MaxPage = SysInfo.NumberOfPhysicalPages;
        while(st.BufferSize/(4*PAGE_SIZE) > st.MaxPage) {
            st.BufferSize /= 2;
            st.BufferSizeMask /= 2;
        }
    }

    for(i=0; i<=IRP_MJ_MAXIMUM_FUNCTION; i++) {
        DriverObject->MajorFunction[i] = NULL;
    }

    // Set up the device driver entry points.
    DriverObject->MajorFunction[IRP_MJ_CREATE] = DbgPrnHkCreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP] = DbgPrnHkCreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = DbgPrnHkCreateClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DbgPrnHkDeviceControl;

    DriverObject->DriverUnload = DbgPrnHkUnload;

    sprintf((PCHAR)&deviceNameBuffer, NT_DbgPrnHk_NAME);
    KdPrint(("DbgPrnHkInitialize: Checking \\Device\\DbgPrnHk\n"));
    RtlInitAnsiString(&deviceName, (PCHAR)&deviceNameBuffer);
    RtlAnsiStringToUnicodeString(&unicodeDeviceName, &deviceName, TRUE);
    // Create the device object for DbgPrnHk.
    //HalDisplayString((PUCHAR)"DbgPrnHk: IoCreateDevice()\n");
    status = IoCreateDevice(DriverObject, 0 /*sizeof(LM_DEVICE_EXTENSION)*/, &unicodeDeviceName,
                        FILE_DEVICE_UNKNOWN, NULL, FALSE, &DbgPrnHkDevice);
    if(!NT_SUCCESS(status)) {
        //HalDisplayString((PUCHAR)"DbgPrnHk: IoCreateDevice() failed\n");
        RtlFreeUnicodeString(&unicodeDeviceName);
        FreeSigPage();
        return status;
    }

    sprintf((PCHAR)&deviceNameBufferDos, NT_DbgPrnHk_DOS_NAME);
    KdPrint(("DbgPrnHkInitialize: Checking \\DosDevices\\DbgPrnHk\n"));
    RtlInitAnsiString(&deviceNameDos, (PCHAR)&deviceNameBufferDos);
    RtlAnsiStringToUnicodeString(&unicodeDeviceNameDos, &deviceNameDos, TRUE);
    //HalDisplayString((PUCHAR)"DbgPrnHk: IoCreateSymbolicLink()\n");
    status = IoCreateSymbolicLink (&unicodeDeviceNameDos, &unicodeDeviceName);
    if(!NT_SUCCESS(status)) {
        //HalDisplayString((PUCHAR)"DbgPrnHk: IoCreateSymbolicLink() failed\n");
        IoDeleteDevice(DbgPrnHkDevice);
        RtlFreeUnicodeString(&unicodeDeviceName);
        RtlFreeUnicodeString(&unicodeDeviceNameDos);
        FreeSigPage();
        return status;
    }
    RtlFreeUnicodeString(&unicodeDeviceName);

    // get NTOSKRNL.EXE exports
    //HalDisplayString((PUCHAR)"DbgPrnHk: KernelGetModuleBase()\n");
    //ModuleBase = KernelGetModuleBase3 (DriverObject, "NTOSKRNL.EXE");
    //ModuleBase = KernelGetModuleBase ("NTOSKRNL.EXE");
    ModuleBase = CrNtGetModuleBase ("NTOSKRNL.EXE");
    KdPrint(("DbgPrnHkInitialize: NTOSKRNL.EXE ModuleBase %#x\n", ModuleBase));
    if(ModuleBase) {
        //HalDisplayString((PUCHAR)"DbgPrnHk: KernelGetProcAddress(DbgPrint)\n");
        OrigDbgPrint = (PDBG_PRINT_API)CrNtGetProcAddress(ModuleBase, "DbgPrint");
        //HalDisplayString((PUCHAR)"DbgPrnHk: KernelGetProcAddress(vDbgPrintExWithPrefix)\n");
        pvDbgPrintExWithPrefix = CrNtGetProcAddress(ModuleBase, "vDbgPrintExWithPrefix");
        //p_Kd_WIN2000_Mask = (PDBG_PRINT_API)CrNtGetProcAddress(ModuleBase, "Kd_WIN2000_Mask");
    } else {
        //HalDisplayString((PUCHAR)"DbgPrnHk: NTOSKRNL.EXE not found\n");
    }
    KdPrint(("DbgPrnHkInitialize: DbgPrint              Address %#x\n", OrigDbgPrint));
    KdPrint(("DbgPrnHkInitialize: vDbgPrintExWithPrefix Address %#x\n", pvDbgPrintExWithPrefix));
    if(OrigDbgPrint) {
        KdDump(OrigDbgPrint, 0x100);
    } else {
        //HalDisplayString((PUCHAR)"DbgPrnHk: DbgPrint not found\n");
        KdPrint(("DbgPrnHk: DbgPrint not found\n"));
        DbgPrnHkUnload(DriverObject);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    for(i=0; i<(ULONG)(*g_KeNumberProcessors); i++) {
        st.MsgBuffers[i] = (PCHAR)ExAllocatePool(NonPagedPool, 2048);
        if(!st.MsgBuffers[i]) {
            DbgPrnHkUnload(driverObject);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    //HalDisplayString((PUCHAR)"DbgPrnHk: RegGetKeyHandle()\n");
    status = DbgPrnHkReadRegistry();

    //HalDisplayString((PUCHAR)"DbgPrnHk: open reg ok 2\n");

    st.BufferSizeMask = (st.BufferSize / sizeof(DbgPrnHk_GetMessages_USER_OUT)) - 1;
    st.MaxQueueSize = st.BufferSizeMask-1; // keep 2 reserved empty items
    st.MsgBuffer = (PDbgPrnHk_GetMessages_USER_OUT)
        ExAllocatePool(NonPagedPool, st.BufferSize);
    if(!st.MsgBuffer) {
        //HalDisplayString((PUCHAR)"DbgPrnHk: !MsgBuffer\n");
        st.MsgBuffer = (PDbgPrnHk_GetMessages_USER_OUT)
            ExAllocatePool(PagedPool, st.BufferSize);
        if(st.MsgBuffer) {
            st.BufferMdl = IoAllocateMdl(st.MsgBuffer, st.BufferSize, FALSE, FALSE, NULL);
            if(st.BufferMdl) {
                __try {
                    MmProbeAndLockPages(st.BufferMdl, KernelMode, IoWriteAccess);
                } __except(EXCEPTION_EXECUTE_HANDLER) {
                    IoFreeMdl(st.BufferMdl);
                    st.BufferMdl = NULL;
                }
            }
            if(!st.BufferMdl) {
                ExFreePool(st.MsgBuffer);
                st.MsgBuffer = NULL;
            }
        }

        if(!st.MsgBuffer) {
            DbgPrnHkUnload(DriverObject);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    g_LockedBufferSize = TRUE;
    SigPage->ph_buf = MmGetPhysicalAddress(st.MsgBuffer);
    SigPage->buf_descr = buf_descr = (PULONG)ExAllocatePool(NonPagedPool, PAGE_SIZE);
    if(!buf_descr) {
        DbgPrnHkUnload(DriverObject);
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    SigPage->ph_buf_descr = MmGetPhysicalAddress(buf_descr);
    for(j=0,i=0; i<st.BufferSize; i+=PAGE_SIZE,j++) {
        PHYSICAL_ADDRESS ph_addr_d;
        if(j+2 == PAGE_SIZE/sizeof(ULONG)) {
            PULONG new_buf_descr;

            new_buf_descr = (PULONG)ExAllocatePool(NonPagedPool, PAGE_SIZE);
            ph_addr_d = MmGetPhysicalAddress(new_buf_descr);
            buf_descr[PAGE_SIZE/sizeof(ULONG)-2] = (ULONG)new_buf_descr;
            buf_descr[PAGE_SIZE/sizeof(ULONG)-1] = ph_addr_d.LowPart;
            j=0;
            buf_descr = new_buf_descr;
        }
        if(!(j & (PAGE_SIZE-1))) {
            buf_descr[0] = DBGPRNHK_POOLTAG;
            j++;
            memset(buf_descr+1, 0, PAGE_SIZE-sizeof(ULONG));
        }
        ph_addr_d = MmGetPhysicalAddress(st.MsgBuffer+i);
        buf_descr[j] = ph_addr_d.LowPart;
    }

    if(MajorVersion == 6) {
        KdPrint(("Mode Vista\n"));

        /*
          Under Vista debugger is invoked like under 2003 R2, via some common function,
          lets call it DebugDispatch().
          This function is called directly from all other debug functions we knew before,
          entry point of DebugDispatch() can be located through DbgPrint disassembly:

          Invariant 1 (fastcall):

              PUSH 1                                // 6a 01                    +0
              LEA  EAX, [ebp+arg4] ArgPtrArray      // 8d 45 0c (not checked)   +2
              PUSH EAX                              // 50       (not checked)   +5
              PUSH [ebp+arg0] Format                // ff 75 08                 +6
              MOV  ECX, SomePtr                     // b9 xx xx xx xx           +9
              PUSH 3                                // 6a 03                    +14
              PUSH 0x65                             // 6a 65                    +16
              CALL DebugDispatch                    // e8 xx xx xx xx           +18

          Invariant 2 (stdcall):

              PUSH 1                                // 6a 01                    +0
              LEA  EAX, [ebp+arg4] ArgPtrArray      // 8d 45 0c (not checked)   +2
              PUSH EAX                              // 50       (not checked)   +5
              PUSH [ebp+arg0] Format                // ff 75 08                 +6
              PUSH 3                                // 6a 03                    +9
              PUSH 0x65                             // 6a 65                    +11
              PUSH SomePtr                          // 68 xx xx xx xx           +13
              CALL DebugDispatch                    // e8 xx xx xx xx           +18

          DebugDispatch:
          ; __fastcall vDbgPrintExWithPrefixInternal(x, x, x, x, x, x)

        */
        f_ptr = (PUCHAR)OrigDbgPrint;
        KdPrint(("search @ %#x\n", f_ptr));
        for(i=0; i<0x20; i++) {
            if(
               f_ptr[i+0 ] == 0x6a &&
               f_ptr[i+1 ] == 0x01 &&
               f_ptr[i+6 ] == 0xff &&
               f_ptr[i+7 ] == 0x75 &&
               f_ptr[i+8 ] == 0x08 &&
               f_ptr[i+18] == 0xe8 &&
               TRUE) {

                VistaInvariant = 0;
                if(f_ptr[i+9 ] == 0xb9 &&
                   f_ptr[i+14] == 0x6a &&
                   f_ptr[i+15] == 0x03 &&
                   f_ptr[i+16] == 0x6a &&
                   f_ptr[i+17] == 0x65 &&
                   TRUE) {
                    KdPrint(("found call to DebugDispatch() fastcall invariant 1 @ %#x\n", f_ptr+i+18));
                    VistaInvariant = 1;
                } else
                if(f_ptr[i+9 ] == 0x6a &&
                   f_ptr[i+10] == 0x03 &&
                   f_ptr[i+11] == 0x6a &&
                   f_ptr[i+12] == 0x65 &&
                   f_ptr[i+13] == 0x68 &&
                   TRUE) {
                    KdPrint(("found call to DebugDispatch() stdcall invariant 2 @ %#x\n", f_ptr+i+18));
                    VistaInvariant = 2;
                } else {
                   continue;
                }
                // relative -> abs
                KdPrint(("found call to DebugDispatch() @ %#x\n", f_ptr+i+18));
                f_ptr = PtrRelativeToAbs(f_ptr+i+18+1);
                KdPrint(("found entry point of DebugDispatch() @ %#x\n", f_ptr));

                /*
                  here we have the following:
                  PUSH 0x228
                  PUSH caller's_int3_address
                  CALL xxxxxxxx
                */

                KdPrint(("search @ %#x\n", f_ptr));
                i=0;

                if(
                   f_ptr[i+0 ] == 0x68 &&
                   f_ptr[i+5 ] == 0x68 &&
                   f_ptr[i+10] == 0xe8 &&
                   TRUE) {
                    if(VistaInvariant == 1) {
                        OrigDebugDispatchVista = (PDEBUG_DISPATCH_VISTA_API)(f_ptr+i);
                        PatchAddress = (PUCHAR)OrigDebugDispatchVista;
                    } else
                    if(VistaInvariant == 2) {
                        OrigDebugDispatchVista_i2 = (PDEBUG_DISPATCH_VISTA_API_STD)(f_ptr+i);
                        PatchAddress = (PUCHAR)OrigDebugDispatchVista_i2;
                    }
                    KdPrint(("verified entry point of DebugDispatch()\n"));
                    callOrigDebugDispatch = f_ptr+i+5;
                    DebugDispatchVistaParam = *((PULONG)(f_ptr+i+1));
                    found = TRUE;
                    break;
                }
                found = FALSE;
                break;
            }
        }

    } else
    if(pvDbgPrintExWithPrefix) {
        KdPrint(("Mode XP/2003\n"));
        /*
          Under XP/2003 DebugPrint is called from vDbgPrintExWithPrefix() function.
          The ASM code of this call looks like this:

          PUSH EAX
          CALL xxxxxxxx
          CMP EAX,0x80000003

          We shall look for corresponding byte sequence
        */
        //HalDisplayString((PUCHAR)"Mode XP\n");
        f_ptr = (PUCHAR)pvDbgPrintExWithPrefix;
        KdPrint(("search @ %#x\n", f_ptr));
        for(i=0; i<0xff; i++) {
            if(f_ptr[i]   == 0x50 &&
               f_ptr[i+1] == 0xe8 &&
               f_ptr[i+6] == 0x3d &&
               (*(PULONG)(f_ptr+i+7)) == 0x80000003) {
                found = TRUE;
                PatchAddress = f_ptr+i+1+1;
                break;
            }
        }

        if(!found && WinVer_IsdNETp) {
            KdPrint(("Mode 2003 RC2\n"));
            /*
              Under 2003 RC2 debugger is invoked via some other function,
              lets call it DebugDispatch().
              This function is called directly from all other debug functions we knew before,
              e.g. vDbgPrintExWithPrefix(), DbgPrint(), etc.
              entry point of DebugDispatch() can be located through DbgPrint disassembly:

              PUSH 1                                // 6a 01                    +0
              LEA  EAX, [ebp+arg4] ArgPtrArray      // 8d 45 0c (not checked)   +2
              PUSH EAX                              // 50       (not checked)   +5
              PUSH [ebp+arg0] Format                // ff 75 08                 +6
              PUSH 0                                // 6a 00                    +9
              PUSH 0xFFFFFFFF                       // 6a ff                    +11
              PUSH caller's_int3_address            // 68 xx xx xx xx           +13
              CALL DebugDispatch                    // e8 xx xx xx xx           +18

              DebugDispatch:
              ; __stdcall vDbgPrintExWithPrefixInternal(x, x, x, x, x, x)

            */
            f_ptr = (PUCHAR)OrigDbgPrint;
            KdPrint(("search @ %#x\n", f_ptr));
            for(i=0; i<0xff; i++) {
                if(
                   f_ptr[i+0 ] == 0x6a &&
                   f_ptr[i+1 ] == 0x01 &&
                   f_ptr[i+6 ] == 0xff &&
                   f_ptr[i+7 ] == 0x75 &&
                   f_ptr[i+8 ] == 0x08 &&
                   f_ptr[i+9 ] == 0x6a &&
                   f_ptr[i+10] == 0x00 &&
                   f_ptr[i+11] == 0x6a &&
                   f_ptr[i+12] == 0xff &&
                   f_ptr[i+13] == 0x68 &&
                   f_ptr[i+18] == 0xe8 &&
                   TRUE) {
                    // relative -> abs
                    callOrigDebugDispatch = f_ptr+i+18;
                    KdPrint(("found call to DebugDispatch() @ %#x\n", f_ptr+i+18));
                    f_ptr = PtrRelativeToAbs(f_ptr+i+18+1);
                    KdPrint(("found entry point of DebugDispatch() @ %#x\n", f_ptr));

                    /*
                      here we have the following:
                      PUSH 0x228
                      PUSH caller's_int3_address
                      CALL xxxxxxxx
                      MOV  EAX,[BugCheckParameter]
                    */

                    KdPrint(("search @ %#x\n", f_ptr));
                    i=0;
                    //for(i=0; i<0xff; i++) {
                        if(
                           f_ptr[i+0 ] == 0x68 &&
                           f_ptr[i+5 ] == 0x68 &&
                           f_ptr[i+10] == 0xe8 &&
                           f_ptr[i+15] == 0xa1 &&
                           TRUE) {
                            //OrigDebugDispatch = (PDEBUG_DISPATCH_2003_R2_API)(f_ptr+i);
                            KdPrint(("verified entry point of DebugDispatch()\n"));

                            /*
                              Under 2003 RC2 DebugPrint is called from vDbgPrintExWithPrefix() function.
                              The ASM code of this call looks like this:

                              LEA  EAX, [ebp-xxx]     // 8d 85 xx xx xx xx        +0
                              PUSH EAX                // 50                       +6
                              CALL xxxxxxxx           // e8 xx xx xx xx           +7
                              CMP  byte [ebp+1Ch], 1  // 80 7d 1c 01              +12

                              We shall look for corresponding byte sequence
                            */

                            KdPrint(("search for DebugPrint @ %#x\n", f_ptr));
                            for(i=20; i<0xff; i++) {
                                if(
                                   f_ptr[i+0 ] == 0x8d &&
                                   f_ptr[i+1 ] == 0x85 &&
                                   f_ptr[i+6 ] == 0x50 &&
                                   f_ptr[i+7 ] == 0xe8 &&
                                   f_ptr[i+12] == 0x80 &&
                                   f_ptr[i+13] == 0x7d &&
                                   TRUE) {
                                    KdPrint(("found call to DebugPrint() @ %#x\n", f_ptr+7));

                                    found = TRUE;
                                    PatchAddress = f_ptr+i+7+1;
                                    break;
                                }
                            }
                            break;
                        }
                    //}
                    break;
                }
            }
        }

    } else
    if(MajorVersion == 5) {
        /*
          Under 2000 DebugPrint is called from DbgPrint() function.
          The ASM code of this call looks like this (same as XP):

          PUSH EAX
          CALL xxxxxxxx
          CMP EAX,0x80000003

          We shall look for corresponding byte sequence
        */
        //HalDisplayString((PUCHAR)"Mode v5\n");
        KdPrint(("Mode v5\n"));
        f_ptr = (PUCHAR)OrigDbgPrint;
        KdPrint(("search @ %#x\n", f_ptr));
        for(i=0; i<0xff; i++) {
            if(f_ptr[i]   == 0x50 &&
               f_ptr[i+1] == 0xe8 &&
               f_ptr[i+6] == 0x3d &&
               (*(PULONG)(f_ptr+i+7)) == 0x80000003) {
                found = TRUE;
                PatchAddress = f_ptr+i+1+1;
                break;
            }
        }
    } else
    if(MajorVersion == 4) {
        /*
        
          There is difference between NT4 and NT4-TS

          NT4-TS ASM on entry:            NT4-WS/SRV ASM on entry:
        
            push    ebp                     push    ebp                
            lea     eax, [esp+arg_4]        lea     eax, [esp+arg_4]   
            mov     ebp, esp                mov     ebp, esp           
            sub     esp, 208h               sub     esp, 208h          
            push    esi                     push    esi                
!           push    edi                     
            push    eax                     push    eax                
            mov     esi, 200h               mov     esi, 200h          
!                                           lea     eax, [ebp+var_208] 
            push    [ebp+arg_0]             push    [ebp+arg_0]        
            push    esi                     push    esi                
!           lea     eax, [ebp+var_208]      
            push    eax                     push    eax                
            call    _vsnprintf              call    _vsnprintf         
            add     esp, 10h                add     esp, 10h           




        */
        
        
        /*
          Under NT4 DebugPrint is called from DbgPrint() function.
          The ASM code of this call looks like this:

            PUSH EAX
            CALL xxxxxxxx

          this call is preceded with very similar call to vsnprintf():

            PUSH EAX
            CALL yyyyyyyy

          under NT-TS there are still 2 similar calls before call to DebugPring:

            PUSH EAX
            CALL InitLogFileName

            ...

            PUSH EAX
            CALL WriteLogToFile

          We shall look for the 2nd PUSH EAX/CALL xxxxxxxx byte sequence under NT4 and
          for the 4th one under NT4-TS
        */


        //HalDisplayString((PUCHAR)"Mode NT4\n");
        KdPrint(("Mode NT4\n"));
        f_ptr = (PUCHAR)OrigDbgPrint;
        KdPrint(("search @ %#x\n", f_ptr));

        if(f_ptr[13] == 0x56 &&   // push esi
           f_ptr[14] == 0x57 &&   // push edi
           f_ptr[15] == 0x50 &&   // push eax
           TRUE) {
            KdPrint(("Sub-Mode NT4-TS\n"));
            count_50_e8 = 4;
        } else
        if(f_ptr[13] == 0x56 &&   // push esi
           f_ptr[14] == 0x50 &&   // push eax
           f_ptr[15] == 0xbe &&   // mov esi,200h
           *((PULONG)&(f_ptr[16])) == 0x200 &&
           TRUE) {
            KdPrint(("Sub-Mode NT4-WS/SRV\n"));
            count_50_e8 = 2;
        }

        for(i=0; i<0xff; i++) {
            if(f_ptr[i]   == 0x50 &&
               f_ptr[i+1] == 0xe8) {
                if(!count_50_e8) {
                    KdPrint(("Not found\n"));
                    break;
                }
                count_50_e8--;
                if(count_50_e8 == 0) {
                    found = TRUE;
                    PatchAddress = f_ptr+i+1+1;
                    break;
                }
            }
        }

        // TODO: hook WriteLogToFile under NT4-TS

    } else 
    if(MajorVersion == 3) {

        /*
          Under NT3.51 DebugPrint is called from DbgPrint() function.
          The ASM code of this call looks like this:

          PUSH EAX
          MOV  [EBP-4],ECX
          CALL xxxxxxxx

          We shall look for the 2nd PUSH EAX/MOV  [EBP-4],ECX/CALL xxxxxxxx byte sequence
        */
        //HalDisplayString((PUCHAR)"Mode NT3.51\n");
        KdPrint(("Mode NT3.51\n"));
        f_ptr = (PUCHAR)OrigDbgPrint;
        KdPrint(("search @ %#x\n", f_ptr));
        for(i=0; i<0xff; i++) {
            if(f_ptr[i]     == 0x50 &&

               f_ptr[i+1]   == 0x89 &&
               f_ptr[i+2]   == 0x4d &&
               f_ptr[i+3]   == 0xfc &&  // -4

               f_ptr[i+4]   == 0xe8) {
                found = TRUE;
                PatchAddress = f_ptr+i+4+1;
                break;
            }
        }
    }
    if(!found || !PatchAddress) {
        //HalDisplayString((PUCHAR)"DbgPrnHk: entry point not found\n");
        KdPrint(("DbgPrnHk: no entry point found, PatchAddress %#x\n", PatchAddress));
        DbgPrnHkUnload(DriverObject);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //HalDisplayString((PUCHAR)"make patch\n");
    KdPrint(("DbgPrnHkInitialize: Patching call to DebugPrint @ %#x\n", PatchAddress));
    //========================================

    saved_LoggingPaused = st.LoggingPaused;
    st.LoggingPaused = TRUE;
    SigPage->state = 0x1;

    if(OrigDebugDispatchVista) {
        KdPrint(("DbgPrnHkInitialize: Vista KdPrintEx inv 1\n"));
        // patch entry point with JMP
        hHook = HookAtPtr(OrigDebugDispatchVista, HookDebugDispatchVista);
    } else
    if(OrigDebugDispatchVista_i2) {
        KdPrint(("DbgPrnHkInitialize: Vista KdPrintEx inv 2\n"));
        // patch entry point with JMP
        hHook = HookAtPtr(OrigDebugDispatchVista_i2, HookDebugDispatchVista_std);
    } else
    if(pvDbgPrintExWithPrefix) {
        KdPrint(("DbgPrnHkInitialize: XP KdPrintEx\n"));
        // patch CALL address (relative)
        OrigDebugPrintEx = (PDEBUG_PRINTEX_API)
            (PatchDword((PULONG)PatchAddress, (ULONG)HookDebugPrintEx - (ULONG)(PatchAddress+4)) +
             (ULONG)(PatchAddress+4));
        OrigDebugPrint = (PDEBUG_PRINT_API)OrigDebugPrintEx;
    } else {
        KdPrint(("DbgPrnHkInitialize: std KdPrint\n"));
        // patch CALL address (relative)
        OrigDebugPrint = (PDEBUG_PRINT_API)
            (PatchDword((PULONG)PatchAddress, (ULONG)HookDebugPrint - (ULONG)(PatchAddress+4)) +
             (ULONG)(PatchAddress+4));
    }

    //HalDisplayString((PUCHAR)"Patched...\n");
    KdPrint(("DbgPrnHkInitialize: Patched...\n"));
    st.LoggingPaused = saved_LoggingPaused;
    SigPage->state = 0x2;

    memset(&BugCheckCallbackRecord, 0, sizeof(BugCheckCallbackRecord));
    KeInitializeCallbackRecord(&BugCheckCallbackRecord);
    if(KeRegisterBugCheckCallback(&BugCheckCallbackRecord,
        DbgPrnHkBugCheckHandler,
        &st,
        sizeof(st),
        (PUCHAR)"DbgPrint Dump")) {
        st.BugCheckRegistered = TRUE;
    }

    if(g_CpuGen > 4) {
        __try {
            RdtscTimeStampCalibration0 = QueryRdtscPerformanceCounter();
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            RdtscTimeStampCalibration0 = 0;
            if(st.TimeStampType == TimeStampType_RdtscPerfCounter) {
                st.TimeStampType = TimeStampType_SysTime;
            }
        }
    } else {
        if(st.TimeStampType == TimeStampType_RdtscPerfCounter) {
            st.TimeStampType = TimeStampType_SysTime;
        }
    }
    SysTimeStampCalibration0 = KeQueryPerformanceCounter(NULL);

    KdPrint(("DbgPrnHkInitialize: RDTSC %I64d, SYS  %I64d\n", RdtscTimeStampCalibration0, SysTimeStampCalibration0.QuadPart));

    DbgPrnHk_State = &st;
    SigPage->state = 0x1000;

    return STATUS_SUCCESS;

} // DriverEntry()

/*
    This routine services open commands. It establishes
    the driver's existance by returning status success.

    Arguments:

    DeviceObject - Context for the activity.
    Irp          - The device control argument block.

    Returns:    NT Status
 */
NTSTATUS
DbgPrnHkCreateClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return STATUS_SUCCESS;
} // end DbgPrnHkCreateClose()


/*
    This device control dispatcher handles only the disk performance
    device control. All others are passed down to the disk drivers.
    The disk performane device control returns a current snapshot of
    the performance data.

    Arguments:

    DeviceObject - Context for the activity.
    Irp          - The device control argument block.

    Returns:    NT Status
 */
NTSTATUS
DbgPrnHkDeviceControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )
{
//    KdPrint(("DbgPrnHkDeviceControl\n"));
//    PLM_DEVICE_EXTENSION deviceExtension = (PLM_DEVICE_EXTENSION)DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION  irpStack = IoGetCurrentIrpStackLocation(Irp);
    ULONG               ioctlCode;
    NTSTATUS            status;
    ULONG               length;

    Irp->IoStatus.Information = 0;
    ioctlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;
    switch (ioctlCode) {
    case IOCTL_DbgPrnHk_GetMessages: {
        //KdPrint(("DbgPrnHk: GetMessages\n"));
        // We have <DeviceName> & its attributes.
        PDbgPrnHk_GetMessages_USER_OUT obuffer;

        length = sizeof(DbgPrnHk_GetMessages_USER_OUT); 
        CHECK_OUT_BUFFER();

        obuffer = (PDbgPrnHk_GetMessages_USER_OUT)(Irp->AssociatedIrp.SystemBuffer);

        if(!st.KMSaving.Enabled) {
            length = ReadFromBuffer(obuffer,
                irpStack->Parameters.DeviceIoControl.OutputBufferLength);
        } else {
            // If KMSaving is active, messages are taken out from buffer
            // from special kernel thread or BugCheck handler
            length = 0;
        }

        Irp->IoStatus.Information = length;
        status = STATUS_SUCCESS;
        break;
    }
    case IOCTL_DbgPrnHk_GetVersion: {
        //PDbgPrnHk_GetVersion_USER_OUT obuffer;

        length = sizeof(DbgPrnHk_GetVersion_USER_OUT); 
        CHECK_OUT_BUFFER();

/*
        obuffer = (PDbgPrnHk_GetVersion_USER_OUT)(Irp->AssociatedIrp.SystemBuffer);
        obuffer->Major = PROD_VER_MJ;
        obuffer->Minor = PROD_VER_MN;
        obuffer->Sub   = PROD_VER_NSUB;
*/
        memcpy(Irp->AssociatedIrp.SystemBuffer, &st.Ver, sizeof(DbgPrnHk_GetVersion_USER_OUT));

        Irp->IoStatus.Information = length;
        status = STATUS_SUCCESS;
        break;
    }
    case IOCTL_DbgPrnHk_PostMessage: {

        length = irpStack->Parameters.DeviceIoControl.InputBufferLength;

        PrintToBuffer(NULL, (PCHAR)(Irp->AssociatedIrp.SystemBuffer), length, MsgFlags_DbgCallerMode_U);

        Irp->IoStatus.Information = length;
        status = STATUS_SUCCESS;
        break;
    }
    case IOCTL_DbgPrnHk_GetRdtscCalibration: {
        PDbgPrnHk_GetRdtscCalibration_USER_IN  ibuffer;
        PDbgPrnHk_GetRdtscCalibration_USER_OUT obuffer;

        length = irpStack->Parameters.DeviceIoControl.InputBufferLength;
        if(length) {
            length = sizeof(DbgPrnHk_GetRdtscCalibration_USER_IN);
            CHECK_IN_BUFFER();
            ibuffer = (PDbgPrnHk_GetRdtscCalibration_USER_IN)(Irp->AssociatedIrp.SystemBuffer);
            if(ibuffer->Recalibrate) {
                if(g_CpuGen > 4) {
                    RdtscTimeStampCalibration = QueryRdtscPerformanceCounter();
                }
                SysTimeStampCalibration   = KeQueryPerformanceCounter(NULL);

                //KdPrint(("DbgPrnHkDeviceControl: RDTSC %I64d, SYS  %I64d\n", RdtscTimeStampCalibration, SysTimeStampCalibration.QuadPart));
            }
            if(ibuffer->ChangeType) {
                if(ibuffer->TimeStampType > TimeStampType_Max) {
                    status = STATUS_INVALID_PARAMETER;
                    break;
                }
                st.TimeStampType = ibuffer->TimeStampType;
                if(g_CpuGen < 5 &&
                   st.TimeStampType == TimeStampType_RdtscPerfCounter) {
                    st.TimeStampType = TimeStampType_SysTime;
                }
            }
        }

        length = irpStack->Parameters.DeviceIoControl.OutputBufferLength;
        if(length) {
            length = sizeof(DbgPrnHk_GetRdtscCalibration_USER_OUT);
            CHECK_OUT_BUFFER();
            if(!RdtscTimeStampCalibration && !SysTimeStampCalibration.QuadPart) {
                if(g_CpuGen > 4) {
                    __try {
                        RdtscTimeStampCalibration = QueryRdtscPerformanceCounter();
                    } __except(EXCEPTION_EXECUTE_HANDLER) {
                        RdtscTimeStampCalibration = 0;
                        if(st.TimeStampType == TimeStampType_RdtscPerfCounter) {
                            st.TimeStampType = TimeStampType_SysTime;
                        }
                    }
                } else {
                    if(st.TimeStampType == TimeStampType_RdtscPerfCounter) {
                        st.TimeStampType = TimeStampType_SysTime;
                    }
                }
                SysTimeStampCalibration   = KeQueryPerformanceCounter(NULL);
            }
            obuffer = (PDbgPrnHk_GetRdtscCalibration_USER_OUT)(Irp->AssociatedIrp.SystemBuffer);
            obuffer->RdtscTimeStampCalibration0 = RdtscTimeStampCalibration0;
            obuffer->SysTimeStampCalibration0   = SysTimeStampCalibration0.QuadPart;
            obuffer->RdtscTimeStampCalibration  = RdtscTimeStampCalibration;
            obuffer->SysTimeStampCalibration    = SysTimeStampCalibration.QuadPart;
            obuffer->TimeStampType              = st.TimeStampType;
        }

        Irp->IoStatus.Information = length;
        status = STATUS_SUCCESS;
        break;
    }
    case IOCTL_DbgPrnHk_GetRdtsc: {
        PLONGLONG obuffer;

        length = sizeof(LONGLONG);
        CHECK_OUT_BUFFER();

        obuffer = (PLONGLONG)(Irp->AssociatedIrp.SystemBuffer);
        __try {
            (*obuffer) = QueryRdtscPerformanceCounter();
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            status = STATUS_ILLEGAL_INSTRUCTION;
            break;
        }
        
        Irp->IoStatus.Information = length;
        status = STATUS_SUCCESS;
        break;
    }
    case IOCTL_DbgPrnHk_CanUnload: {
        ULONG Addr;

        if(pvDbgPrintExWithPrefix) {
            Addr = (ULONG)HookDebugPrintEx - (ULONG)(PatchAddress+4);
        } else {
            Addr = (ULONG)HookDebugPrint - (ULONG)(PatchAddress+4);
        }
        if(*(PULONG)(PatchAddress) == Addr) {
            status = STATUS_SUCCESS;
        } else {
            status = STATUS_UNSUCCESSFUL;
        }
        
        break;
    }
    case IOCTL_DbgPrnHk_PostMessageEx: {

        length = irpStack->Parameters.DeviceIoControl.InputBufferLength;

        PrintToBuffer((PDbgPrnHk_PostMessageEx_USER_IN)(Irp->AssociatedIrp.SystemBuffer),
            NULL,
            length - offsetof(DbgPrnHk_PostMessageEx_USER_IN, Msg),
            MsgFlags_DbgCallerMode_U);
        //status = PutToBuffer((PDbgPrnHk_GetMessages_USER_OUT)(Irp->AssociatedIrp.SystemBuffer), NULL, 0);

        Irp->IoStatus.Information = 0;
        status = STATUS_SUCCESS;
        break;
    }
    case IOCTL_DbgPrnHk_PostBinMessage: {

        length = irpStack->Parameters.DeviceIoControl.InputBufferLength;

        PrintToBuffer(NULL, (PCHAR)(Irp->AssociatedIrp.SystemBuffer), length,
            MsgFlags_DbgCallerMode_U | MsgFlags_DbgDataType_Bin);

        Irp->IoStatus.Information = length;
        status = STATUS_SUCCESS;
        break;
    }
    case IOCTL_DbgPrnHk_ReadRegConf: {

        status = DbgPrnHkReadRegistry();
    }
    case IOCTL_DbgPrnHk_GetDrvConf: {
        PDBGPRNHK_INTERNAL_STATE obuffer;

        length = sizeof(DBGPRNHK_INTERNAL_STATE);
        CHECK_OUT_BUFFER();

        obuffer = (PDBGPRNHK_INTERNAL_STATE)(Irp->AssociatedIrp.SystemBuffer);
        memcpy(obuffer, &st, length);
        
        Irp->IoStatus.Information = length;
        status = STATUS_SUCCESS;
        break;
    }
    case IOCTL_DbgPrnHk_SetDrvConf: {
        PDBGPRNHK_INTERNAL_STATE opt;

        length = sizeof(DBGPRNHK_INTERNAL_STATE);
        CHECK_IN_BUFFER();

        opt = (PDBGPRNHK_INTERNAL_STATE)(Irp->AssociatedIrp.SystemBuffer);

        status = DbgPrnHkUpdateST(&st, opt);
        if(status == STATUS_SUCCESS) {
            Irp->IoStatus.Information = length;
        }
        break;
    }
    default:

        status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

DbgPrnHkSpecDevCtrlCompleteRequest:

    Irp->IoStatus.Status = status;    // Update IRP with completion status.
    IoCompleteRequest(Irp, IO_DISK_INCREMENT);    // Complete the request.
//    KdPrint(( "DbgPrnHkDeviceControl: Status is %lx\n", status));
    return status;

} // end DbgPrnHkDeviceControl()

VOID
FreeSigPage()
{
    PULONG buf_descr;
    PULONG next_buf_descr;
    if(SigPage) {
        DbgPrint("DbgPrnHkUnload: Clean signature page at %#x\n", SigPage);
        buf_descr = SigPage->buf_descr;
        while(buf_descr) {
            next_buf_descr = (PULONG)buf_descr[PAGE_SIZE/sizeof(ULONG)-2];
            memset(buf_descr, 0, PAGE_SIZE);
            ExFreePool(buf_descr);
            buf_descr = next_buf_descr;
        }
        memset(SigPage, 0, sizeof(DBGPRNHK_SIGNATURE_PAGE));
        if(st.ContiguousMemory) {
            MmFreeContiguousMemory(SigPage);
        } else {
            ExFreePool(SigPage);
        }
        SigPage = NULL;
    }
    return;
} // end FreeSigPage()

/*
    This routine services driver unload commands.

    Arguments:

    DeviceObject - Context for the activity.

    Returns:    none
 */
VOID
DbgPrnHkUnload(
    IN PDRIVER_OBJECT DriverObject
    )
{
    LARGE_INTEGER Delay;
    ULONG Addr;
    ULONG i;

    Delay.QuadPart = -10*1000*1000; // 1 sec

    if(st.BugCheckRegistered) {
        KeDeregisterBugCheckCallback(&BugCheckCallbackRecord);
    }

    if(hHook) {
        UnHookAtPtr(hHook, 0);
        ReleaseHookAtPtr(hHook);
    } else
    if(pvDbgPrintExWithPrefix) {
        KdPrint(("DbgPrnHkInitialize: Prepare to Unpatch XP KdPrintEx\n"));
        Addr = (ULONG)HookDebugPrintEx - (ULONG)(PatchAddress+4);
    } else {
        Addr = (ULONG)HookDebugPrint - (ULONG)(PatchAddress+4);
    }

    if(!hHook) {
        while(PatchAddress) {
            if(*(PULONG)(PatchAddress) == Addr) {
                KdPrint(("DbgPrnHkInitialize: UnPatching call to DebugPrint @ %#x\n", PatchAddress-2));
                // restore CALL address (relative)
                if(pvDbgPrintExWithPrefix) {
                    KdPrint(("DbgPrnHkInitialize: Unpatch XP KdPrintEx\n"));
                    PatchDword((PULONG)PatchAddress, (ULONG)OrigDebugPrintEx - (ULONG)(PatchAddress+4));
                } else {
                    PatchDword((PULONG)PatchAddress, (ULONG)OrigDebugPrint - (ULONG)(PatchAddress+4));
                }

                KdPrint(("DbgPrnHkInitialize: UnPatched...\n"));
                break;
            }
            KdPrint(("DbgPrnHkInitialize: wait for Unpatch...\n"));
            KeDelayExecutionThread(KernelMode, FALSE, &Delay);
        }
    } else {
        hHook = NULL;
    }

    Delay.QuadPart = -1*1000*1000; // 0.1 sec
    while(EnterCount) {
        KdPrint(("DbgPrnHkInitialize: wait for not returned...\n"));
        KeDelayExecutionThread(KernelMode, FALSE, &Delay);
    }
    KeDelayExecutionThread(KernelMode, FALSE, &Delay);

    if(st.BufferMdl) {
        KdPrint(("DbgPrnHkUnload: unlock buffer\n"));
        MmUnlockPages(st.BufferMdl);
        IoFreeMdl(st.BufferMdl);
        st.BufferMdl = NULL;
    }
    if(st.MsgBuffer) {
        KdPrint(("DbgPrnHkUnload: free buffer\n"));
        ExFreePool(st.MsgBuffer);
    }

    for(i=0; i<(ULONG)(*g_KeNumberProcessors); i++) {
        if(st.MsgBuffers[i]) {
            ExFreePool(st.MsgBuffers[i]);
            st.MsgBuffers[i] = NULL;
        }
    }

    IoDeleteSymbolicLink(&unicodeDeviceNameDos);
    
    RtlFreeUnicodeString(&unicodeDeviceNameDos);

    if(DbgPrnHkDevice) {
        IoDeleteDevice(DbgPrnHkDevice);
        DbgPrnHkDevice = NULL;
    }

    FreeSigPage();

    return;
} // end DbgPrnHkUnload()

PDbgPrnHk_GetMessages_USER_OUT
GetBufferItem(
    BOOLEAN Next,
    PULONG  _i
    )
{
    ULONG PrevPos;
    ULONG i;
    ULONG qs;

    if(!Next) {
        // get current
        PrevPos = st.WritePosition.LowPart;
        if(!PrevPos && !st.WritePosition.HighPart) {
            (*_i) = 0;
            return NULL;
        }
        PrevPos--;
        i = PrevPos & st.BufferSizeMask;
    } else {

        // get next
        qs = CrNtInterlockedIncrement((PLONG)&st.QueueSize);
        if(st.StopOnBufferOverflow != BufferOverflow_Continue) {
            if(qs >= st.MaxQueueSize - 6) {

                qs = CrNtInterlockedDecrement((PLONG)&st.QueueSize);

                if(st.StopOnBufferOverflow == BufferOverflow_CallDebugger) {
                    // call kernel debugger
                    //DbgBreakPoint();
                    DbgPrint("DbgPrnHk: Debug message buffer is full.\n");
                    DbgPrint("  You can download messages with !dbgprn.save KD Extension command\n");
                    DbgPrint("    or view them with !dbgprn.ls command.\n");
                    DbgPrint("    When you continue execution previously captured logs will be lost.\n");
                    DbgPrint("  Read manual or attend http://alter.org.ua/soft/win/dbgdump\n");
                    DbgPrint("    for details\n");
                    __asm int 3;
/*
                    PrevPos = CrNtInterlockedExchangeAdd((PLONG)&st.ReadPosition.LowPart, qs);
                    if(PrevPos + qs < PrevPos) {
                        CrNtInterlockedIncrement((PLONG)&st.ReadPosition.HighPart);
                    }
*/
                    st.ReadPosition.QuadPart = st.WritePosition.QuadPart;
                    st.QueueSize = 0;

                } else {
                    (*_i) = 0;
                    return NULL;
                }
            }
        } else {
            if(qs >= st.MaxQueueSize) {
                PrevPos = CrNtInterlockedIncrement((PLONG)&st.ReadPosition.LowPart);
                if(!(PrevPos)) {
                    CrNtInterlockedIncrement((PLONG)&st.ReadPosition.HighPart);
                }
                qs = CrNtInterlockedDecrement((PLONG)&st.QueueSize);
            }
        }
        PrevPos = CrNtInterlockedIncrement((PLONG)&st.WritePosition.LowPart);
        if(!(PrevPos)) {
            CrNtInterlockedIncrement((PLONG)&st.WritePosition.HighPart);
        }
        PrevPos--;
        i = PrevPos & st.BufferSizeMask;

    }

    (*_i) = (i-1) & st.BufferSizeMask;
    return &st.MsgBuffer[i];
} // end GetBufferItem()

LONGLONG
GetTimeStamp()
{
    LONGLONG TimeStamp;
    switch(st.TimeStampType) {
    case TimeStampType_SysPerfCounter:
        return (LONGLONG)(KeQueryPerformanceCounter(NULL).QuadPart);

    case TimeStampType_RdtscPerfCounter:
        if(g_CpuGen > 4) {
            return QueryRdtscPerformanceCounter();
        }
        // FALL THROUGH

    case TimeStampType_SysTime:
        //MsgBuffer_i->TimeStamp.QuadPart = QueryRdtscPerformanceCounter();
        KeQuerySystemTime((PLARGE_INTEGER)&TimeStamp);
        return TimeStamp;
    }
    return 0;
} // end GetTimeStamp()

__declspec (naked)
PVOID
__stdcall
GetCurrentKThread(void) 
{
  _asm {
    mov eax,fs:[124h]
    ret
  }
} // end GetCurrentKThread()

VOID
PrintToBuffer(
    IN PDbgPrnHk_PostMessageEx_USER_IN MsgBuffer, /* optional */
    IN PCHAR Text,
    IN ULONG Length,
    IN ULONG Flags
    )
{
    ULONG to_write;
    PDbgPrnHk_GetMessages_USER_OUT MsgBuffer_i;
    union _DbgPrnHk_MsgFlags f;

    UCHAR  CpuNumber;
    HANDLE ProcessId;
    HANDLE ThreadId;
    KIRQL  Irql;
    ULONG  aggr_offs = 0;
    f.Flags = Flags;
    ULONG  i;

    LONGLONG TimeStamp;
    //BOOLEAN CallerMode = (ExGetPreviousMode() == UserMode);

    if(!MsgBuffer) {
        if(!Text)
            return;
        TimeStamp = GetTimeStamp();
        f.TimeStampType = st.TimeStampType;
        CpuNumber = (UCHAR)KeGetCurrentProcessorNumber();
        if(GetCurrentKThread()) {
            ProcessId = CrNtPsGetCurrentProcessId();
            ThreadId  = CrNtPsGetCurrentThreadId();
        } else /*__except(EXCEPTION_EXECUTE_HANDLER)*/ {
            // some API hook tools call DbgPrint having invalid FS
            // this will cause page fault inside PsGetXxx()
            ProcessId = (PVOID)0xffffffff;
            ThreadId  = (PVOID)0xffffffff;
        }
        Irql = KeGetCurrentIrql();
    } else {
        if(!MsgBuffer->TimeStamp.QuadPart) {
            MsgBuffer->TimeStamp.QuadPart = GetTimeStamp();
            f.TimeStampType = st.TimeStampType;
        }
        if(!Text) {
            Text = MsgBuffer->Msg;
            Length = min(MsgBuffer->Length, Length);
        }
        if(st.AggregateMessages) {
            TimeStamp = MsgBuffer->TimeStamp.QuadPart;
            ProcessId = MsgBuffer->ProcessId;
            ThreadId  = MsgBuffer->ThreadId;
            Irql      = MsgBuffer->Irql;
            CpuNumber = MsgBuffer->CpuNumber;
        }
    }

    if(st.DumpToHalDisplay) {
        UCHAR tmp[128];
        PUCHAR p;
        ULONG n, l;

        p = (PUCHAR)Text;
        for(n=Length; n>0; ) {
            l = min(Length, 127);
            memcpy(tmp, p, l);
            tmp[l] = 0;
            HalDisplayString((PUCHAR)tmp);
            n -= l;
            p += l;
        }
    }

    if(st.AggregateMessages || st.SuppressDuplicates) {
        MsgBuffer_i = GetBufferItem(FALSE, &i); // Get current and previous
        if(!MsgBuffer_i) {
            // the buffer is either full or empty
            // fall to normal processing
            goto no_aggr;
        }
        if((TimeStamp - MsgBuffer_i->TimeStamp.QuadPart > 10000) &&
           !st.SuppressDuplicates) {
            goto no_aggr;
        }
        if(MsgBuffer_i->ProcessId  != ProcessId ||
           MsgBuffer_i->ThreadId   != ThreadId  ||
           MsgBuffer_i->Irql       != Irql      ||
           MsgBuffer_i->CpuNumber  != CpuNumber/* ||
           MsgBuffer_i->Flags      != Flags*/
          ) {
            goto no_aggr;
        }
        if(st.SuppressDuplicates && Length) {
            // check if we have a dup
            PDbgPrnHk_GetMessages_USER_OUT MsgBuffToCheck;
            BOOLEAN FirstDup;
            ULONG old_i;

            if(MsgBuffer_i->DataType == DbgDataType_Dup) {
                MsgBuffToCheck = &(st.MsgBuffer[i]);
                FirstDup = FALSE;
            } else {
                MsgBuffToCheck = MsgBuffer_i;
                FirstDup = TRUE;
            }
//            if((Flags & MsgFlags_ValidMask) == MsgBuffToCheck->Flags &&
            if(f.Flags != MsgBuffToCheck->Flags) {
                goto no_aggr;
            }
            if(Length == MsgBuffToCheck->Length) {
                if(RtlCompareMemory(MsgBuffToCheck->Msg, Text, Length) == Length) {
                    if(FirstDup) {
                        old_i = i;
                        MsgBuffer_i = GetBufferItem(TRUE, &i); // Get next
                        if(((i - old_i) & st.BufferSizeMask) == 1) {
                            MsgBuffer_i->ProcessId     = ProcessId;
                            MsgBuffer_i->ThreadId      = ThreadId;
                            MsgBuffer_i->Irql          = Irql;
                            MsgBuffer_i->CpuNumber     = CpuNumber;
                            MsgBuffer_i->Flags         = f.Flags;
                            MsgBuffer_i->DataType      = DbgDataType_Dup;
                            MsgBuffer_i->TimeStamp.QuadPart = TimeStamp;
                            MsgBuffer_i->MsgDup.DupCounter  = 0;
                            MsgBuffer_i->Length        = sizeof(MsgBuffer_i->MsgDup);
                            return;
                        }
                    } else {
                        if(++MsgBuffer_i->MsgDup.DupCounter) {
                            MsgBuffer_i->TimeStamp.QuadPart = TimeStamp;
                            return;
                        }
                        MsgBuffer_i->MsgDup.DupCounter--;
                    }
                }
            }
        }
        if(MsgBuffer_i->Flags != f.Flags) {
            goto no_aggr;
        }
        to_write = min(Length, MAX_MSG_SIZE-MsgBuffer_i->Length);
        if(f.DataType == DbgDataType_Bin) {
            // to avoid alignment problems when dumping in text form in fmt_output.cpp
            to_write &= ~((ULONG)0xf);
        }
        if(to_write < Length) {
            goto no_aggr;
        }
        aggr_offs = MsgBuffer_i->Length;
        goto aggr;
    }

no_aggr:

    //HalDisplayString((PUCHAR)Text);
    while(Length)  {

        MsgBuffer_i = GetBufferItem(TRUE, &i); // Get next
        if(!MsgBuffer_i) {
            return;
        }

        if(st.DumpStackFramePtr/* && !aggr_offs*/) { // !aggr_offs is always TRUE here
            if(Length < sizeof(ULONG)) {
                f.WithSF = 0;
                to_write = min(Length, MAX_MSG_SIZE);
            } else {
                f.WithSF = 1;
                to_write = min(Length, MAX_MSG_SIZE-sizeof(ULONG));
            }
        } else {
            to_write = min(Length, MAX_MSG_SIZE);
        }
        if(f.DataType == DbgDataType_Bin) {
            // to avoid alignment problems when dumping in text form in fmt_output.cpp
            to_write &= ~((ULONG)0xf);
        }
        //if(!aggr_offs) { // is always TRUE here
        MsgBuffer_i->Length = 0;
        //}

        if(to_write) {
            if(!MsgBuffer) {
                MsgBuffer_i->ProcessId = ProcessId;
                MsgBuffer_i->ThreadId  = ThreadId;
                MsgBuffer_i->Irql      = Irql;
                MsgBuffer_i->CpuNumber = CpuNumber;
                MsgBuffer_i->TimeStamp.QuadPart = TimeStamp;
                //memset((&MsgBuffer_i->Reserved[0])-1, 0, 4);
                f.Flags &= MsgFlags_ValidMask;
                MsgBuffer_i->Flags     = f.Flags;
aggr:
                __try {
                    if(f.WithSF) {
                        MsgBuffer_i->StackFramePtr = (ULONG)&MsgBuffer;
                        memcpy(&MsgBuffer_i->Msg[sizeof(ULONG)], Text, to_write);
                        MsgBuffer_i->Length    += (USHORT)to_write + sizeof(ULONG);
                    } else {
                        memcpy(&MsgBuffer_i->Msg[aggr_offs], Text, to_write);
                        MsgBuffer_i->Length    += (USHORT)to_write;
                    }
                } __except(EXCEPTION_EXECUTE_HANDLER) {
                    return;
                }
            } else {
                __try {
                    if(!MsgBuffer->TimeStamp.QuadPart) {
                        MsgBuffer_i->TimeStamp.QuadPart = TimeStamp;
                    }
                    memcpy(MsgBuffer_i, MsgBuffer, offsetof(DbgPrnHk_GetMessages_USER_OUT, Msg));
                    memcpy(&MsgBuffer_i->Msg[aggr_offs], Text, to_write);
                    MsgBuffer_i->Length = (USHORT)to_write;
                } __except(EXCEPTION_EXECUTE_HANDLER) {
                    return;
                }
            }

            Text += to_write;
            Length -= to_write;
        }
        aggr_offs = 0;
    }
    return;
} // end PrintToBuffer()

ULONG
ReadFromBuffer(
    IN OUT PDbgPrnHk_GetMessages_USER_OUT UserBuffer,
    IN ULONG Length
    )
{
    ULONG PrevPos;
    ULONG i;
    ULONG to_write = 0;

    while(Length >= sizeof(DbgPrnHk_GetMessages_USER_OUT))  {
        if(st.ReadPosition.QuadPart >= st.WritePosition.QuadPart) {
            break;
        }
        PrevPos = CrNtInterlockedIncrement((PLONG)&st.ReadPosition.LowPart);
        if(!(PrevPos)) {
            CrNtInterlockedIncrement((PLONG)&st.ReadPosition.HighPart);
        }
        PrevPos--;
        i = PrevPos & st.BufferSizeMask;
        __try {
            memcpy(UserBuffer, &st.MsgBuffer[i], sizeof(DbgPrnHk_GetMessages_USER_OUT));
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            break;
        }
        UserBuffer++;
        to_write += sizeof(DbgPrnHk_GetMessages_USER_OUT);
        Length -= sizeof(DbgPrnHk_GetMessages_USER_OUT);
        CrNtInterlockedDecrement((PLONG)&st.QueueSize);
    }
    return to_write;
} // end ReadFromBuffer()

#define IsMsgBlocked() \
    (st.DoNotPassMessagesDown || \
     (st.CheckIrql && (KeGetCurrentIrql() > DISPATCH_LEVEL)) )
/*
BOOLEAN
IsMsgBlocked()
{
    if(DoNotPassMessagesDown)
        return TRUE;
    if(CheckIrql && (KeGetCurrentIrql() > DISPATCH_LEVEL))
        return TRUE;
    return FALSE;
} // end IsMsgBlocked()
*/
NTSTATUS
HookDebugPrint(
    IN PSTRING Output
    )
{
    NTSTATUS status;

    InterlockedIncrement((PLONG)&EnterCount);

    SigPage->state = 0x100;

    status = STATUS_SUCCESS;
    if(Output && !st.LoggingPaused) {
        SigPage->state = 0x101;
        PrintToBuffer(NULL, Output->Buffer, Output->Length, MsgFlags_DbgCallerMode_K);
        SigPage->state = 0x110;
    }
    if(!OrigDebugPrint)
        goto exit;
    if(IsMsgBlocked())
        goto exit;
    SigPage->state = 0x120;
    status = OrigDebugPrint(Output);
    SigPage->state = 0x130;
exit:
    SigPage->state = 0x1000;
    InterlockedDecrement((PLONG)&EnterCount);
    return status;
} // end HookDebugPrint()

NTSTATUS
HookDebugPrintEx(
    IN PSTRING Output,
    IN ULONG   Arg1,
    IN ULONG   Arg2
    )
{
    NTSTATUS status;

    InterlockedIncrement((PLONG)&EnterCount);

    status = STATUS_SUCCESS;
    if(Output && !st.LoggingPaused) {
        PrintToBuffer(NULL, Output->Buffer, Output->Length, MsgFlags_DbgCallerMode_K);
    }
    if(!OrigDebugPrintEx)
        goto exit;
    if(IsMsgBlocked())
        goto exit;
    status = OrigDebugPrintEx(Output, Arg1, Arg2);
exit:
    InterlockedDecrement((PLONG)&EnterCount);
    return status;
} //end HookDebugPrintEx()

ULONG
_HookDebugDispatchVista(
//    IN PCH Prefix,
    IN ULONG ComponentId,
    IN ULONG Level,
    IN PCH Format,
    IN va_list arglist,
    IN BOOLEAN ContinueExecution
    )
{
    BOOLEAN retval = TRUE;
    InterlockedIncrement((PLONG)&EnterCount);

    if(Format && !st.LoggingPaused) {
        _asm pushfd
        _asm cli

        st.MsgBuffersSz[KeGetCurrentProcessorNumber()] =
            _vsnprintf(st.MsgBuffers[KeGetCurrentProcessorNumber()], 2048, Format, arglist);
        PrintToBuffer(NULL, st.MsgBuffers[KeGetCurrentProcessorNumber()],
                            st.MsgBuffersSz[KeGetCurrentProcessorNumber()],
                            MsgFlags_DbgCallerMode_K);

        _asm popfd
    }
    if(!OrigDebugDispatchVista) {
        retval = FALSE;
    } else
    if(ContinueExecution && IsMsgBlocked()) {
        retval = FALSE;
    } 

    InterlockedDecrement((PLONG)&EnterCount);
    return retval;

} //end _HookDebugDispatchVista()

__declspec(naked)
ULONG
__fastcall
HookDebugDispatchVista(
    IN PCH Prefix,
    IN ULONG Arg1,
    IN ULONG ComponentId,
    IN ULONG Level,
    IN PCH Format,
    IN va_list arglist,
    IN BOOLEAN ContinueExecution
    )
{
  __asm {
    push ebp
    mov  ebp,esp

    push ebx
    push ecx
    push edx
    push esi
    push edi

    push dword ptr [ebp+0x18]
    push dword ptr [ebp+0x14]
    push dword ptr [ebp+0x10]
    push dword ptr [ebp+0x0c]
    push dword ptr [ebp+8]

    call _HookDebugDispatchVista;

    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx

    pop ebp

    or eax,eax
    jz exit

    push dword ptr [DebugDispatchVistaParam];

    jmp dword ptr [callOrigDebugDispatch];

exit:

//    xor eax,eax
    ret 14h;
  }

} //end HookDebugDispatchVista()

__declspec(naked)
ULONG
HookDebugDispatchVista_std(
    IN ULONG Arg1,
    IN ULONG ComponentId,
    IN ULONG Level,
    IN PCH Format,
    IN va_list arglist,
    IN BOOLEAN ContinueExecution
    )
{
  __asm {
    push ebp
    mov  ebp,esp

    push ebx
    push ecx
    push edx
    push esi
    push edi

    push dword ptr [ebp+0x18]
    push dword ptr [ebp+0x14]
    push dword ptr [ebp+0x10]
    push dword ptr [ebp+0x0c]
    push dword ptr [ebp+8]

    call _HookDebugDispatchVista;

    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx

    pop ebp

    or eax,eax
    jz exit

    push dword ptr [DebugDispatchVistaParam];

    jmp dword ptr [callOrigDebugDispatch];

exit:

//    xor eax,eax
    ret 18h;
  }

} //end HookDebugDispatchVista_std()

NTSTATUS
KernelGetSysInfo(
    PSYSTEM_BASIC_INFORMATION SysInfo
    )
{
    NTSTATUS status;
    ULONG    SystemInfoBufferSize = sizeof(SYSTEM_BASIC_INFORMATION);

    KdPrint(("KernelGetSysInfo:\n"));
    memset(SysInfo, 0, sizeof(*SysInfo));
    KdPrint(("CrNtNtQuerySystemInformation = %x\n", CrNtNtQuerySystemInformation));
    status = CrNtNtQuerySystemInformation(SystemBasicInformation,
        SysInfo,
        SystemInfoBufferSize,
        &SystemInfoBufferSize);

    if(!NT_SUCCESS(status)) {
        memset(SysInfo, 0, sizeof(*SysInfo));
    }
    return status;
} // end KernelGetSysInfo()

NTSTATUS
DbgPrnHkGetCPUFreq()
{
    NTSTATUS            status;
    HANDLE              hReg = NULL;
    ULONG               tmp;

    status = RegGetKeyHandle(NULL, NT_CPUDESCR_REG_PATH, &hReg);
    if(hReg) {
        tmp = 0;
        RegGetDword(hReg, NULL, L"~MHz", &tmp);
        ZwClose(hReg);
        KdPrint(("DbgPrnHkInitialize: CPU0 %d MHz\n", tmp));
    }
    return status;
} // end DbgPrnHkGetCPUFreq()

VOID
DbgPrnHkReadRegBoolean(
    IN HANDLE    hReg,
    IN PWSTR     Name,
 IN OUT PBOOLEAN Val
    )
{
    ULONG               tmp;

    tmp = (*Val);
    RegGetDword(hReg, NULL, Name, &tmp);
    if(tmp <= 1) {
        (*Val) = (UCHAR)tmp;
    }
    KdPrint(("DbgPrnHkInitialize: %S=%#x\n", Name, (*Val)));
    return;
} // end DbgPrnHkReadRegBoolean()

VOID
DbgPrnHkReadRegUchar(
    IN HANDLE    hReg,
    IN PWSTR     Name,
 IN OUT PUCHAR   Val
    )
{
    ULONG               tmp;

    tmp = (*Val);
    RegGetDword(hReg, NULL, Name, &tmp);
    if(tmp <= 1) {
        (*Val) = (UCHAR)tmp;
    }
    KdPrint(("DbgPrnHkInitialize: %S=%#x\n", Name, (*Val)));
    return;
} // end DbgPrnHkReadRegUchar()

VOID
DbgPrnHkReadRegDword(
    IN HANDLE    hReg,
    IN PWSTR     Name,
 IN OUT PULONG   Val
    )
{
    ULONG               tmp;

    tmp = (*Val);
    RegGetDword(hReg, NULL, Name, &tmp);
    if(tmp <= 1) {
        (*Val) = tmp;
    }
    KdPrint(("DbgPrnHkInitialize: %S=%#x\n", Name, (*Val)));
    return;
} // end DbgPrnHkReadRegDword()

VOID
DbgPrnHkReadRegStr(
    IN HANDLE    hReg,
    IN PWSTR     Name,
 IN OUT PWCHAR*  Val
    )
{
    if(*Val) {
        ExFreePool(*Val);
        (*Val) = NULL;
        (*Val) = (PWCHAR)ExAllocatePool(NonPagedPool, 257*sizeof(WCHAR));
    }
    if(!(*Val))
        return;
    RegGetStringValue(hReg, NULL, NT_DBGPRNHK_REG_KM_SAVING_VAL, *Val, 257*sizeof(WCHAR));
    KdPrint(("DbgPrnHkInitialize: %S='%S'\n", Name, (*Val)));
    return;
} // end DbgPrnHkReadRegStr()

NTSTATUS
DbgPrnHkReadRegistry()
{
    NTSTATUS            status;
    HANDLE              hReg = NULL;
    ULONG               i;
    ULONG               tmp;

    status = RegGetKeyHandle(NULL, NT_DBGPRNHK_REG_PATH, &hReg);
    if(hReg) {

        //HalDisplayString((PUCHAR)"DbgPrnHk: open reg ok\n");
        
        if(!g_LockedBufferSize) {
            st.BufferSize /= 1024;
            RegGetDword(hReg, NULL, NT_DBGPRNHK_REG_BUFFER_SIZE_VAL, &st.BufferSize);
            st.BufferSize *= 1024;
            i=0;
            while(st.BufferSize) {
                st.BufferSize >>= 1;
                i++;
            }
            if(i<8) {
                i = 8;
            } else {
                i--;
            }
            st.BufferSize = ((ULONG)1) << i;
            // check if we have enough physical memory
            if(st.MaxPage) {
                while(st.BufferSize/(4*PAGE_SIZE) > st.MaxPage) {
                    st.BufferSize /= 2;
                    st.BufferSizeMask /= 2;
                }
            }

            KdPrint(("DbgPrnHkInitialize: %S %#x\n", NT_DBGPRNHK_REG_BUFFER_SIZE_VAL, st.BufferSize));
            st.BufferSizeMask = (st.BufferSize/sizeof(DbgPrnHk_GetMessages_USER_OUT))-1;
            KdPrint(("DbgPrnHkInitialize: BufferSizeMask %#x\n", st.BufferSizeMask));
        }

        DbgPrnHkReadRegBoolean(hReg, NT_DBGPRNHK_REG_CHECK_IRQL_VAL, &st.CheckIrql);

        DbgPrnHkReadRegUchar(hReg, NT_DBGPRNHK_REG_BLOCK_MSG_VAL, &st.DoNotPassMessagesDown);
        if(st.DoNotPassMessagesDown == DoNotPassMessages_NoBuffer) {
            st.LoggingPaused = TRUE;
        } else {
            st.LoggingPaused = FALSE;
        }

        tmp = st.StopOnBufferOverflow;
        RegGetDword(hReg, NULL, NT_DBGPRNHK_REG_STOP_OVERFLOW_VAL, &tmp);
        if(tmp > BufferOverflow_Max) {
            st.StopOnBufferOverflow = BufferOverflow_Stop;
        } else {
            st.StopOnBufferOverflow = (UCHAR)tmp;
        }
        KdPrint(("DbgPrnHkInitialize: %S %#x\n", NT_DBGPRNHK_REG_STOP_OVERFLOW_VAL, st.StopOnBufferOverflow));

        tmp = st.TimeStampType;
        RegGetDword(hReg, NULL, NT_DBGPRNHK_REG_TSTAMP_TYPE_VAL, &tmp);
        if(tmp <= TimeStampType_Max) {
            st.TimeStampType = (UCHAR)tmp;
        }
        KdPrint(("DbgPrnHkInitialize: %S %#x\n", NT_DBGPRNHK_REG_TSTAMP_TYPE_VAL, st.TimeStampType));

        DbgPrnHkReadRegBoolean(hReg, NT_DBGPRNHK_REG_AGGREGATE_MSG_VAL, &st.AggregateMessages);

        DbgPrnHkReadRegBoolean(hReg, NT_DBGPRNHK_REG_SUPPRESS_DUPS_VAL, &st.SuppressDuplicates);

        DbgPrnHkReadRegBoolean(hReg, NT_DBGPRNHK_REG_HAL_DISPLAY_VAL, &st.DumpToHalDisplay);

        DbgPrnHkReadRegBoolean(hReg, NT_DBGPRNHK_REG_STACK_PTR_VAL, &st.DumpStackFramePtr);

        /* KM Saving */
        DbgPrnHkReadRegBoolean(hReg, NT_DBGPRNHK_REG_KM_SAVING_VAL, &st.KMSaving.Enabled);

        if(st.KMSaving.Enabled) {
            DbgPrnHkReadRegBoolean(hReg, NT_DBGPRNHK_REG_KMS_OWN_DEV_DRV_VAL, &st.KMSaving.KMSOwnDevDriver);

            DbgPrnHkReadRegDword(hReg, NT_DBGPRNHK_REG_KMS_TARGET_TYPE_VAL, &st.KMSaving.KMSTargetType);

            DbgPrnHkReadRegStr(hReg, NT_DBGPRNHK_REG_KMS_TARGET_FILE_VAL, &st.KMSaving.KMSTargetFile);

            DbgPrnHkReadRegStr(hReg, NT_DBGPRNHK_REG_KMS_TARGET_IP_VAL, &st.KMSaving.KMSTargetIP);
            DbgPrnHkReadRegDword(hReg, NT_DBGPRNHK_REG_KMS_TARGET_PORT_VAL, &st.KMSaving.KMSTargetPort);

            DbgPrnHkReadRegStr(hReg, NT_DBGPRNHK_REG_KMS_LOCAL_IP_VAL, &st.KMSaving.KMSLocalIP);
            DbgPrnHkReadRegDword(hReg, NT_DBGPRNHK_REG_KMS_LOCAL_PORT_VAL, &st.KMSaving.KMSLocalPort);

            DbgPrnHkReadRegDword(hReg, NT_DBGPRNHK_REG_KMS_PCI_ADDR_VAL, &st.KMSaving.KMSPCIAddr);

            DbgPrnHkReadRegDword(hReg, NT_DBGPRNHK_REG_KMS_STREAM_TYPE_VAL, &st.KMSaving.KMSStreamType);

            DbgPrnHkReadRegDword(hReg, NT_DBGPRNHK_REG_KMS_BLOCK_SIZE_VAL, &st.KMSaving.KMSBlockSize);
        }

        ZwClose(hReg);
    }
    return status;
} // end DbgPrnHkReadRegistry()

NTSTATUS
DbgPrnHkUpdateST(
    PDBGPRNHK_INTERNAL_STATE st,
    PDBGPRNHK_INTERNAL_STATE opt
    )
{
    if(opt->Ver.Major != PROD_VER_MJ ||
       opt->Ver.Minor != PROD_VER_MN ||
       opt->Ver.Sub   != PROD_VER_NSUB) {
        return STATUS_UNSUCCESSFUL;
    }

    #include "update_st.h"

    return STATUS_SUCCESS;
} // end DbgPrnHkUpdateST()

VOID
DbgPrnHkBugCheckHandler(
    IN PVOID Buffer,
    IN ULONG Length
    )
{
    __asm pushfd;
    __asm cli;

    //PrintToBuffer(NULL, "...and BugCheck Happened...\n", sizeof("...and BugCheck Happened...\n")-sizeof(CHAR), MsgFlags_DbgCallerMode_K);

    __asm popfd;
    return;
} // end DbgPrnHkBugCheckHandler()
