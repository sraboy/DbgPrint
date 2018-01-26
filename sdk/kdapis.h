#ifndef __KDAPIS_H__
#define __KDAPIS_H__

extern "C" {

#pragma pack(push, 8)

#include <Ntsecapi.h>
#include "WINDBGKD.H"

#define ERROR_INTERRUPTED	95

#ifndef ERROR_TIMEOUT
#define ERROR_TIMEOUT           640
#endif

#define HLDSIG_ENABLE		0
#define HLDSIG_DISABLE		1

extern UCHAR DbgKdpPacketLeader[4];
extern HANDLE ConsoleInputHandle;
//extern HANDLE ConsoleOutputHandle;

extern UCHAR DbgKdpPacket[];
extern KD_PACKET PacketHeader;
extern UCHAR chLastCommand[256];   //  last command executed

extern BOOLEAN DbgKdpUse64bit;

VOID
DbgKdpWritePacket(
    IN PVOID PacketData,
    IN USHORT PacketDataLength,
    IN USHORT PacketType,
    IN PVOID MorePacketData OPTIONAL,
    IN USHORT MorePacketDataLength OPTIONAL
    );

BOOLEAN
DbgKdpWaitForPacket(
    IN USHORT PacketType,
    OUT PVOID Packet
    );

VOID
DbgKdpHandlePromptString(
    IN PDBGKD_DEBUG_IO IoMessage
    );

typedef VOID (*ptrDbgKdpPrint)(
    IN USHORT Processor,
    IN USHORT ProcessorLevel,
    IN PUCHAR String,
    IN USHORT StringLength
    );

extern ptrDbgKdpPrint DbgKdpPrint;


typedef BOOL (*ptrDbgKdpGetConsoleByte)(
    PVOID pBuf,
    DWORD cbBuf,
    LPDWORD pcbBytesRead
    );

extern ptrDbgKdpGetConsoleByte DbgKdpGetConsoleByte;

//
// Global Data
//
extern HANDLE DbgKdpComPort;

//
// This overlapped structure will be used for all serial read
// operations. We only need one structure since the code is
// designed so that no more than one serial read operation is
// outstanding at any one time.
//
extern OVERLAPPED ReadOverlapped;

//
// This overlapped structure will be used for all serial write
// operations. We only need one structure since the code is
// designed so that no more than one serial write operation is
// outstanding at any one time.
//
extern OVERLAPPED WriteOverlapped;

//
// APIs
//

BOOLEAN
DbgKdpStartThreads(VOID);

ULONG
DbgKdpKbdPollThread(VOID);


HANDLE
DbgKdpInitComPort(
    ULONG ComPort,
    ULONG Baud
    );

VOID
DbgKdpAsyncControl(
   IN OUT PVOID Data,
   IN OUT PVOID Parm,
   IN USHORT Function
   );

DWORD
DbgKdConnectAndInitialize(
    ULONG ComPort,
    ULONG Baud
   );

#pragma pack(pop)

};

#endif // __KDAPIS_H__
