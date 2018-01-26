/*++ BUILD Version: 0063    // Increment this if a change has global effects

Copyright (c) 1990-1994  Microsoft Corporation

Module Name:

    windbgkd.h

Abstract:

    This module defines the 32-Bit Windows Kernel Debugger interface.

Revision History:

--*/

#ifndef _WINDBGKD_
#define _WINDBGKD_

typedef LARGE_INTEGER PHYSICAL_ADDRESS, *PPHYSICAL_ADDRESS; 
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

#ifdef _X86_

//
// DBGKD_CONTROL_REPORT
//
// This structure contains machine specific data passed to the debugger
// when a Wait_State_Change message is sent.  Idea is to allow debugger
// to do what it needes without reading any more packets.
// Structure is filled in by KdpSetControlReport
//

#define DBGKD_MAXSTREAM 16

typedef struct _DBGKD_CONTROL_REPORT {
    DWORD   Dr6;
    DWORD   Dr7;
    WORD    InstructionCount;
    WORD    ReportFlags;
    BYTE    InstructionStream[DBGKD_MAXSTREAM];
    WORD    SegCs;
    WORD    SegDs;
    WORD    SegEs;
    WORD    SegFs;
    DWORD   EFlags;
} DBGKD_CONTROL_REPORT, *PDBGKD_CONTROL_REPORT;

#define REPORT_INCLUDES_SEGS    0x0001  // this is for backward compatibility

//
// DBGKD_CONTROL_SET
//
// This structure control value the debugger wants to set on every
// continue, and thus sets here to avoid packet traffic.
//

typedef struct _DBGKD_CONTROL_SET {
    DWORD   TraceFlag;                  // WARNING: This must NOT be a BOOLEAN,
                                        //     or host and target will end
                                        //     up with different alignments!
    DWORD   Dr7;
    DWORD   CurrentSymbolStart;         // Range in which to trace locally
    DWORD   CurrentSymbolEnd;
} DBGKD_CONTROL_SET, *PDBGKD_CONTROL_SET;

#endif //_X86_


typedef struct _DESCRIPTOR_TABLE_ENTRY {
    DWORD Selector;
    LDT_ENTRY Descriptor;
} DESCRIPTOR_TABLE_ENTRY, *PDESCRIPTOR_TABLE_ENTRY;


#if defined(_MIPS_)

// end_ntddk end_nthal

//
// Define MIPS specific kernel debugger information.
//
// The following structure contains machine specific data passed to
// the host system kernel debugger in a wait state change message.
//

#define DBGKD_MAXSTREAM 16

typedef struct _DBGKD_CONTROL_REPORT {
    DWORD InstructionCount;
    BYTE  InstructionStream[DBGKD_MAXSTREAM];
} DBGKD_CONTROL_REPORT, *PDBGKD_CONTROL_REPORT;

//
// The following structure contains information that the host system
// kernel debugger wants to set on every continue operation and avoids
// the need to send extra packets of information.
//

typedef DWORD DBGKD_CONTROL_SET, *PDBGKD_CONTROL_SET;

#endif                          // ntddk nthal

#ifdef _ALPHA_

//
// Define Alpha specific kernel debugger information.
//
// The following structure contains machine specific data passed to
// the host system kernel debugger in a wait state change message.
//

#define DBGKD_MAXSTREAM 16

typedef struct _DBGKD_CONTROL_REPORT {
    DWORD InstructionCount;
    BYTE  InstructionStream[DBGKD_MAXSTREAM];
} DBGKD_CONTROL_REPORT, *PDBGKD_CONTROL_REPORT;

//
// The following structure contains information that the host system
// kernel debugger wants to set on every continue operation and avoids
// the need to send extra packets of information.
//

typedef DWORD DBGKD_CONTROL_SET, *PDBGKD_CONTROL_SET;

#endif // _ALPHA_

#if defined(_PPC_)


//
// Define PowerPC specific kernel debugger information.
//
// The following structure contains machine specific data passed to
// the host system kernel debugger in a wait state change message.
//
// **FINISH**  Copied without change from MIPS; may need modification

#define DBGKD_MAXSTREAM 16

typedef struct _DBGKD_CONTROL_REPORT {
    DWORD InstructionCount;
    BYTE  InstructionStream[DBGKD_MAXSTREAM];
} DBGKD_CONTROL_REPORT, *PDBGKD_CONTROL_REPORT;

//
// The following structure contains information that the host system
// kernel debugger wants to set on every continue operation and avoids
// the need to send extra packets of information.
//

typedef DWORD DBGKD_CONTROL_SET, *PDBGKD_CONTROL_SET;

#endif // defined(_PPC_)

#ifdef _PPC_
//
// Special Registers for PowerPC
//

typedef struct _KSPECIAL_REGISTERS {
    DWORD  KernelDr0;
    DWORD  KernelDr1;
    DWORD  KernelDr2;
    DWORD  KernelDr3;
    DWORD  KernelDr4;
    DWORD  KernelDr5;
    DWORD  KernelDr6;
    DWORD  KernelDr7;
    DWORD  Sprg0;
    DWORD  Sprg1;
    DWORD  Sr0;
    DWORD  Sr1;
    DWORD  Sr2;
    DWORD  Sr3;
    DWORD  Sr4;
    DWORD  Sr5;
    DWORD  Sr6;
    DWORD  Sr7;
    DWORD  Sr8;
    DWORD  Sr9;
    DWORD  Sr10;
    DWORD  Sr11;
    DWORD  Sr12;
    DWORD  Sr13;
    DWORD  Sr14;
    DWORD  Sr15;
    DWORD  DBAT0L;
    DWORD  DBAT0U;
    DWORD  DBAT1L;
    DWORD  DBAT1U;
    DWORD  DBAT2L;
    DWORD  DBAT2U;
    DWORD  DBAT3L;
    DWORD  DBAT3U;
    DWORD  IBAT0L;
    DWORD  IBAT0U;
    DWORD  IBAT1L;
    DWORD  IBAT1U;
    DWORD  IBAT2L;
    DWORD  IBAT2U;
    DWORD  IBAT3L;
    DWORD  IBAT3U;
    DWORD  Sdr1;
    DWORD  Reserved[9];
} KSPECIAL_REGISTERS, *PKSPECIAL_REGISTERS;

//
// Processor State structure.
//

typedef struct _KPROCESSOR_STATE {
    struct _CONTEXT ContextFrame;
    struct _KSPECIAL_REGISTERS SpecialRegisters;
} KPROCESSOR_STATE, *PKPROCESSOR_STATE;

#endif // _PPC_

#ifdef _X86_
//
// Special Registers for i386
//

typedef struct _DESCRIPTOR {
    WORD    Pad;
    WORD    Limit;
    DWORD   Base;
} KDESCRIPTOR, *PKDESCRIPTOR;

typedef struct _KSPECIAL_REGISTERS {
    DWORD Cr0;
    DWORD Cr2;
    DWORD Cr3;
    DWORD Cr4;
    DWORD KernelDr0;
    DWORD KernelDr1;
    DWORD KernelDr2;
    DWORD KernelDr3;
    DWORD KernelDr6;
    DWORD KernelDr7;
    KDESCRIPTOR Gdtr;
    KDESCRIPTOR Idtr;
    WORD   Tr;
    WORD   Ldtr;
    DWORD Reserved[6];
} KSPECIAL_REGISTERS, *PKSPECIAL_REGISTERS;

//
// Processor State frame: Before a processor freezes itself, it
// dumps the processor state to the processor state frame for
// debugger to examine.
//

typedef struct _KPROCESSOR_STATE {
    struct _CONTEXT ContextFrame;
    struct _KSPECIAL_REGISTERS SpecialRegisters;
} KPROCESSOR_STATE, *PKPROCESSOR_STATE;
#endif // _X86_


//
// The following are explicitly sized versions of common system
// structures which appear in the kernel debugger API.
//
// All of the debugger structures which are exposed to both
// sides of the KD API are declared below in explicitly sized
// versions as well, with inline converter functions.
//

//
// Macro for sign extending 32 bit addresses into 64 bits
//

#define COPYSE(p64,p32,f) p64->f = (ULONG64)(LONG64)(LONG)p32->f

__inline
void
ExceptionRecord32To64(
    IN PEXCEPTION_RECORD32 Ex32,
    OUT PEXCEPTION_RECORD64 Ex64
    )
{
    ULONG i;
    Ex64->ExceptionCode = Ex32->ExceptionCode;
    Ex64->ExceptionFlags = Ex32->ExceptionFlags;
    Ex64->ExceptionRecord = Ex32->ExceptionRecord;
    COPYSE(Ex64,Ex32,ExceptionAddress);
    Ex64->NumberParameters = Ex32->NumberParameters;
    for (i = 0; i < Ex64->NumberParameters; i++) {
        COPYSE(Ex64,Ex32,ExceptionInformation[i]);
    }
}

__inline
void
ExceptionRecord64To32(
    IN PEXCEPTION_RECORD64 Ex64,
    OUT PEXCEPTION_RECORD32 Ex32
    )
{
    ULONG i;
    Ex32->ExceptionCode = Ex64->ExceptionCode;
    Ex32->ExceptionFlags = Ex64->ExceptionFlags;
    Ex32->ExceptionRecord = (ULONG) Ex64->ExceptionRecord;
    Ex32->ExceptionAddress = (ULONG) Ex64->ExceptionAddress;
    Ex32->NumberParameters = Ex64->NumberParameters;
    for (i = 0; i < Ex32->NumberParameters; i++) {
        Ex32->ExceptionInformation[i] = (ULONG) Ex64->ExceptionInformation[i];
    }
}


typedef struct _DBGKM_EXCEPTION {
    EXCEPTION_RECORD ExceptionRecord;
    DWORD FirstChance;
} DBGKM_EXCEPTION, *PDBGKM_EXCEPTION;

typedef struct _DBGKM_EXCEPTION32 {
    EXCEPTION_RECORD32 ExceptionRecord;
    ULONG FirstChance;
} DBGKM_EXCEPTION32, *PDBGKM_EXCEPTION32;

typedef struct _DBGKM_EXCEPTION64 {
    EXCEPTION_RECORD64 ExceptionRecord;
    ULONG FirstChance;
} DBGKM_EXCEPTION64, *PDBGKM_EXCEPTION64;

__inline
void
DbgkmException32To64(
    IN PDBGKM_EXCEPTION32 E32,
    OUT PDBGKM_EXCEPTION64 E64
    )
{
    ExceptionRecord32To64(&E32->ExceptionRecord, &E64->ExceptionRecord);
    E64->FirstChance = E32->FirstChance;
}

