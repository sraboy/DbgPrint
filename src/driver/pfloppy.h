#ifndef __DBGFLOPPY_POLL_MODE__H__
#define __DBGFLOPPY_POLL_MODE__H__

extern "C" {

#include <ntddk.h>
#include <ntdddisk.h>

};
#include "stddef.h"

#define READ_CONTROLLER( Address )                         \
    READ_PORT_UCHAR( (PUCHAR)(Address) )

#define WRITE_CONTROLLER( Address, Value )                 \
    WRITE_PORT_UCHAR( (PUCHAR)(Address), (UCHAR)(Value) )

typedef struct _FLOPPY_PORT_LAYOUT {
    UCHAR Reserved0; // PS/2, Intel only    3f0
    UCHAR MediaId;   // PS/2, Intel only    3f1
    UCHAR DriveControl;
    UCHAR Reserved1;
    UCHAR Status;
    UCHAR Fifo;
    UCHAR Reserved2;
    union {
        UCHAR DataRate;
        UCHAR DiskChange;
    } DRDC;
} FLOPPY_PORT_LAYOUT, *PFLOPPY_PORT_LAYOUT;

//
// Floppy commands.                                Optional bits allowed.
//

#define COMMND_READ_DATA                   0x06    // Multi-Track, MFM, Skip
#define COMMND_READ_DELETED_DATA           0x0C    // Multi-Track, MFM, Skip
#define COMMND_READ_TRACK                  0x02    // MFM
#define COMMND_WRITE_DATA                  0x05    // Multi-Track, MFM
#define COMMND_WRITE_DELETED_DATA          0x09    // Multi-Track, MFM
#define COMMND_READ_ID                     0x0A    // MFM
#define COMMND_FORMAT_TRACK                0x0D    // MFM
#define COMMND_RECALIBRATE                 0x07
#define COMMND_SENSE_INTERRUPT             0x08
#define COMMND_SPECIFY                     0x03
#define COMMND_SENSE_DRIVE                 0x04
#define COMMND_SEEK                        0x0F
#define COMMND_PERPENDICULAR_MODE          0x12
#define COMMND_CONFIGURE                   0x13

//
// Optional bits used with the commands.
//

#define COMMND_MULTI_TRACK                 0x80
#define COMMND_MFM                         0x40
#define COMMND_SKIP                        0x20


//
// Bits in the DRIVE_CONTROL register.
//

#define DRVCTL_RESET                       0x00
#define DRVCTL_ENABLE_CONTROLLER           0x04
#define DRVCTL_ENABLE_DMA_AND_INTERRUPTS   0x08
#define DRVCTL_DRIVE_0                     0x10
#define DRVCTL_DRIVE_1                     0x21
#define DRVCTL_DRIVE_2                     0x42
#define DRVCTL_DRIVE_3                     0x83
#define DRVCTL_DRIVE_MASK                  0x03
#define DRVCTL_MOTOR_MASK                  0xf0

//
// Bits in the STATUS register.
//

#define STATUS_DRIVE_0_BUSY                0x01
#define STATUS_DRIVE_1_BUSY                0x02
#define STATUS_DRIVE_2_BUSY                0x04
#define STATUS_DRIVE_3_BUSY                0x08
#define STATUS_CONTROLLER_BUSY             0x10
#define STATUS_DMA_UNUSED                  0x20
#define STATUS_DIRECTION_READ              0x40
#define STATUS_DATA_REQUEST                0x80

#define STATUS_IO_READY_MASK               0xc0
#define STATUS_READ_READY                  0xc0
#define STATUS_WRITE_READY                 0x80

//
// Bits in the DATA_RATE register.
//

#define DATART_0125                        0x03
#define DATART_0250                        0x02
#define DATART_0300                        0x01
#define DATART_0500                        0x00
#define DATART_1000                        0x03
#define DATART_RESERVED                    0xfc

//
// Bits in the DISK_CHANGE register.
//

#define DSKCHG_RESERVED                    0x7f
#define DSKCHG_DISKETTE_REMOVED            0x80

//
// Bits in status register 0.
//

#define STREG0_DRIVE_0                     0x00
#define STREG0_DRIVE_1                     0x01
#define STREG0_DRIVE_2                     0x02
#define STREG0_DRIVE_3                     0x03
#define STREG0_HEAD                        0x04
#define STREG0_DRIVE_NOT_READY             0x08
#define STREG0_DRIVE_FAULT                 0x10
#define STREG0_SEEK_COMPLETE               0x20
#define STREG0_END_NORMAL                  0x00
#define STREG0_END_ERROR                   0x40
#define STREG0_END_INVALID_COMMAND         0x80
#define STREG0_END_DRIVE_NOT_READY         0xC0
#define STREG0_END_MASK                    0xC0

//
// Bits in status register 1.
//

#define STREG1_ID_NOT_FOUND                0x01
#define STREG1_WRITE_PROTECTED             0x02
#define STREG1_SECTOR_NOT_FOUND            0x04
#define STREG1_RESERVED1                   0x08
#define STREG1_DATA_OVERRUN                0x10
#define STREG1_CRC_ERROR                   0x20
#define STREG1_RESERVED2                   0x40
#define STREG1_END_OF_DISKETTE             0x80

