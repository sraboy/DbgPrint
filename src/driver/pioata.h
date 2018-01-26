#ifndef __DBG_ATA_POLL_PIO_MODE__H__
#define __DBG_ATA_POLL_PIO_MODE__H__

extern "C" {

#include <ntddk.h>
#include <ntdddisk.h>

};
#include "stddef.h"
#include "..\sdk\CrossNt\CrossNt.h"

NTSTATUS
DbgAtaPreInit(
    ULONG BusType,    // PCI/ISA
    ULONG BusId,      // 0 for ISA
    ULONG BusAddress,
    ULONG ldev        // Channel/Target
    );

#endif //__DBG_ATA_POLL_PIO_MODE__H__