__inline
void
DbgkmException64To32(
    IN PDBGKM_EXCEPTION64 E64,
    OUT PDBGKM_EXCEPTION32 E32
    )
{
    ExceptionRecord64To32(&E64->ExceptionRecord, &E32->ExceptionRecord);
    E32->FirstChance = E64->FirstChance;
}


//
// DbgKd APIs are for the portable kernel debugger
//

//
// KD_PACKETS are the low level data format used in KD. All packets
// begin with a packet leader, byte count, packet type. The sequence
// for accepting a packet is:
//
//  - read 4 bytes to get packet leader.  If read times out (10 seconds)
//    with a short read, or if packet leader is incorrect, then retry
//    the read.
//
//  - next read 2 byte packet type.  If read times out (10 seconds) with
//    a short read, or if packet type is bad, then start again looking
//    for a packet leader.
//
//  - next read 4 byte packet Id.  If read times out (10 seconds)
//    with a short read, or if packet Id is not what we expect, then
//    ask for resend and restart again looking for a packet leader.
//
//  - next read 2 byte byte count.  If read times out (10 seconds) with
//    a short read, or if byte count is greater than PACKET_MAX_SIZE,
//    then start again looking for a packet leader.
//
//  - next read 4 byte packet data checksum.
//
//  - The packet data immediately follows the packet.  There should be
//    ByteCount bytes following the packet header.  Read the packet
//    data, if read times out (10 seconds) then start again looking for
//    a packet leader.
//


typedef struct _KD_PACKET {
    DWORD PacketLeader;
    WORD   PacketType;
    WORD   ByteCount;
    DWORD PacketId;
    DWORD Checksum;
} KD_PACKET, *PKD_PACKET;


#define PACKET_MAX_SIZE 4000
#define INITIAL_PACKET_ID 0x80800000    // DON't use 0
#define SYNC_PACKET_ID    0x00000800    // Or in with INITIAL_PACKET_ID
                                        // to force a packet ID reset.

//
// BreakIn packet
//

#define BREAKIN_PACKET                  0x62626262
#define BREAKIN_PACKET_BYTE             0x62

//
// Packet lead in sequence
//

#define PACKET_LEADER                   0x30303030 //0x77000077
#define PACKET_LEADER_BYTE              0x30

#define CONTROL_PACKET_LEADER           0x69696969
#define CONTROL_PACKET_LEADER_BYTE      0x69

//
// Packet Trailing Byte
//

#define PACKET_TRAILING_BYTE            0xAA

//
// Packet Types
//

#define PACKET_TYPE_UNUSED              0
#define PACKET_TYPE_KD_STATE_CHANGE     1
#define PACKET_TYPE_KD_STATE_MANIPULATE 2
#define PACKET_TYPE_KD_DEBUG_IO         3
#define PACKET_TYPE_KD_ACKNOWLEDGE      4       // Packet-control type
#define PACKET_TYPE_KD_RESEND           5       // Packet-control type
#define PACKET_TYPE_KD_RESET            6       // Packet-control type
#define PACKET_TYPE_KD_STATE_CHANGE64   7
#define PACKET_TYPE_KD_POLL_BREAKIN     8
#define PACKET_TYPE_KD_TRACE_IO         9
#define PACKET_TYPE_KD_CONTROL_REQUEST  10      // 0x0a
#define PACKET_TYPE_KD_FILE_IO          11      // 0x0b
#define PACKET_TYPE_MAX                 12

//
// If the packet type is PACKET_TYPE_KD_STATE_CHANGE, then
// the format of the packet data is as follows:
//

#define DbgKdExceptionStateChange   0x00003030L
#define DbgKdLoadSymbolsStateChange 0x00003031L

#define KD_REBOOT    (-1)
#define KD_HIBERNATE (-2)

//
// If the packet type is PACKET_TYPE_KD_FILE_IO, then
// the format of the packet data is as follows:
//

#define DbgKdCreateFileApi      0x00003430L
#define DbgKdReadFileApi        0x00003431L
#define DbgKdWriteFileApi       0x00003432L
#define DbgKdCloseFileApi       0x00003433L

// Unicode filename follows as additional data.
typedef struct _DBGKD_CREATE_FILE {
    ULONG DesiredAccess;
    ULONG FileAttributes;
    ULONG ShareAccess;
    ULONG CreateDisposition;
    ULONG CreateOptions;
    // Return values.
    ULONG64 Handle;
    ULONG64 Length;
} DBGKD_CREATE_FILE, *PDBGKD_CREATE_FILE;

// Data is returned as additional data in the response.
typedef struct _DBGKD_READ_FILE {
    ULONG64 Handle;
    ULONG64 Offset;
    ULONG Length;
} DBGKD_READ_FILE, *PDBGKD_READ_FILE;

// Data is given as additional data.
typedef struct _DBGKD_WRITE_FILE {
    ULONG64 Handle;
    ULONG64 Offset;
    ULONG Length;
} DBGKD_WRITE_FILE, *PDBGKD_WRITE_FILE;

typedef struct _DBGKD_CLOSE_FILE {
    ULONG64 Handle;
} DBGKD_CLOSE_FILE, *PDBGKD_CLOSE_FILE;

typedef struct _DBGKD_FILE_IO {
    ULONG ApiNumber;
    ULONG Status;
    union {
        ULONG64 ReserveSpace[7];
        DBGKD_CREATE_FILE CreateFile;
        DBGKD_READ_FILE ReadFile;
        DBGKD_WRITE_FILE WriteFile;
        DBGKD_CLOSE_FILE CloseFile;
    } u;
} DBGKD_FILE_IO, *PDBGKD_FILE_IO;

#define CP_GET_SUCCESS  0
#define CP_GET_NODATA   1
#define CP_GET_ERROR    2


//
// Pathname Data follows directly
//

typedef struct _DBGKD_LOAD_SYMBOLS {
    DWORD PathNameLength;
    PVOID BaseOfDll;
    DWORD ProcessId;
    DWORD CheckSum;
    DWORD SizeOfImage;
    BOOLEAN UnloadSymbols;
} DBGKD_LOAD_SYMBOLS, *PDBGKD_LOAD_SYMBOLS;

#define DBGKD_LOAD_SYMBOLS32  DBGKD_LOAD_SYMBOLS
#define PDBGKD_LOAD_SYMBOLS32 PDBGKD_LOAD_SYMBOLS

typedef struct _DBGKD_LOAD_SYMBOLS64 {
    ULONG PathNameLength;
    ULONG64 BaseOfDll;
    ULONG64 ProcessId;
    ULONG CheckSum;
    ULONG SizeOfImage;
    BOOLEAN UnloadSymbols;
} DBGKD_LOAD_SYMBOLS64, *PDBGKD_LOAD_SYMBOLS64;

__inline
void
DbgkdLoadSymbols32To64(
    IN PDBGKD_LOAD_SYMBOLS32 Ls32,
    OUT PDBGKD_LOAD_SYMBOLS64 Ls64
    )
{
    Ls64->PathNameLength = Ls32->PathNameLength;
    Ls64->ProcessId = Ls32->ProcessId;
    COPYSE(Ls64,Ls32,BaseOfDll);
    Ls64->CheckSum = Ls32->CheckSum;
    Ls64->SizeOfImage = Ls32->SizeOfImage;
    Ls64->UnloadSymbols = Ls32->UnloadSymbols;
}

__inline
void
LoadSymbols64To32(
    IN PDBGKD_LOAD_SYMBOLS64 Ls64,
    OUT PDBGKD_LOAD_SYMBOLS32 Ls32
    )
{
    //printf("LoadSymbols64To32: len %x\n", Ls64->PathNameLength);
    Ls32->PathNameLength = Ls64->PathNameLength;
    Ls32->ProcessId = (ULONG)Ls64->ProcessId;
    Ls32->BaseOfDll = (PVOID)Ls64->BaseOfDll;
    Ls32->CheckSum = Ls64->CheckSum;
    Ls32->SizeOfImage = Ls64->SizeOfImage;
    Ls32->UnloadSymbols = Ls64->UnloadSymbols;
}

#define MAXIMUM_SUPPORTED_EXTENSION     512

#define CONTEXT_EXTENDED_REGISTERS  (CONTEXT_i386 | 0x00000020L) // cpu specific extensions

typedef struct _DBGKD_WAIT_STATE_CHANGE {
    DWORD NewState;
    WORD   ProcessorLevel;
    WORD   Processor;
    DWORD NumberProcessors;
    PVOID Thread;
    PVOID ProgramCounter;
    union {
        DBGKM_EXCEPTION Exception;
        DBGKD_LOAD_SYMBOLS LoadSymbols;
    } u;
    DBGKD_CONTROL_REPORT ControlReport;
    CONTEXT Context;
} DBGKD_WAIT_STATE_CHANGE, *PDBGKD_WAIT_STATE_CHANGE;

#define DBGKD_WAIT_STATE_CHANGE32  DBGKD_WAIT_STATE_CHANGE
#define PDBGKD_WAIT_STATE_CHANGE32 PDBGKD_WAIT_STATE_CHANGE

typedef struct _DBGKD_WAIT_STATE_CHANGE64 {
    ULONG NewState;
    USHORT ProcessorLevel;
    USHORT Processor;
    ULONG NumberProcessors;
    ULONG64 Thread;
    ULONG64 ProgramCounter;
    union {
        DBGKM_EXCEPTION64 Exception;
        DBGKD_LOAD_SYMBOLS64 LoadSymbols;
    } u;
    DBGKD_CONTROL_REPORT ControlReport;
    CONTEXT Context;
    //UCHAR   ExtendedRegisters[MAXIMUM_SUPPORTED_EXTENSION];
} DBGKD_WAIT_STATE_CHANGE64, *PDBGKD_WAIT_STATE_CHANGE64;

__inline
void
WaitStateChange32To64(
    IN PDBGKD_WAIT_STATE_CHANGE32 Ws32,
    OUT PDBGKD_WAIT_STATE_CHANGE64 Ws64
    )
{
    Ws64->NewState = Ws32->NewState;
    Ws64->ProcessorLevel = Ws32->ProcessorLevel;
    Ws64->Processor = Ws32->Processor;
    Ws64->NumberProcessors = Ws32->NumberProcessors;
    COPYSE(Ws64,Ws32,Thread);
    COPYSE(Ws64,Ws32,ProgramCounter);
    Ws64->ControlReport = Ws32->ControlReport;
    memcpy(&Ws64->Context, &Ws32->Context, sizeof(CONTEXT));
    if (Ws32->NewState == DbgKdLoadSymbolsStateChange) {
        DbgkdLoadSymbols32To64(&Ws32->u.LoadSymbols, &Ws64->u.LoadSymbols);
    } else {
        DbgkmException32To64((PDBGKM_EXCEPTION32)&Ws32->u.Exception, &Ws64->u.Exception);
    }
}