//
// Bits in status register 2.
//

#define STREG2_SUCCESS                     0x00
#define STREG2_DATA_NOT_FOUND              0x01
#define STREG2_BAD_CYLINDER                0x02
#define STREG2_SCAN_FAIL                   0x04
#define STREG2_SCAN_EQUAL                  0x08
#define STREG2_WRONG_CYLINDER              0x10
#define STREG2_CRC_ERROR                   0x20
#define STREG2_DELETED_DATA                0x40
#define STREG2_RESERVED                    0x80

//
// Bits in status register 3.
//

#define STREG3_DRIVE_0                     0x00
#define STREG3_DRIVE_1                     0x01
#define STREG3_DRIVE_2                     0x02
#define STREG3_DRIVE_3                     0x03
#define STREG3_HEAD                        0x04
#define STREG3_TWO_SIDED                   0x08
#define STREG3_TRACK_0                     0x10
#define STREG3_DRIVE_READY                 0x20
#define STREG3_WRITE_PROTECTED             0x40
#define STREG3_DRIVE_FAULT                 0x80

#define VALID_NEC_FDC                      0x90    // version number
#define NSC_PRIMARY_VERSION                0x70    // National 8477 verion number
#define NSC_MASK                           0xF0    // mask for National version number
#define INTEL_MASK                         0xe0
#define INTEL_44_PIN_VERSION               0x40
#define INTEL_64_PIN_VERSION               0x00

typedef struct _FLOPPYDRV {
    DISK_GEOMETRY Geom;
    PFLOPPY_PORT_LAYOUT Port;            // 3f0
    UCHAR               FifoBuffer[10];
    ULONG DriveId; //0-3
    ULONG DriveType;
    ULONG CurPos;
    ULONG Size;
} FLOPPYDRV, *PFLOPPYDRV;

//
// The byte in the boot sector that specifies the type of media, and
// the values that it can assume.  We can often tell what type of media
// is in the drive by seeing which controller parameters allow us to read
// the diskette, but some different densities are readable with the same
// parameters so we use this byte to decide the media type.
//

typedef struct _FLOPPY_BOOT_SECTOR_INFO {
    UCHAR   JumpByte[1];
    UCHAR   Ignore1[2];
    UCHAR   OemData[8];
    UCHAR   BytesPerSector[2];
    UCHAR   Ignore2[6];
    UCHAR   NumberOfSectors[2];
    UCHAR   MediaByte[1];
    UCHAR   Ignore3[2];
    UCHAR   SectorsPerTrack[2];
    UCHAR   NumberOfHeads[2];
} FLOPPY_BOOT_SECTOR_INFO, *PFLOPPY_BOOT_SECTOR_INFO;


//
// Retry counts -
//
// When moving a byte to/from the FIFO, we sit in a tight loop for a while
// waiting for the controller to become ready.  The number of times through
// the loop is controlled by FIFO_TIGHTLOOP_RETRY_COUNT.  When that count
// expires, we'll wait in 10ms increments.  FIFO_DELAY_RETRY_COUNT controls
// how many times we wait.
//
// The ISR_SENSE_RETRY_COUNT is the maximum number of 1 microsecond
// stalls that the ISR will do waiting for the controller to accept
// a SENSE INTERRUPT command.  We do this because there is a hardware
// quirk in at least the NCR 8 processor machine where it can take
// up to 50 microseconds to accept the command.
//
// When attempting I/O, we may run into many different errors.  The
// hardware retries things 8 times invisibly.  If the hardware reports
// any type of error, we will recalibrate and retry the operation
// up to RECALIBRATE_RETRY_COUNT times.  When this expires, we check to
// see if there's an overrun - if so, the DMA is probably being hogged
// by a higher priority device, so we repeat the earlier loop up to
// OVERRUN_RETRY_COUNT times.
//
// Any packet that is about to be returned with an error caused by an
// unexpected hardware error or state will be restarted from the very
// beginning after resetting the hardware HARDWARE_RESET_RETRY_COUNT
// times.
//

#define FIFO_TIGHTLOOP_RETRY_COUNT         500
#define FIFO_ISR_TIGHTLOOP_RETRY_COUNT     25
#define ISR_SENSE_RETRY_COUNT              50
#define FIFO_DELAY_RETRY_COUNT             5
#define RECALIBRATE_RETRY_COUNT            3
#define OVERRUN_RETRY_COUNT                1
#define HARDWARE_RESET_RETRY_COUNT         2
#define FLOPPY_RESET_ISR_THRESHOLD         20


#define COMMAND_MASK                       0x1f

typedef struct _COMMAND_TABLE {
    UCHAR   NumberOfParameters;
    UCHAR   FirstResultByte;
    UCHAR   NumberOfResultBytes;
    BOOLEAN InterruptExpected;
    BOOLEAN AlwaysImplemented;
} COMMAND_TABLE;

#endif //__DBGFLOPPY_POLL_MODE__H__
