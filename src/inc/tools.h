/*
Module Name:

    tools.h

Abstract:

    This header contains some useful definitions for data manipulation.

Environment:

    kernel mode only
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef __TOOLS_H__
#define __TOOLS_H__

#ifdef _DEBUG
#ifndef DBG
#define DBG
#endif //DBG
#endif //_DEBUG

extern "C" {

//----------------

#ifndef USER_MODE
#include <ntddk.h>                  // various NT definitions
#endif USER_MODE

//----------------

#ifndef FOUR_BYTE_DEFINED
#define FOUR_BYTE_DEFINED
typedef struct _FOUR_BYTE {
    UCHAR Byte0;
    UCHAR Byte1;
    UCHAR Byte2;
    UCHAR Byte3;
} FOUR_BYTE, *PFOUR_BYTE;
#endif //FOUR_BYTE_DEFINED


// This macro has the effect of Bit = log2(Data)

#define WHICH_BIT(Data, Bit) {                      \
    for (Bit = 0; Bit < 32; Bit++) {                \
        if ((Data >> Bit) == 1) {                   \
            break;                                  \
        }                                           \
    }                                               \
}

#define ntohs(x)    ( (((USHORT)x[0])<<8) | x[1]  )
#define DEC_TO_BCD(x) (((x / 10) << 4) + (x % 10))

#define MOV_3B_SWP(a,b)     MOV_MSF_SWP(a,b)

#ifdef DBG
#define KdDump(a,b)                         \
if((a)!=NULL) {                             \
    ULONG i;                                \
    for(i=0; i<(b); i++) {                  \
        ULONG c;                            \
        c = (ULONG)(*(((PUCHAR)(a))+i));    \
        KdPrint(("%2.2x ",c));              \
        if ((i & 0x0f) == 0x0f) KdPrint(("\n"));   \
    }                                       \
    KdPrint(("\n"));                        \
}
#else

#define KdDump(a,b)
#undef KdPrint
#define KdPrint(_x_)

#endif //DBG

#define CHECK_OUT_MDL()                                                 \
if(irpStack->Parameters.DeviceIoControl.OutputBufferLength < length) {  \
    KdPrint(("Output buffer (MDL) too small (%x/%x)\n", length, \
                    irpStack->Parameters.DeviceIoControl.OutputBufferLength));   \
    Irp->IoStatus.Information = 0;                                      \
    status = STATUS_BUFFER_TOO_SMALL;                                   \
    goto DbgPrnHkSpecDevCtrlCompleteRequest;                                \
}                                                                       \
if ( Irp->MdlAddress == NULL ){                                         \
    Irp->IoStatus.Information = 0;                                      \
    status = STATUS_NOT_MAPPED_DATA;                                    \
    goto DbgPrnHkSpecDevCtrlCompleteRequest;                                \
}

#define CHECK_IN_MDL()                                                  \
if(irpStack->Parameters.DeviceIoControl.InputBufferLength < length) {   \
    KdPrint(("Input buffer (MDL) too small (%x/%x)\n", length, \
                    irpStack->Parameters.DeviceIoControl.InputBufferLength));   \
    Irp->IoStatus.Information = 0;                                      \
    status = STATUS_INFO_LENGTH_MISMATCH;                               \
    goto DbgPrnHkSpecDevCtrlCompleteRequest;                                \
}                                                                       \
if ( Irp->MdlAddress == NULL ){                                         \
    Irp->IoStatus.Information = 0;                                      \
    status = STATUS_NOT_MAPPED_DATA;                                    \
    goto DbgPrnHkSpecDevCtrlCompleteRequest;                                \
}

#define CHECK_OUT_BUFFER()                                              \
if(irpStack->Parameters.DeviceIoControl.OutputBufferLength < length) {  \
    KdPrint(("Output buffer too small (%x/%x)\n", length, \
                    irpStack->Parameters.DeviceIoControl.OutputBufferLength));   \
    Irp->IoStatus.Information = 0;                                      \
    status = STATUS_BUFFER_TOO_SMALL;                                   \
    goto DbgPrnHkSpecDevCtrlCompleteRequest;                                \
}                                                                       \
if ( Irp->AssociatedIrp.SystemBuffer == NULL ){                                         \
    Irp->IoStatus.Information = 0;                                      \
    status = STATUS_NOT_MAPPED_DATA;                                    \
    goto DbgPrnHkSpecDevCtrlCompleteRequest;                                \
}

#define CHECK_IN_BUFFER()                                              \
if(irpStack->Parameters.DeviceIoControl.InputBufferLength < length) {  \
    KdPrint(("Input buffer too small (%x/%x)\n", length, \
                    irpStack->Parameters.DeviceIoControl.InputBufferLength));   \
    Irp->IoStatus.Information = 0;                                      \
    status = STATUS_INFO_LENGTH_MISMATCH;                               \
    goto DbgPrnHkSpecDevCtrlCompleteRequest;                                \
}                                                                       \
if ( Irp->AssociatedIrp.SystemBuffer == NULL ){                                         \
    Irp->IoStatus.Information = 0;                                      \
    status = STATUS_NOT_MAPPED_DATA;                                    \
    goto DbgPrnHkSpecDevCtrlCompleteRequest;                                \
}

};

#ifndef min
#define min(a,b)  (((a)>(b)) ? (b) : (a))
#endif

#ifndef offsetof
#define offsetof(type, field)   (ULONG)&(((type *)0)->field)
#endif //offsetof

BOOLEAN
__fastcall
EnvGetBit(
    IN PULONG arr,
    IN ULONG  bit
    );

BOOLEAN
__fastcall
EnvSetBit(
    IN PULONG arr,
    IN ULONG  bit
    );

#endif // __TOOLS_H__