__inline
void
WaitStateChange64To32(
    IN PDBGKD_WAIT_STATE_CHANGE64 Ws64,
    OUT PDBGKD_WAIT_STATE_CHANGE Ws32
    )
{
    Ws32->NewState = Ws64->NewState;
    Ws32->ProcessorLevel = Ws64->ProcessorLevel;
    Ws32->Processor = Ws64->Processor;
    Ws32->NumberProcessors = Ws64->NumberProcessors;
    Ws32->Thread = (PVOID)Ws64->Thread;
    Ws32->ProgramCounter = (PVOID)Ws64->ProgramCounter;
    Ws32->ControlReport = Ws64->ControlReport;
    memcpy(&Ws32->Context, &Ws64->Context, sizeof(CONTEXT));
    if (Ws32->NewState == DbgKdLoadSymbolsStateChange) {
        LoadSymbols64To32(&Ws64->u.LoadSymbols, &Ws32->u.LoadSymbols);
    } else {
        DbgkmException64To32(&Ws64->u.Exception, (PDBGKM_EXCEPTION32)&Ws32->u.Exception);
    }
}



//
// If the packet type is PACKET_TYPE_KD_STATE_MANIPULATE, then
// the format of the packet data is as follows:
//
// Api Numbers for state manipulation
//

#define DbgKdReadVirtualMemoryApi           0x00003130L
#define DbgKdWriteVirtualMemoryApi          0x00003131L
#define DbgKdGetContextApi                  0x00003132L
#define DbgKdSetContextApi                  0x00003133L
#define DbgKdWriteBreakPointApi             0x00003134L
#define DbgKdRestoreBreakPointApi           0x00003135L
#define DbgKdContinueApi                    0x00003136L
#define DbgKdReadControlSpaceApi            0x00003137L
#define DbgKdWriteControlSpaceApi           0x00003138L
#define DbgKdReadIoSpaceApi                 0x00003139L
#define DbgKdWriteIoSpaceApi                0x0000313AL
#define DbgKdRebootApi                      0x0000313BL
#define DbgKdContinueApi2                   0x0000313CL
#define DbgKdReadPhysicalMemoryApi          0x0000313DL
#define DbgKdWritePhysicalMemoryApi         0x0000313EL
#define DbgKdQuerySpecialCallsApi           0x0000313FL
#define DbgKdSetSpecialCallApi              0x00003140L
#define DbgKdClearSpecialCallsApi           0x00003141L
#define DbgKdSetInternalBreakPointApi       0x00003142L
#define DbgKdGetInternalBreakPointApi       0x00003143L
#define DbgKdReadIoSpaceExtendedApi         0x00003144L
#define DbgKdWriteIoSpaceExtendedApi        0x00003145L
#define DbgKdGetVersionApi                  0x00003146L
#define DbgKdWriteBreakPointExApi           0x00003147L
#define DbgKdRestoreBreakPointExApi         0x00003148L
#define DbgKdCauseBugCheckApi               0x00003149L
#define DbgKdSwitchProcessor                0x00003150L
#define DbgKdPageInApi                      0x00003151L
#define DbgKdReadMachineSpecificRegister    0x00003152L
#define DbgKdWriteMachineSpecificRegister   0x00003153L
#define OldVlm1                             0x00003154L
#define OldVlm2                             0x00003155L
#define DbgKdSearchMemoryApi                0x00003156L
#define DbgKdGetBusDataApi                  0x00003157L
#define DbgKdSetBusDataApi                  0x00003158L
#define DbgKdCheckLowMemoryApi              0X00003159L

//
// Response is a read memory message with data following
//

typedef struct _DBGKD_READ_MEMORY {
    PVOID TargetBaseAddress;
    DWORD TransferCount;
    DWORD ActualBytesRead;
} DBGKD_READ_MEMORY, *PDBGKD_READ_MEMORY;

#define DBGKD_READ_MEMORY32  DBGKD_READ_MEMORY
#define PDBGKD_READ_MEMORY32 PDBGKD_READ_MEMORY

typedef struct _DBGKD_READ_MEMORY64 {
    ULONG64 TargetBaseAddress;
    ULONG TransferCount;
    ULONG ActualBytesRead;
} DBGKD_READ_MEMORY64, *PDBGKD_READ_MEMORY64;

__inline
void
DbgkdReadMemory32To64(
    IN PDBGKD_READ_MEMORY32 r32,
    OUT PDBGKD_READ_MEMORY64 r64
    )
{
    COPYSE(r64,r32,TargetBaseAddress);
    r64->TransferCount = r32->TransferCount;
    r64->ActualBytesRead = r32->ActualBytesRead;
}

__inline
void
DbgkdReadMemory64To32(
    IN PDBGKD_READ_MEMORY64 r64,
    OUT PDBGKD_READ_MEMORY32 r32
    )
{
    r32->TargetBaseAddress = (PVOID)r64->TargetBaseAddress;
    r32->TransferCount = r64->TransferCount;
    r32->ActualBytesRead = r64->ActualBytesRead;
}

//
// Data follows directly
//

typedef struct _DBGKD_WRITE_MEMORY {
    PVOID TargetBaseAddress;
    DWORD TransferCount;
    DWORD ActualBytesWritten;
} DBGKD_WRITE_MEMORY, *PDBGKD_WRITE_MEMORY;

#define DBGKD_WRITE_MEMORY32  DBGKD_WRITE_MEMORY
#define PDBGKD_WRITE_MEMORY32 PDBGKD_WRITE_MEMORY

typedef struct _DBGKD_WRITE_MEMORY64 {
    ULONG64 TargetBaseAddress;
    ULONG TransferCount;
    ULONG ActualBytesWritten;
} DBGKD_WRITE_MEMORY64, *PDBGKD_WRITE_MEMORY64;


__inline
void
DbgkdWriteMemory32To64(
    IN PDBGKD_WRITE_MEMORY32 r32,
    OUT PDBGKD_WRITE_MEMORY64 r64
    )
{
    COPYSE(r64,r32,TargetBaseAddress);
    r64->TransferCount = r32->TransferCount;
    r64->ActualBytesWritten = r32->ActualBytesWritten;
}

__inline
void
DbgkdWriteMemory64To32(
    IN PDBGKD_WRITE_MEMORY64 r64,
    OUT PDBGKD_WRITE_MEMORY32 r32
    )
{
    r32->TargetBaseAddress = (PVOID)r64->TargetBaseAddress;
    r32->TransferCount = r64->TransferCount;
    r32->ActualBytesWritten = r64->ActualBytesWritten;
}
//
// Response is a get context message with a full context record following
//

typedef struct _DBGKD_GET_CONTEXT {
    DWORD ContextFlags;
} DBGKD_GET_CONTEXT, *PDBGKD_GET_CONTEXT;

//
// Full Context record follows
//

typedef struct _DBGKD_SET_CONTEXT {
    DWORD ContextFlags;
} DBGKD_SET_CONTEXT, *PDBGKD_SET_CONTEXT;

#define BREAKPOINT_TABLE_SIZE   32      // max number supported by kernel

typedef struct _DBGKD_WRITE_BREAKPOINT {
    PVOID BreakPointAddress;
    DWORD BreakPointHandle;
} DBGKD_WRITE_BREAKPOINT, *PDBGKD_WRITE_BREAKPOINT;

#define DBGKD_WRITE_BREAKPOINT32  DBGKD_WRITE_BREAKPOINT
#define PDBGKD_WRITE_BREAKPOINT32 PDBGKD_WRITE_BREAKPOINT

typedef struct _DBGKD_WRITE_BREAKPOINT64 {
    ULONG64 BreakPointAddress;
    ULONG BreakPointHandle;
} DBGKD_WRITE_BREAKPOINT64, *PDBGKD_WRITE_BREAKPOINT64;


__inline
void
DbgkdWriteBreakpoint32To64(
    IN PDBGKD_WRITE_BREAKPOINT32 r32,
    OUT PDBGKD_WRITE_BREAKPOINT64 r64
    )
{
    COPYSE(r64,r32,BreakPointAddress);
    r64->BreakPointHandle = r32->BreakPointHandle;
}

__inline
void
DbgkdWriteBreakpoint64To32(
    IN PDBGKD_WRITE_BREAKPOINT64 r64,
    OUT PDBGKD_WRITE_BREAKPOINT32 r32
    )
{
    r32->BreakPointAddress = (PVOID)r64->BreakPointAddress;
    r32->BreakPointHandle = r64->BreakPointHandle;
}

typedef struct _DBGKD_RESTORE_BREAKPOINT {
    DWORD BreakPointHandle;
} DBGKD_RESTORE_BREAKPOINT, *PDBGKD_RESTORE_BREAKPOINT;

typedef struct _DBGKD_BREAKPOINTEX {
    DWORD     BreakPointCount;
    DWORD     ContinueStatus;
} DBGKD_BREAKPOINTEX, *PDBGKD_BREAKPOINTEX;

typedef struct _DBGKD_CONTINUE {
    DWORD    ContinueStatus;
} DBGKD_CONTINUE, *PDBGKD_CONTINUE;

typedef struct _DBGKD_CONTINUE2 {
    DWORD    ContinueStatus;
    DBGKD_CONTROL_SET ControlSet;
} DBGKD_CONTINUE2, *PDBGKD_CONTINUE2;

typedef struct _DBGKD_READ_WRITE_IO {
    DWORD DataSize;                     // 1, 2, 4
    PVOID IoAddress;
    DWORD DataValue;
} DBGKD_READ_WRITE_IO, *PDBGKD_READ_WRITE_IO;

#define DBGKD_READ_WRITE_IO32  DBGKD_READ_WRITE_IO
#define PDBGKD_READ_WRITE_IO32 PDBGKD_READ_WRITE_IO

typedef struct _DBGKD_READ_WRITE_IO64 {
    ULONG64 IoAddress;
    ULONG DataSize;                     // 1, 2, 4
    ULONG DataValue;
} DBGKD_READ_WRITE_IO64, *PDBGKD_READ_WRITE_IO64;

__inline
void
DbgkdReadWriteIo32To64(
    IN PDBGKD_READ_WRITE_IO32 r32,
    OUT PDBGKD_READ_WRITE_IO64 r64
    )
{
    COPYSE(r64,r32,IoAddress);
    r64->DataSize = r32->DataSize;
    r64->DataValue = r32->DataValue;
}

__inline
void
DbgkdReadWriteIo64To32(
    IN PDBGKD_READ_WRITE_IO64 r64,
    OUT PDBGKD_READ_WRITE_IO32 r32
    )
{
    r32->IoAddress = (PVOID)r64->IoAddress;
    r32->DataSize = r64->DataSize;
    r32->DataValue = r64->DataValue;
}

typedef struct _DBGKD_READ_WRITE_IO_EXTENDED {
    DWORD DataSize;                     // 1, 2, 4
    DWORD InterfaceType;
    DWORD BusNumber;
    DWORD AddressSpace;
    PVOID IoAddress;
    DWORD DataValue;
} DBGKD_READ_WRITE_IO_EXTENDED, *PDBGKD_READ_WRITE_IO_EXTENDED;

#define DBGKD_READ_WRITE_IO_EXTENDED32  DBGKD_READ_WRITE_IO_EXTENDED
#define PDBGKD_READ_WRITE_IO_EXTENDED32 PDBGKD_READ_WRITE_IO_EXTENDED

typedef struct _DBGKD_READ_WRITE_IO_EXTENDED64 {
    ULONG DataSize;                     // 1, 2, 4
    ULONG InterfaceType;
    ULONG BusNumber;
    ULONG AddressSpace;
    ULONG64 IoAddress;
    ULONG DataValue;
} DBGKD_READ_WRITE_IO_EXTENDED64, *PDBGKD_READ_WRITE_IO_EXTENDED64;

__inline
void
DbgkdReadWriteIoExtended32To64(
    IN PDBGKD_READ_WRITE_IO_EXTENDED32 r32,
    OUT PDBGKD_READ_WRITE_IO_EXTENDED64 r64
    )
{
    r64->DataSize = r32->DataSize;
    r64->InterfaceType = r32->InterfaceType;
    r64->BusNumber = r32->BusNumber;
    r64->AddressSpace = r32->AddressSpace;
    COPYSE(r64,r32,IoAddress);
    r64->DataValue = r32->DataValue;
}

__inline
void
DbgkdReadWriteIoExtended64To32(
    IN PDBGKD_READ_WRITE_IO_EXTENDED64 r64,
    OUT PDBGKD_READ_WRITE_IO_EXTENDED32 r32
    )
{
    r32->DataSize = r64->DataSize;
    r32->InterfaceType = r64->InterfaceType;
    r32->BusNumber = r64->BusNumber;
    r32->AddressSpace = r64->AddressSpace;
    r32->IoAddress = (PVOID)r64-> IoAddress;
    r32->DataValue = r64->DataValue;
}

typedef struct _DBGKD_READ_WRITE_MSR {
    DWORD Msr;
    DWORD DataValueLow;
    DWORD DataValueHigh;
} DBGKD_READ_WRITE_MSR, *PDBGKD_READ_WRITE_MSR;


typedef struct _DBGKD_QUERY_SPECIAL_CALLS {
    DWORD NumberOfSpecialCalls;
    // DWORD SpecialCalls[];
} DBGKD_QUERY_SPECIAL_CALLS, *PDBGKD_QUERY_SPECIAL_CALLS;

typedef struct _DBGKD_SET_SPECIAL_CALL {
    DWORD SpecialCall;
} DBGKD_SET_SPECIAL_CALL, *PDBGKD_SET_SPECIAL_CALL;

#define DBGKD_SET_SPECIAL_CALL32  DBGKD_SET_SPECIAL_CALL
#define PDBGKD_SET_SPECIAL_CALL32 PDBGKD_SET_SPECIAL_CALL

typedef struct _DBGKD_SET_SPECIAL_CALL64 {
    ULONG64 SpecialCall;
} DBGKD_SET_SPECIAL_CALL64, *PDBGKD_SET_SPECIAL_CALL64;

__inline
void
DbgkdSetSpecialCall64To32(
    IN PDBGKD_SET_SPECIAL_CALL64 r64,
    OUT PDBGKD_SET_SPECIAL_CALL32 r32
    )
{
    r32->SpecialCall = (ULONG)r64->SpecialCall;
}

typedef struct _DBGKD_SET_INTERNAL_BREAKPOINT {
    DWORD BreakpointAddress;
    DWORD Flags;
} DBGKD_SET_INTERNAL_BREAKPOINT, *PDBGKD_SET_INTERNAL_BREAKPOINT;

#define DBGKD_SET_INTERNAL_BREAKPOINT32  DBGKD_SET_INTERNAL_BREAKPOINT
#define PDBGKD_SET_INTERNAL_BREAKPOINT32 PDBGKD_SET_INTERNAL_BREAKPOINT

typedef struct _DBGKD_SET_INTERNAL_BREAKPOINT64 {
    ULONG64 BreakpointAddress;
    ULONG Flags;
} DBGKD_SET_INTERNAL_BREAKPOINT64, *PDBGKD_SET_INTERNAL_BREAKPOINT64;

__inline
void
DbgkdSetInternalBreakpoint64To32(
    IN PDBGKD_SET_INTERNAL_BREAKPOINT64 r64,
    OUT PDBGKD_SET_INTERNAL_BREAKPOINT32 r32
    )
{
    r32->BreakpointAddress = (ULONG)r64->BreakpointAddress;
    r32->Flags = r64->Flags;
}

typedef struct _DBGKD_GET_INTERNAL_BREAKPOINT {
    DWORD BreakpointAddress;
    DWORD Flags;
    DWORD Calls;
    DWORD MaxCallsPerPeriod;
    DWORD MinInstructions;
    DWORD MaxInstructions;
    DWORD TotalInstructions;
} DBGKD_GET_INTERNAL_BREAKPOINT, *PDBGKD_GET_INTERNAL_BREAKPOINT;

#define DBGKD_GET_INTERNAL_BREAKPOINT32  DBGKD_GET_INTERNAL_BREAKPOINT
#define PDBGKD_GET_INTERNAL_BREAKPOINT32 PDBGKD_GET_INTERNAL_BREAKPOINT

typedef struct _DBGKD_GET_INTERNAL_BREAKPOINT64 {
    ULONG64 BreakpointAddress;
    ULONG Flags;
    ULONG Calls;
    ULONG MaxCallsPerPeriod;
    ULONG MinInstructions;
    ULONG MaxInstructions;
    ULONG TotalInstructions;
} DBGKD_GET_INTERNAL_BREAKPOINT64, *PDBGKD_GET_INTERNAL_BREAKPOINT64;

__inline
void
DbgkdGetInternalBreakpoint32To64(
    IN PDBGKD_GET_INTERNAL_BREAKPOINT32 r32,
    OUT PDBGKD_GET_INTERNAL_BREAKPOINT64 r64
    )
{
    COPYSE(r64,r32,BreakpointAddress);
    r64->Flags = r32->Flags;
    r64->Calls = r32->Calls;
    r64->MaxCallsPerPeriod = r32->MaxCallsPerPeriod;
    r64->MinInstructions = r32->MinInstructions;
    r64->MaxInstructions = r32->MaxInstructions;
    r64->TotalInstructions = r32->TotalInstructions;
}

__inline
void
DbgkdGetInternalBreakpoint64To32(
    IN PDBGKD_GET_INTERNAL_BREAKPOINT64 r64,
    OUT PDBGKD_GET_INTERNAL_BREAKPOINT32 r32
    )
{
    r32->BreakpointAddress = (ULONG)r64->BreakpointAddress;
    r32->Flags = r64->Flags;
    r32->Calls = r64->Calls;
    r32->MaxCallsPerPeriod = r64->MaxCallsPerPeriod;
    r32->MinInstructions = r64->MinInstructions;
    r32->MaxInstructions = r64->MaxInstructions;
    r32->TotalInstructions = r64->TotalInstructions;
}

#define DBGKD_INTERNAL_BP_FLAG_COUNTONLY 0x00000001 // don't count instructions
#define DBGKD_INTERNAL_BP_FLAG_INVALID   0x00000002 // disabled BP
#define DBGKD_INTERNAL_BP_FLAG_SUSPENDED 0x00000004 // temporarily suspended
#define DBGKD_INTERNAL_BP_FLAG_DYING     0x00000008 // kill on exit


//
// The packet protocol was widened to 64 bits in version 5.
// The PTR64 flag allows the debugger to read the right
// size of pointer when neccessary.
//
// The version packet was changed in the same revision, to remove the
// data that are now available in KDDEBUGGER_DATA.
//
#define DBGKD_64BIT_PROTOCOL_VERSION 5


#ifndef DBGKD_GET_VERSION_DEFINED
#define DBGKD_GET_VERSION_DEFINED
//
// The following structures have changed in more than pointer size.
//
//
// This is the version packet for pre-NT5 Beta 2 systems.
// For now, it is also still used on x86
//
typedef struct _DBGKD_GET_VERSION {
    WORD    MajorVersion;
    WORD    MinorVersion;
    WORD    ProtocolVersion;
    WORD    Flags;
    DWORD   KernBase;
    DWORD   PsLoadedModuleList;
    WORD    MachineType;

    //
    // help for walking stacks with user callbacks:
    //

    //
    // The address of the thread structure is provided in the
    // WAIT_STATE_CHANGE packet.  This is the offset from the base of
    // the thread structure to the pointer to the kernel stack frame
    // for the currently active usermode callback.
    //

    WORD    ThCallbackStack;            // offset in thread data

    //
    // these values are offsets into that frame:
    //

    WORD    NextCallback;               // saved pointer to next callback frame
    WORD    FramePointer;               // saved frame pointer

    //
    // Address of the kernel callout routine.
    //

    DWORD   KiCallUserMode;             // kernel routine

    //
    // Address of the usermode entry point for callbacks.
    //

    DWORD   KeUserCallbackDispatcher;   // address in ntdll

    //
    // DbgBreakPointWithStatus is a function which takes an argument
    // and hits a breakpoint.  This field contains the address of the
    // breakpoint instruction.  When the debugger sees a breakpoint
    // at this address, it may retrieve the argument from the first
    // argument register, or on x86 the eax register.
    //

    DWORD   BreakpointWithStatus;       // address of breakpoint

    DWORD   DebuggerDataList;
} DBGKD_GET_VERSION, *PDBGKD_GET_VERSION;

#define DBGKD_GET_VERSION32  DBGKD_GET_VERSION
#define PDBGKD_GET_VERSION32 PDBGKD_GET_VERSION

typedef struct _DBGKD_GET_VERSION64 {
    USHORT  MajorVersion;
    USHORT  MinorVersion;
    USHORT  ProtocolVersion;
    USHORT  Flags;
    USHORT  MachineType;

    USHORT  Unused[3];

    ULONG64 KernBase;
    ULONG64 PsLoadedModuleList;

    //
    // Components may register a debug data block for use by
    // debugger extensions.  This is the address of the list head.
    //
    // There will always be an entry for the debugger.
    //

    ULONG64 DebuggerDataList;

} DBGKD_GET_VERSION64, *PDBGKD_GET_VERSION64;


//
// If DBGKD_VERS_FLAG_DATA is set in Flags, info should be retrieved from
// the KDDEBUGGER_DATA block rather than from the DBGKD_GET_VERSION
// packet.  The data will remain in the version packet for a while to
// reduce compatibility problems.
//

#define DBGKD_VERS_FLAG_MP      0x0001      // kernel is MP built
#define DBGKD_VERS_FLAG_DATA    0x0002      // DebuggerDataList is valid
#define DBGKD_VERS_FLAG_PTR64   0x0004      // native pointers are 64 bits
#define DBGKD_VERS_FLAG_NOMM    0x0008      // No MM - don't decode PTEs


typedef struct _DBGKD_PAGEIN {
    DWORD   Address;
    DWORD   ContinueStatus;
} DBGKD_PAGEIN, *PDBGKD_PAGEIN;

#define KDBG_TAG    'GBDK'

typedef struct _DBGKD_DEBUG_DATA_HEADER32 {

    //
    // Link to other blocks
    //

    LIST_ENTRY32 List;

    //
    // This is a unique tag to identify the owner of the block.
    // If your component only uses one pool tag, use it for this, too.
    //

    ULONG           OwnerTag;

    //
    // This must be initialized to the size of the data block,
    // including this structure.
    //

    ULONG           Size;

} DBGKD_DEBUG_DATA_HEADER32, *PDBGKD_DEBUG_DATA_HEADER32;




//
// DO NOT CHANGE THIS STRUCTURE!
// ONLY MAKE CHAGES TO THE 64 BIT VERSION ABOVE!!
//
// This is the debugger data packet for pre NT5 Beta 2 systems.
// For now, it is still used on x86
//
typedef struct _KDDEBUGGER_DATA32 {

    DBGKD_DEBUG_DATA_HEADER32 Header;

    //
    // Base address of kernel image
    //

    ULONG   KernBase;

    //
    // DbgBreakPointWithStatus is a function which takes an argument
    // and hits a breakpoint.  This field contains the address of the
    // breakpoint instruction.  When the debugger sees a breakpoint
    // at this address, it may retrieve the argument from the first
    // argument register, or on x86 the eax register.
    //

    ULONG   BreakpointWithStatus;       // address of breakpoint

    //
    // Address of the saved context record during a bugcheck
    //
    // N.B. This is an automatic in KeBugcheckEx's frame, and
    // is only valid after a bugcheck.
    //

    ULONG   SavedContext;

    //
    // help for walking stacks with user callbacks:
    //

    //
    // The address of the thread structure is provided in the
    // WAIT_STATE_CHANGE packet.  This is the offset from the base of
    // the thread structure to the pointer to the kernel stack frame
    // for the currently active usermode callback.
    //

    USHORT  ThCallbackStack;            // offset in thread data

    //
    // these values are offsets into that frame:
    //

    USHORT  NextCallback;               // saved pointer to next callback frame
    USHORT  FramePointer;               // saved frame pointer

    USHORT  PaeEnabled:1;

    //
    // Address of the kernel callout routine.
    //

    ULONG   KiCallUserMode;             // kernel routine

    //
    // Address of the usermode entry point for callbacks.
    //

    ULONG   KeUserCallbackDispatcher;   // address in ntdll


    //
    // Addresses of various kernel data structures and lists
    // that are of interest to the kernel debugger.
    //

    ULONG   PsLoadedModuleList;
    ULONG   PsActiveProcessHead;
    ULONG   PspCidTable;

    ULONG   ExpSystemResourcesList;
    ULONG   ExpPagedPoolDescriptor;
    ULONG   ExpNumberOfPagedPools;

    ULONG   KeTimeIncrement;
    ULONG   KeBugCheckCallbackListHead;
    ULONG   KiBugcheckData;

    ULONG   IopErrorLogListHead;

    ULONG   ObpRootDirectoryObject;
    ULONG   ObpTypeObjectType;

    ULONG   MmSystemCacheStart;
    ULONG   MmSystemCacheEnd;
    ULONG   MmSystemCacheWs;

    ULONG   MmPfnDatabase;
    ULONG   MmSystemPtesStart;
    ULONG   MmSystemPtesEnd;
    ULONG   MmSubsectionBase;
    ULONG   MmNumberOfPagingFiles;

    ULONG   MmLowestPhysicalPage;
    ULONG   MmHighestPhysicalPage;
    ULONG   MmNumberOfPhysicalPages;

    ULONG   MmMaximumNonPagedPoolInBytes;
    ULONG   MmNonPagedSystemStart;
    ULONG   MmNonPagedPoolStart;
    ULONG   MmNonPagedPoolEnd;

    ULONG   MmPagedPoolStart;
    ULONG   MmPagedPoolEnd;
    ULONG   MmPagedPoolInformation;
    ULONG   MmPageSize;

    ULONG   MmSizeOfPagedPoolInBytes;

    ULONG   MmTotalCommitLimit;
    ULONG   MmTotalCommittedPages;
    ULONG   MmSharedCommit;
    ULONG   MmDriverCommit;
    ULONG   MmProcessCommit;
    ULONG   MmPagedPoolCommit;
    ULONG   MmExtendedCommit;

    ULONG   MmZeroedPageListHead;
    ULONG   MmFreePageListHead;
    ULONG   MmStandbyPageListHead;
    ULONG   MmModifiedPageListHead;
    ULONG   MmModifiedNoWritePageListHead;
    ULONG   MmAvailablePages;
    ULONG   MmResidentAvailablePages;

    ULONG   PoolTrackTable;
    ULONG   NonPagedPoolDescriptor;

    ULONG   MmHighestUserAddress;
    ULONG   MmSystemRangeStart;
    ULONG   MmUserProbeAddress;

    ULONG   KdPrintCircularBuffer;
    ULONG   KdPrintCircularBufferEnd;
    ULONG   KdPrintWritePointer;
    ULONG   KdPrintRolloverCount;

    ULONG   MmLoadedUserImageList;
} KDDEBUGGER_DATA32, *PKDDEBUGGER_DATA32;

//
// DO NOT CHANGE KDDEBUGGER_DATA32!!
// ONLY MAKE CHANGES TO KDDEBUGGER_DATA64!!!
//




//
// This structure is used by the debugger for all targets
// It is the same size as DBGKD_DATA_HEADER on all systems
//
typedef struct _DBGKD_DEBUG_DATA_HEADER64 {

    //
    // Link to other blocks
    //

    LIST_ENTRY64 List;

    //
    // This is a unique tag to identify the owner of the block.
    // If your component only uses one pool tag, use it for this, too.
    //

    ULONG           OwnerTag;

    //
    // This must be initialized to the size of the data block,
    // including this structure.
    //

    ULONG           Size;

} DBGKD_DEBUG_DATA_HEADER64, *PDBGKD_DEBUG_DATA_HEADER64;


//
// This structure is the same size on all systems.  The only field
// which must be translated by the debugger is Header.List.
//

//
// DO NOT ADD OR REMOVE FIELDS FROM THE MIDDLE OF THIS STRUCTURE!!!
//
// If you remove a field, replace it with an "unused" placeholder.
// Do not reuse fields until there has been enough time for old debuggers
// and extensions to age out.
//
typedef struct _KDDEBUGGER_DATA64 {

    DBGKD_DEBUG_DATA_HEADER64 Header;

    //
    // Base address of kernel image
    //

    ULONG64   KernBase;

    //
    // DbgBreakPointWithStatus is a function which takes an argument
    // and hits a breakpoint.  This field contains the address of the
    // breakpoint instruction.  When the debugger sees a breakpoint
    // at this address, it may retrieve the argument from the first
    // argument register, or on x86 the eax register.
    //

    ULONG64   BreakpointWithStatus;       // address of breakpoint

    //
    // Address of the saved context record during a bugcheck
    //
    // N.B. This is an automatic in KeBugcheckEx's frame, and
    // is only valid after a bugcheck.
    //

    ULONG64   SavedContext;

    //
    // help for walking stacks with user callbacks:
    //

    //
    // The address of the thread structure is provided in the
    // WAIT_STATE_CHANGE packet.  This is the offset from the base of
    // the thread structure to the pointer to the kernel stack frame
    // for the currently active usermode callback.
    //

    USHORT  ThCallbackStack;            // offset in thread data

    //
    // these values are offsets into that frame:
    //

    USHORT  NextCallback;               // saved pointer to next callback frame
    USHORT  FramePointer;               // saved frame pointer

    //
    // pad to a quad boundary
    //
    USHORT  PaeEnabled:1;

    //
    // Address of the kernel callout routine.
    //

    ULONG64   KiCallUserMode;             // kernel routine

    //
    // Address of the usermode entry point for callbacks.
    //

    ULONG64   KeUserCallbackDispatcher;   // address in ntdll


    //
    // Addresses of various kernel data structures and lists
    // that are of interest to the kernel debugger.
    //

    ULONG64   PsLoadedModuleList;
    ULONG64   PsActiveProcessHead;
    ULONG64   PspCidTable;

    ULONG64   ExpSystemResourcesList;
    ULONG64   ExpPagedPoolDescriptor;
    ULONG64   ExpNumberOfPagedPools;

    ULONG64   KeTimeIncrement;
    ULONG64   KeBugCheckCallbackListHead;
    ULONG64   KiBugcheckData;

    ULONG64   IopErrorLogListHead;

    ULONG64   ObpRootDirectoryObject;
    ULONG64   ObpTypeObjectType;

    ULONG64   MmSystemCacheStart;
    ULONG64   MmSystemCacheEnd;
    ULONG64   MmSystemCacheWs;

    ULONG64   MmPfnDatabase;
    ULONG64   MmSystemPtesStart;
    ULONG64   MmSystemPtesEnd;
    ULONG64   MmSubsectionBase;
    ULONG64   MmNumberOfPagingFiles;

    ULONG64   MmLowestPhysicalPage;
    ULONG64   MmHighestPhysicalPage;
    ULONG64   MmNumberOfPhysicalPages;

    ULONG64   MmMaximumNonPagedPoolInBytes;
    ULONG64   MmNonPagedSystemStart;
    ULONG64   MmNonPagedPoolStart;
    ULONG64   MmNonPagedPoolEnd;

    ULONG64   MmPagedPoolStart;
    ULONG64   MmPagedPoolEnd;
    ULONG64   MmPagedPoolInformation;
    ULONG64   MmPageSize;

    ULONG64   MmSizeOfPagedPoolInBytes;

    ULONG64   MmTotalCommitLimit;
    ULONG64   MmTotalCommittedPages;
    ULONG64   MmSharedCommit;
    ULONG64   MmDriverCommit;
    ULONG64   MmProcessCommit;
    ULONG64   MmPagedPoolCommit;
    ULONG64   MmExtendedCommit;

    ULONG64   MmZeroedPageListHead;
    ULONG64   MmFreePageListHead;
    ULONG64   MmStandbyPageListHead;
    ULONG64   MmModifiedPageListHead;
    ULONG64   MmModifiedNoWritePageListHead;
    ULONG64   MmAvailablePages;
    ULONG64   MmResidentAvailablePages;

    ULONG64   PoolTrackTable;
    ULONG64   NonPagedPoolDescriptor;

    ULONG64   MmHighestUserAddress;
    ULONG64   MmSystemRangeStart;
    ULONG64   MmUserProbeAddress;

    ULONG64   KdPrintCircularBuffer;
    ULONG64   KdPrintCircularBufferEnd;
    ULONG64   KdPrintWritePointer;
    ULONG64   KdPrintRolloverCount;

    ULONG64   MmLoadedUserImageList;
} KDDEBUGGER_DATA64, *PKDDEBUGGER_DATA64;

#endif

__inline
void
DbgkdGetVersion32To64(
    IN PDBGKD_GET_VERSION32 vs32,
    OUT PDBGKD_GET_VERSION64 vs64,
    OUT PKDDEBUGGER_DATA64 dd64
    )
{
    vs64->MajorVersion = vs32->MajorVersion;
    vs64->MinorVersion = vs32->MinorVersion;
    vs64->ProtocolVersion = vs32->ProtocolVersion;
    vs64->Flags = vs32->Flags;
    vs64->MachineType = vs32->MachineType;
    COPYSE(vs64,vs32,PsLoadedModuleList);
    COPYSE(vs64,vs32,DebuggerDataList);
    COPYSE(vs64,vs32,KernBase);

    COPYSE(dd64,vs32,KernBase);
    COPYSE(dd64,vs32,PsLoadedModuleList);
    dd64->ThCallbackStack = vs32->ThCallbackStack;
    dd64->NextCallback = vs32->NextCallback;
    dd64->FramePointer = vs32->FramePointer;
    COPYSE(dd64,vs32,KiCallUserMode);
    COPYSE(dd64,vs32,KeUserCallbackDispatcher);
    COPYSE(dd64,vs32,BreakpointWithStatus);

}

__inline
void
DbgkdGetVersion64To32(
    IN PDBGKD_GET_VERSION64 vs64,
    IN PKDDEBUGGER_DATA64 dd64,
    OUT PDBGKD_GET_VERSION32 vs32
    )
{
    vs32->MajorVersion = vs64->MajorVersion;
    vs32->MinorVersion = vs64->MinorVersion;
    vs32->ProtocolVersion = vs64->ProtocolVersion;
    vs32->Flags = vs64->Flags;

    vs32->KernBase = (ULONG)vs64->KernBase;
    vs32->PsLoadedModuleList = (ULONG)vs64->PsLoadedModuleList;

    vs32->MachineType = vs64->MachineType;

    vs32->DebuggerDataList = (ULONG)vs64->DebuggerDataList;

    if(dd64) {
        vs32->ThCallbackStack = dd64->ThCallbackStack;
        vs32->NextCallback = dd64->NextCallback;
        vs32->FramePointer = dd64->FramePointer;

        vs32->KiCallUserMode = (ULONG)dd64->KiCallUserMode;
        vs32->KeUserCallbackDispatcher = (ULONG)dd64->KeUserCallbackDispatcher;
        vs32->BreakpointWithStatus = (ULONG)dd64->BreakpointWithStatus;
    }
}

__inline
void
DebuggerDataHeader32To64(
    IN  PDBGKD_DEBUG_DATA_HEADER32 Dd32,
    OUT PDBGKD_DEBUG_DATA_HEADER64 Dd64
    )
{
#define UIP(f) Dd64->f = (ULONG64)(LONG64)(LONG)Dd32->f
#define CP(f) Dd64->f = Dd32->f

    UIP(List.Flink);
    UIP(List.Blink);
    CP(OwnerTag);
    Dd64->Size = sizeof(KDDEBUGGER_DATA64);

#undef UIP
#undef CP
}

__inline
void
DebuggerDataHeader64To32(
    IN  PDBGKD_DEBUG_DATA_HEADER64 Dd64,
    OUT PDBGKD_DEBUG_DATA_HEADER32 Dd32
    )
{
#define UIP(f) Dd32->f = (ULONG)Dd64->f
#define CP(f) Dd32->f = Dd64->f

    UIP(List.Flink);
    UIP(List.Blink);
    CP(OwnerTag);
    Dd32->Size = sizeof(KDDEBUGGER_DATA32);

#undef UIP
#undef CP
}

__inline
void
DebuggerData32To64(
    IN PKDDEBUGGER_DATA32 Dd32,
    OUT PKDDEBUGGER_DATA64 Dd64
    )
{
#define UIP(f) Dd64->f = (ULONG64)(LONG64)(LONG)Dd32->f
#define CP(f) Dd64->f = Dd32->f

    DebuggerDataHeader32To64(&Dd32->Header, &Dd64->Header);

    UIP(KernBase);
    UIP(BreakpointWithStatus);
    UIP(SavedContext);
    CP(ThCallbackStack);
    CP(NextCallback);
    CP(FramePointer);
    CP(PaeEnabled);
    UIP(KiCallUserMode);
    UIP(KeUserCallbackDispatcher);
    UIP(PsLoadedModuleList);
    UIP(PsActiveProcessHead);
    UIP(PspCidTable);
    UIP(ExpSystemResourcesList);
    UIP(ExpPagedPoolDescriptor);
    UIP(ExpNumberOfPagedPools);
    UIP(KeTimeIncrement);
    UIP(KeBugCheckCallbackListHead);
    UIP(KiBugcheckData);
    UIP(IopErrorLogListHead);
    UIP(ObpRootDirectoryObject);
    UIP(ObpTypeObjectType);
    UIP(MmSystemCacheStart);
    UIP(MmSystemCacheEnd);
    UIP(MmSystemCacheWs);
    UIP(MmPfnDatabase);
    UIP(MmSystemPtesStart);
    UIP(MmSystemPtesEnd);
    UIP(MmSubsectionBase);
    UIP(MmNumberOfPagingFiles);
    UIP(MmLowestPhysicalPage);
    UIP(MmHighestPhysicalPage);
    UIP(MmNumberOfPhysicalPages);
    UIP(MmMaximumNonPagedPoolInBytes);
    UIP(MmNonPagedSystemStart);
    UIP(MmNonPagedPoolStart);
    UIP(MmNonPagedPoolEnd);
    UIP(MmPagedPoolStart);
    UIP(MmPagedPoolEnd);
    UIP(MmPagedPoolInformation);
    CP(MmPageSize);
    UIP(MmSizeOfPagedPoolInBytes);
    UIP(MmTotalCommitLimit);
    UIP(MmTotalCommittedPages);
    UIP(MmSharedCommit);
    UIP(MmDriverCommit);
    UIP(MmProcessCommit);
    UIP(MmPagedPoolCommit);
    UIP(MmExtendedCommit);
    UIP(MmZeroedPageListHead);
    UIP(MmFreePageListHead);
    UIP(MmStandbyPageListHead);
    UIP(MmModifiedPageListHead);
    UIP(MmModifiedNoWritePageListHead);
    UIP(MmAvailablePages);
    UIP(MmResidentAvailablePages);
    UIP(PoolTrackTable);
    UIP(NonPagedPoolDescriptor);
    UIP(MmHighestUserAddress);
    UIP(MmSystemRangeStart);
    UIP(MmUserProbeAddress);
    UIP(KdPrintCircularBuffer);
    UIP(KdPrintCircularBufferEnd);
    UIP(KdPrintWritePointer);
    UIP(KdPrintRolloverCount);
    UIP(MmLoadedUserImageList);

#undef UIP
#undef CP
}

__inline
void
DebuggerData64To32(
    IN PKDDEBUGGER_DATA64 Dd64,
    OUT PKDDEBUGGER_DATA32 Dd32
    )
{
#define UIP(f) Dd32->f = (ULONG)Dd64->f
#define CP(f) Dd32->f = Dd64->f

    DebuggerDataHeader64To32(&Dd64->Header, &Dd32->Header);

    UIP(KernBase);
    UIP(BreakpointWithStatus);
    UIP(SavedContext);
    CP(ThCallbackStack);
    CP(NextCallback);
    CP(FramePointer);
    CP(PaeEnabled);
    UIP(KiCallUserMode);
    UIP(KeUserCallbackDispatcher);
    UIP(PsLoadedModuleList);
    UIP(PsActiveProcessHead);
    UIP(PspCidTable);
    UIP(ExpSystemResourcesList);
    UIP(ExpPagedPoolDescriptor);
    UIP(ExpNumberOfPagedPools);
    UIP(KeTimeIncrement);
    UIP(KeBugCheckCallbackListHead);
    UIP(KiBugcheckData);
    UIP(IopErrorLogListHead);
    UIP(ObpRootDirectoryObject);
    UIP(ObpTypeObjectType);
    UIP(MmSystemCacheStart);
    UIP(MmSystemCacheEnd);
    UIP(MmSystemCacheWs);
    UIP(MmPfnDatabase);
    UIP(MmSystemPtesStart);
    UIP(MmSystemPtesEnd);
    UIP(MmSubsectionBase);
    UIP(MmNumberOfPagingFiles);
    UIP(MmLowestPhysicalPage);
    UIP(MmHighestPhysicalPage);
    UIP(MmNumberOfPhysicalPages);
    UIP(MmMaximumNonPagedPoolInBytes);
    UIP(MmNonPagedSystemStart);
    UIP(MmNonPagedPoolStart);
    UIP(MmNonPagedPoolEnd);
    UIP(MmPagedPoolStart);
    UIP(MmPagedPoolEnd);
    UIP(MmPagedPoolInformation);
    UIP(MmPageSize);
    UIP(MmSizeOfPagedPoolInBytes);
    UIP(MmTotalCommitLimit);
    UIP(MmTotalCommittedPages);
    UIP(MmSharedCommit);
    UIP(MmDriverCommit);
    UIP(MmProcessCommit);
    UIP(MmPagedPoolCommit);
    UIP(MmExtendedCommit);
    UIP(MmZeroedPageListHead);
    UIP(MmFreePageListHead);
    UIP(MmStandbyPageListHead);
    UIP(MmModifiedPageListHead);
    UIP(MmModifiedNoWritePageListHead);
    UIP(MmAvailablePages);
    UIP(MmResidentAvailablePages);
    UIP(PoolTrackTable);
    UIP(NonPagedPoolDescriptor);
    UIP(MmHighestUserAddress);
    UIP(MmSystemRangeStart);
    UIP(MmUserProbeAddress);
    UIP(KdPrintCircularBuffer);
    UIP(KdPrintCircularBufferEnd);
    UIP(KdPrintWritePointer);
    UIP(KdPrintRolloverCount);
    UIP(MmLoadedUserImageList);

#undef UIP
#undef CP
}

typedef struct _DBGKD_SEARCH_MEMORY {
    union {
        ULONG64 SearchAddress;
        ULONG64 FoundAddress;
    };
    ULONG64 SearchLength;
    ULONG PatternLength;
} DBGKD_SEARCH_MEMORY, *PDBGKD_SEARCH_MEMORY;


typedef struct _DBGKD_GET_SET_BUS_DATA {
    ULONG BusDataType;
    ULONG BusNumber;
    ULONG SlotNumber;
    ULONG Offset;
    ULONG Length;
} DBGKD_GET_SET_BUS_DATA, *PDBGKD_GET_SET_BUS_DATA;


#include <pshpack4.h>

typedef struct _DBGKD_MANIPULATE_STATE {
    DWORD ApiNumber;
    WORD   ProcessorLevel;
    WORD   Processor;
    DWORD    ReturnStatus;
    union {
        DBGKD_READ_MEMORY ReadMemory;
        DBGKD_WRITE_MEMORY WriteMemory;
        DBGKD_READ_MEMORY64 ReadMemory64;
        DBGKD_WRITE_MEMORY64 WriteMemory64;
        DBGKD_GET_CONTEXT GetContext;
        DBGKD_SET_CONTEXT SetContext;
        DBGKD_WRITE_BREAKPOINT WriteBreakPoint;
        DBGKD_RESTORE_BREAKPOINT RestoreBreakPoint;
        DBGKD_CONTINUE Continue;
        DBGKD_CONTINUE2 Continue2;
        DBGKD_READ_WRITE_IO ReadWriteIo;
        DBGKD_READ_WRITE_IO_EXTENDED ReadWriteIoExtended;
        DBGKD_QUERY_SPECIAL_CALLS QuerySpecialCalls;
        DBGKD_SET_SPECIAL_CALL SetSpecialCall;
        DBGKD_SET_INTERNAL_BREAKPOINT SetInternalBreakpoint;
        DBGKD_GET_INTERNAL_BREAKPOINT GetInternalBreakpoint;
        DBGKD_GET_VERSION GetVersion;
        DBGKD_BREAKPOINTEX BreakPointEx;
        DBGKD_PAGEIN PageIn;
        DBGKD_READ_WRITE_MSR ReadWriteMsr;
        DBGKD_SEARCH_MEMORY SearchMemory;
    } u;
} DBGKD_MANIPULATE_STATE, *PDBGKD_MANIPULATE_STATE;

#define DBGKD_MANIPULATE_STATE32  DBGKD_MANIPULATE_STATE
#define PDBGKD_MANIPULATE_STATE32 PDBGKD_MANIPULATE_STATE

#include <poppack.h>


typedef struct _DBGKD_MANIPULATE_STATE64 {
    ULONG ApiNumber;
    USHORT ProcessorLevel;
    USHORT Processor;
    NTSTATUS ReturnStatus;
    union {
        DBGKD_READ_MEMORY64 ReadMemory;
        DBGKD_WRITE_MEMORY64 WriteMemory;
        DBGKD_GET_CONTEXT GetContext;
        DBGKD_SET_CONTEXT SetContext;
        DBGKD_WRITE_BREAKPOINT64 WriteBreakPoint;
        DBGKD_RESTORE_BREAKPOINT RestoreBreakPoint;
        DBGKD_CONTINUE Continue;
        DBGKD_CONTINUE2 Continue2;
        DBGKD_READ_WRITE_IO64 ReadWriteIo;
        DBGKD_READ_WRITE_IO_EXTENDED64 ReadWriteIoExtended;
        DBGKD_QUERY_SPECIAL_CALLS QuerySpecialCalls;
        DBGKD_SET_SPECIAL_CALL64 SetSpecialCall;
        DBGKD_SET_INTERNAL_BREAKPOINT64 SetInternalBreakpoint;
        DBGKD_GET_INTERNAL_BREAKPOINT64 GetInternalBreakpoint;
        DBGKD_GET_VERSION64 GetVersion64;
        DBGKD_BREAKPOINTEX BreakPointEx;
        DBGKD_READ_WRITE_MSR ReadWriteMsr;
        DBGKD_SEARCH_MEMORY SearchMemory;
        DBGKD_GET_SET_BUS_DATA GetSetBusData;
    } u;
} DBGKD_MANIPULATE_STATE64, *PDBGKD_MANIPULATE_STATE64;

__inline
ULONG
DbgkdManipulateState32To64(
    IN PDBGKD_MANIPULATE_STATE32 r32,
    OUT PDBGKD_MANIPULATE_STATE64 r64,
    OUT PULONG AdditionalDataSize
    )
{
    r64->ApiNumber = r32->ApiNumber;
    r64->ProcessorLevel = r32->ProcessorLevel;
    r64->Processor = r32->Processor;
    r64->ReturnStatus = r32->ReturnStatus;

    *AdditionalDataSize = 0;

    //
    // translate the messages which may be sent by the kernel
    //

    switch (r64->ApiNumber) {

        case DbgKdSetContextApi:
        case DbgKdRestoreBreakPointApi:
        case DbgKdContinueApi:
        case DbgKdContinueApi2:
        case DbgKdRebootApi:
        case DbgKdClearSpecialCallsApi:
        case DbgKdRestoreBreakPointExApi:
        case DbgKdCauseBugCheckApi:
        case DbgKdSwitchProcessor:
        case DbgKdWriteMachineSpecificRegister:
        case DbgKdWriteIoSpaceApi:
        case DbgKdSetSpecialCallApi:
        case DbgKdSetInternalBreakPointApi:
        case DbgKdWriteIoSpaceExtendedApi:
            break;



        case DbgKdReadMachineSpecificRegister:
            r64->u.ReadWriteMsr = r32->u.ReadWriteMsr;
            break;

        //
        // GetVersion may need to be handled by the calling code;
        // it needs to call DbgkdGetVersion32To64 with the DebuggerDataBlock.
        //

        case DbgKdGetVersionApi:
            break;

        case DbgKdGetContextApi:
            *AdditionalDataSize = sizeof(CONTEXT);
            break;

        //case DbgKdQuerySpecialCallsApi:
        //    r64->u.QuerySpecialCalls = r32->u.QuerySpecialCalls;
        //    *AdditionalDataSize = r64->u.QuerySpecialCalls.NumberOfSpecialCalls * sizeof(ULONG);
        //    break;

        case DbgKdWriteBreakPointExApi:
            r64->u.BreakPointEx = r32->u.BreakPointEx;
            *AdditionalDataSize = r64->u.BreakPointEx.BreakPointCount * sizeof(ULONG);
            break;

        case DbgKdReadVirtualMemoryApi:
        case DbgKdReadPhysicalMemoryApi:
        case DbgKdReadControlSpaceApi:
            DbgkdReadMemory32To64(&r32->u.ReadMemory, &r64->u.ReadMemory);
            if (NT_SUCCESS(r32->ReturnStatus)) {
                *AdditionalDataSize = r64->u.ReadMemory.ActualBytesRead;
            }
            break;

        case DbgKdWriteVirtualMemoryApi:
        case DbgKdWritePhysicalMemoryApi:
        case DbgKdWriteControlSpaceApi:
            DbgkdWriteMemory32To64(&r32->u.WriteMemory, &r64->u.WriteMemory);
            break;



        case DbgKdWriteBreakPointApi:
            DbgkdWriteBreakpoint32To64(&r32->u.WriteBreakPoint, &r64->u.WriteBreakPoint);
            break;

        case DbgKdReadIoSpaceApi:
            DbgkdReadWriteIo32To64(&r32->u.ReadWriteIo, &r64->u.ReadWriteIo);
            break;

        case DbgKdReadIoSpaceExtendedApi:
            DbgkdReadWriteIoExtended32To64(&r32->u.ReadWriteIoExtended, &r64->u.ReadWriteIoExtended);
            break;

        case DbgKdGetInternalBreakPointApi:
            DbgkdGetInternalBreakpoint32To64(&r32->u.GetInternalBreakpoint, &r64->u.GetInternalBreakpoint);
            break;

        case DbgKdSearchMemoryApi:
            r64->u.SearchMemory = r32->u.SearchMemory;
            break;
    }

    return sizeof(DBGKD_MANIPULATE_STATE64);
}

__inline
ULONG
DbgkdManipulateState64To32(
    IN PDBGKD_MANIPULATE_STATE64 r64,
    OUT PDBGKD_MANIPULATE_STATE32 r32
    )
{
    r32->ApiNumber = r64->ApiNumber;
    r32->ProcessorLevel = r64->ProcessorLevel;
    r32->Processor = r64->Processor;
    r32->ReturnStatus = r64->ReturnStatus;

    //
    // translate the messages sent by the debugger
    //

    switch (r32->ApiNumber) {

        //
        // These send nothing in the u part.
        case DbgKdGetContextApi:
        case DbgKdSetContextApi:
        case DbgKdClearSpecialCallsApi:
        case DbgKdRebootApi:
        case DbgKdCauseBugCheckApi:
        case DbgKdSwitchProcessor:
            break;


        case DbgKdRestoreBreakPointApi:
            r32->u.RestoreBreakPoint = r64->u.RestoreBreakPoint;
            break;

        case DbgKdContinueApi:
            r32->u.Continue = r64->u.Continue;
            break;

        case DbgKdContinueApi2:
            r32->u.Continue2 = r64->u.Continue2;
            break;

        //case DbgKdQuerySpecialCallsApi:
        //    r32->u.QuerySpecialCalls = r64->u.QuerySpecialCalls;
        //    break;

        case DbgKdRestoreBreakPointExApi:
            // NYI
            break;

        case DbgKdReadMachineSpecificRegister:
        case DbgKdWriteMachineSpecificRegister:
            r32->u.ReadWriteMsr = r64->u.ReadWriteMsr;
            break;

        case DbgKdGetVersionApi:
            //r32->u.GetVersion.ProtocolVersion = r64->u.GetVersion64.ProtocolVersion;
            DbgkdGetVersion64To32(&(r64->u.GetVersion64), NULL, &(r32->u.GetVersion));
            break;

        case DbgKdWriteBreakPointExApi:
            r32->u.BreakPointEx = r64->u.BreakPointEx;
            break;

        case DbgKdWriteVirtualMemoryApi:
            DbgkdWriteMemory64To32(&r64->u.WriteMemory, &r32->u.WriteMemory);
            break;

        //
        // 32 bit systems only support 32 bit physical r/w
        //
        case DbgKdReadControlSpaceApi:
        case DbgKdReadVirtualMemoryApi:
        case DbgKdReadPhysicalMemoryApi:
            DbgkdReadMemory64To32(&r64->u.ReadMemory, &r32->u.ReadMemory);
            break;

        case DbgKdWritePhysicalMemoryApi:
            DbgkdWriteMemory64To32(&r64->u.WriteMemory, &r32->u.WriteMemory);
            break;

        case DbgKdWriteBreakPointApi:
            DbgkdWriteBreakpoint64To32(&r64->u.WriteBreakPoint, &r32->u.WriteBreakPoint);
            break;

        case DbgKdWriteControlSpaceApi:
            DbgkdWriteMemory64To32(&r64->u.WriteMemory, &r32->u.WriteMemory);
            break;

        case DbgKdReadIoSpaceApi:
        case DbgKdWriteIoSpaceApi:
            DbgkdReadWriteIo64To32(&r64->u.ReadWriteIo, &r32->u.ReadWriteIo);
            break;

        case DbgKdSetSpecialCallApi:
            DbgkdSetSpecialCall64To32(&r64->u.SetSpecialCall, &r32->u.SetSpecialCall);
            break;

        case DbgKdSetInternalBreakPointApi:
            DbgkdSetInternalBreakpoint64To32(&r64->u.SetInternalBreakpoint, &r32->u.SetInternalBreakpoint);
            break;

        case DbgKdGetInternalBreakPointApi:
            DbgkdGetInternalBreakpoint64To32(&r64->u.GetInternalBreakpoint, &r32->u.GetInternalBreakpoint);
            break;

        case DbgKdReadIoSpaceExtendedApi:
        case DbgKdWriteIoSpaceExtendedApi:
            DbgkdReadWriteIoExtended64To32(&r64->u.ReadWriteIoExtended, &r32->u.ReadWriteIoExtended);
            break;

        case DbgKdSearchMemoryApi:
            r32->u.SearchMemory = r64->u.SearchMemory;
            break;
    }

    return sizeof(DBGKD_MANIPULATE_STATE32);
}

//
// This is the format for the trace data passed back from the kernel to
// the debugger to describe multiple calls that have returned since the
// last trip back.  The basic format is that there are a bunch of these
// (4 byte) unions stuck together.  Each union is of one of two types: a
// 4 byte unsigned long interger, or a three field struct, describing a
// call (where "call" is delimited by returning or exiting the symbol
// scope).  If the number of instructions executed is too big to fit
// into a WORD   -1, then the Instructions field has
// TRACE_DATA_INSTRUCTIONS_BIG and the next union is a LongNumber
// containing the real number of instructions executed.
//
// The very first union returned in each callback is a LongNumber
// containing the number of unions returned (including the "size"
// record, os it's always at least 1 even if there's no data to return).
//
// This is all returned to the debugger when one of two things
// happens:
//
//   1) The pc moves out of all defined symbol ranges
//   2) The buffer of trace data entries is filled.
//
// The "trace done" case is hacked around on the debugger side.  It
// guarantees that the pc address that indicates a trace exit never
// winds up in a defined symbol range.
//
// The only other complexity in this system is handling the SymbolNumber
// table.  This table is kept in parallel by the kernel and the
// debugger.  When the PC exits a known symbol range, the Begin and End
// symbol ranges are set by the debugger and are allocated to the next
// symbol slot upon return.  "The next symbol slot" means the numerical
// next slot number, unless we've filled all slots, in which case it is
// #0.  (ie., allocation is cyclic and not LRU or something).  The
// SymbolNumber table is flushed when a SpecialCalls call is made (ie.,
// at the beginning of the WatchTrace).
//

typedef union _DBGKD_TRACE_DATA {
    struct {
        BYTE  SymbolNumber;
        CHAR LevelChange;
        WORD   Instructions;
    } s;
    DWORD LongNumber;
} DBGKD_TRACE_DATA, *PDBGKD_TRACE_DATA;

#define TRACE_DATA_INSTRUCTIONS_BIG 0xffff

#define TRACE_DATA_BUFFER_MAX_SIZE 40

//
// If the packet type is PACKET_TYPE_KD_DEBUG_IO, then
// the format of the packet data is as follows:
//

#define DbgKdPrintStringApi     0x00003230L
#define DbgKdGetStringApi       0x00003231L

//
// For print string, the Null terminated string to print
// immediately follows the message
//
typedef struct _DBGKD_PRINT_STRING {
    DWORD LengthOfString;
} DBGKD_PRINT_STRING, *PDBGKD_PRINT_STRING;

//
// For get string, the Null terminated promt string
// immediately follows the message. The LengthOfStringRead
// field initially contains the maximum number of characters
// to read. Upon reply, this contains the number of bytes actually
// read. The data read immediately follows the message.
//
//
typedef struct _DBGKD_GET_STRING {
    DWORD LengthOfPromptString;
    DWORD LengthOfStringRead;
} DBGKD_GET_STRING, *PDBGKD_GET_STRING;

typedef struct _DBGKD_DEBUG_IO {
    DWORD ApiNumber;
    WORD   ProcessorLevel;
    WORD   Processor;
    union {
        DBGKD_PRINT_STRING PrintString;
        DBGKD_GET_STRING GetString;
    } u;
} DBGKD_DEBUG_IO, *PDBGKD_DEBUG_IO;

VOID
NTAPI
DbgKdSendBreakIn(
    VOID
    );

PBYTE 
NTAPI
DbgKdGets(
    PBYTE  Buffer,
    WORD   Length
    );

DWORD   
NTAPI
DbgKdWaitStateChange(
    OUT PDBGKD_WAIT_STATE_CHANGE StateChange,
    OUT PVOID Buffer,
    IN DWORD BufferLength
    );

DWORD   
NTAPI
DbgKdContinue (
    IN DWORD    ContinueStatus
    );

DWORD   
NTAPI
DbgKdContinue2 (
    IN DWORD    ContinueStatus,
    IN DBGKD_CONTROL_SET ControlSet
    );

DWORD   
NTAPI
DbgKdReadVirtualMemory(
    IN PVOID TargetBaseAddress,
    OUT PVOID UserInterfaceBuffer,
    IN DWORD TransferCount,
    OUT PDWORD ActualBytesRead OPTIONAL
    );

DWORD   
NTAPI
DbgKdWriteVirtualMemory(
    IN PVOID TargetBaseAddress,
    OUT PVOID UserInterfaceBuffer,
    IN DWORD TransferCount,
    OUT PDWORD ActualBytesWritten OPTIONAL
    );

DWORD   
NTAPI
DbgKdReadPhysicalMemory(
    IN PHYSICAL_ADDRESS TargetBaseAddress,
    OUT PVOID UserInterfaceBuffer,
    IN DWORD TransferCount,
    OUT PDWORD ActualBytesRead OPTIONAL
    );

DWORD   
NTAPI
DbgKdWritePhysicalMemory(
    IN PHYSICAL_ADDRESS TargetBaseAddress,
    OUT PVOID UserInterfaceBuffer,
    IN DWORD TransferCount,
    OUT PDWORD ActualBytesWritten OPTIONAL
    );

DWORD   
NTAPI
DbgKdReadControlSpace(
    IN WORD   Processor,
    IN PVOID TargetBaseAddress,
    OUT PVOID UserInterfaceBuffer,
    IN DWORD TransferCount,
    OUT PDWORD ActualBytesRead OPTIONAL
    );

DWORD   
NTAPI
DbgKdWriteControlSpace(
    IN WORD   Processor,
    IN PVOID TargetBaseAddress,
    OUT PVOID UserInterfaceBuffer,
    IN DWORD TransferCount,
    OUT PDWORD ActualBytesWritten OPTIONAL
    );

DWORD   
NTAPI
DbgKdReadIoSpace(
    IN PVOID IoAddress,
    OUT PVOID ReturnedData,
    IN DWORD DataSize
    );

DWORD   
NTAPI
DbgKdWriteIoSpace(
    IN PVOID IoAddress,
    IN DWORD DataValue,
    IN DWORD DataSize
    );

DWORD   
NTAPI
DbgKdReadMsr(
    IN DWORD MsrReg,
    OUT PDWORDLONG MsrValue
    );

DWORD   
NTAPI
DbgKdWriteMsr(
    IN DWORD MsrReg,
    IN DWORDLONG MsrValue
    );


DWORD   
NTAPI
DbgKdGetContext(
    IN WORD   Processor,
    IN OUT PCONTEXT Context
    );

DWORD   
NTAPI
DbgKdSetContext(
    IN WORD   Processor,
    IN PCONTEXT Context
    );

DWORD   
NTAPI
DbgKdWriteBreakPoint(
    IN PVOID BreakPointAddress,
    OUT PDWORD BreakPointHandle
    );

DWORD   
NTAPI
DbgKdRestoreBreakPoint(
    IN DWORD BreakPointHandle
    );

DWORD   
NTAPI
DbgKdReboot(
    VOID
    );

#ifdef _X86_
DWORD   
NTAPI
DbgKdLookupSelector(
    IN WORD   Processor,
    IN OUT PDESCRIPTOR_TABLE_ENTRY pDescriptorTableEntry
    );
#endif

DWORD
NTAPI
DbgKdGetVersion (
    OUT PDBGKD_GET_VERSION VersionBuffer
    );

#define DBG_EXCEPTION_HANDLED           ((DWORD   )0x00010001L) 

#endif // _WINDBGKD_

