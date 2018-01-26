#include "pioata.h"

#define UNIATA_PIO_ONLY

#include "atapi.h"               // includes scsi.h
#include "ntdddisk.h"
#include "ntddscsi.h"

#include "bsmaster.h"

PSCSI_REQUEST_BLOCK pCurRequest=NULL;
SCSI_REQUEST_BLOCK  CurRequest;
ATA_REQ             AtaReq;

BUSMASTER_CONTROLLER_INFORMATION  BMListItems[2];

HW_DEVICE_EXTENSION AtaDeviceExtension;

PCI_COMMON_CONFIG     pciData;

PORT_CONFIGURATION_INFORMATION_COMMON _ConfigInfo;
PPORT_CONFIGURATION_INFORMATION       ConfigInfo;

ACCESS_RANGE AccessRanges[6];

ULONG g_ldev=-1;

#define ScsiPortNotification(a,b,c,d)
#define AtapiHwInitializeChanger(a,b,c)

#define AtapiDmaDone(a,b,c,d)       0
#undef GetDmaStatus
#define GetDmaStatus(a,b)           0

#define AtapiDmaInit__(a,b)         FALSE
#define AtapiDmaInit(a,b)           FALSE
#define AtapiDmaReinit(a,b,c)
#define AtapiDmaStart(a,b,c,d)
#define AtapiDmaSetup(a,b,c,d,e,f)  FALSE
#define AtapiDmaPioSync(a,b,c,d)    TRUE

#define ScsiPortLogError()

#define UniataGetCurRequest(a)       pCurRequest
#define UniataRemoveRequest(a,b)     {pCurRequest=NULL;}
#define IdeMediaStatus(a,b,c)

#define AtapiChipDetect(a,b,c,d,e)  TRUE
#define AtapiChipInit(a,b,c)

#define ScsiPortStallExecution      KeStallExecutionProcessor

#define ScsiPortWriteRegisterUlong            WRITE_REGISTER_ULONG
#define ScsiPortWritePortUlong                WRITE_PORT_ULONG
#define ScsiPortWriteRegisterUshort           WRITE_REGISTER_USHORT
#define ScsiPortWritePortUshort               WRITE_PORT_USHORT
#define ScsiPortWriteRegisterUchar            WRITE_REGISTER_UCHAR
#define ScsiPortWritePortUchar                WRITE_PORT_UCHAR
#define ScsiPortReadRegisterUlong             READ_REGISTER_ULONG 
#define ScsiPortReadPortUlong                 READ_PORT_ULONG     
#define ScsiPortReadRegisterUshort            READ_REGISTER_USHORT
#define ScsiPortReadPortUshort                READ_PORT_USHORT    
#define ScsiPortReadRegisterUchar             READ_REGISTER_UCHAR 
#define ScsiPortReadPortUchar                 READ_PORT_UCHAR     

#define ScsiPortWriteRegisterBufferUlong      WRITE_REGISTER_BUFFER_ULONG 
#define ScsiPortWritePortBufferUlong          WRITE_PORT_BUFFER_ULONG     
#define ScsiPortWriteRegisterBufferUshort     WRITE_REGISTER_BUFFER_USHORT
#define ScsiPortWritePortBufferUshort         WRITE_PORT_BUFFER_USHORT    
#define ScsiPortReadRegisterBufferUlong       READ_REGISTER_BUFFER_ULONG 
#define ScsiPortReadPortBufferUlong           READ_PORT_BUFFER_ULONG     
#define ScsiPortReadRegisterBufferUshort      READ_REGISTER_BUFFER_USHORT
#define ScsiPortReadPortBufferUshort          READ_PORT_BUFFER_USHORT    

#define ScsiPortValidateRange(a,b,c,d,e,f)    TRUE

#define ScsiPortMoveMemory                    RtlMoveMemory

#define ScsiPortSetBusDataByOffset(foo,a,b,c,d,e,f)  HalSetBusDataByOffset(a,b,c,d,e,f)
#define ScsiPortGetBusDataByOffset(foo,a,b,c,d,e,f)  HalGetBusDataByOffset(a,b,c,d,e,f)
#define ScsiPortGetBusData(foo,a,b,c,d,e)            HalGetBusData(a,b,c,d,e)

#define ScsiPortConvertUlongToPhysicalAddress        RtlConvertUlongToLargeInteger

#define ScsiPortFreeDeviceBase(a,b)

PVOID
ScsiPortGetDeviceBase(
    IN PVOID HwDeviceExtension,
    IN INTERFACE_TYPE BusType,
    IN ULONG SystemIoBusNumber,
    SCSI_PHYSICAL_ADDRESS IoAddress,
    ULONG NumberOfBytes,
    BOOLEAN InIoSpace
    )
{
    PHYSICAL_ADDRESS phAddress;
    ULONG addressSpace = InIoSpace;
    PVOID mappedAddress;

    if(!HalTranslateBusAddress(
            BusType,
            SystemIoBusNumber,
            IoAddress,
            &addressSpace,
            &phAddress
            )) {
        return NULL;
    }

    if (!addressSpace) {
        mappedAddress = MmMapIoSpace(phAddress,
                                 NumberOfBytes,
                                 MmNonCached);

    } else {

        mappedAddress = (PVOID)phAddress.LowPart;
    }

    return mappedAddress;

} // end ScsiPortGetDeviceBase()

#include "id_ata.cpp"

#include "id_probe.cpp"

NTSTATUS
DbgAtaPreInit(
    ULONG BusType,    // PCI/ISA
    ULONG BusId,      // 0 for ISA
    ULONG BusAddress,
    ULONG ldev        // Channel/Target
    )
{
    ULONG i=0;
    ULONG j=0;
    ULONG channel;
    ULONG dev;
    ULONG                 busDataRead;
    ULONG status;
    BOOLEAN Again = FALSE;

    UCHAR                 vendorString[5];
    UCHAR                 deviceString[5];

    ULONG   VendorID;
    ULONG   DeviceID;

    ///////////
    
    RtlZeroMemory(&CurRequest,         sizeof(CurRequest));
    RtlZeroMemory(&BMListItems,        sizeof(BMListItems));
    RtlZeroMemory(&AtaDeviceExtension, sizeof(AtaDeviceExtension));
    RtlZeroMemory(&pciData,            sizeof(pciData));
    RtlZeroMemory(&_ConfigInfo,        sizeof(_ConfigInfo));
    RtlZeroMemory(&AccessRanges,       sizeof(AccessRanges));
    RtlZeroMemory(&AtaReq,             sizeof(AtaReq));

    ConfigInfo = (PPORT_CONFIGURATION_INFORMATION)&_ConfigInfo;

    channel = ldev >> 1;
    dev     = ldev & 1;

    BMList[i].slotNumber =
        ConfigInfo->SlotNumber = BusAddress;
    BMList[i].busNumber =
        ConfigInfo->SystemIoBusNumber = BusId;
    BMList[i].channel = (UCHAR)channel;
    *((PULONG)&(ConfigInfo->AccessRanges)) = (ULONG)(&AccessRanges);
    CurRequest.SrbExtension = &AtaReq;

    if(BusType == PCIBus) {

        ConfigInfo->AdapterInterfaceType = PCIBus;

        busDataRead = ScsiPortGetBusData(&AtaDeviceExtension,
                                         PCIConfiguration,
                                         BusId,
                                         BusAddress,
                                         &pciData,
                                         PCI_COMMON_HDR_LENGTH);
        if (busDataRead < PCI_COMMON_HDR_LENGTH) {
            KdPrint2((PRINT_PREFIX "busDataRead < PCI_COMMON_HDR_LENGTH => ERROR\n"));
            return STATUS_UNSUCCESSFUL;
        }

        KdPrint2((PRINT_PREFIX "busDataRead\n"));
        if (pciData.VendorID == PCI_INVALID_VENDORID) {
            KdPrint2((PRINT_PREFIX "PCI_INVALID_VENDORID\n"));
            return STATUS_UNSUCCESSFUL;
        }

        VendorID  = pciData.VendorID;
        DeviceID  = pciData.DeviceID;

        sprintf((PCHAR)vendorString, "%4.4x", VendorID);
        sprintf((PCHAR)deviceString, "%4.4x", DeviceID);

        RtlCopyMemory(&(BMList[i].VendorIdStr), vendorString, 4);
        RtlCopyMemory(&(BMList[i].DeviceIdStr), deviceString, 4);
        
        BMList[i].nVendorId = VendorID;
        BMList[i].VendorId = (PCHAR)&(BMList[i].VendorIdStr);
        BMList[i].VendorIdLength = 4;
        BMList[i].nDeviceId = DeviceID;
        BMList[i].DeviceId = (PCHAR)&(BMList[i].DeviceIdStr);
        BMList[i].DeviceIdLength = 4;

        //BMList[i].RaidFlags = RaidFlags;

        BMList[i].MasterDev = IsMasterDev(&pciData) ? 1 : 0;
        
        for(j=0; j<6; j++) {
            AccessRanges[j].RangeStart = ScsiPortConvertUlongToPhysicalAddress(pciData.u.type0.BaseAddresses[j] & ~0x07);
            AccessRanges[j].RangeInMemory = !(pciData.u.type0.BaseAddresses[j] & PCI_ADDRESS_IO_SPACE);
        }
        AccessRanges[0].RangeLength = AccessRanges[2].RangeLength = ATA_IOSIZE;
        AccessRanges[1].RangeLength = AccessRanges[3].RangeLength = ATA_ALTIOSIZE;

        status = UniataFindBusMasterController(&AtaDeviceExtension,
                                         &i,
                                         NULL,
                                         "",
                                         ConfigInfo,
                                         &Again);

    } else {

        ConfigInfo->AdapterInterfaceType = Isa;

        status = AtapiFindController(&AtaDeviceExtension,
                                         &i,
                                         NULL,
                                         "",
                                         ConfigInfo,
                                         &Again);
    }

    if(status != SP_RETURN_FOUND)
        return STATUS_UNSUCCESSFUL;

    if(ldev > (ULONG)(AtaDeviceExtension.NumberChannels * 2)) {
        return STATUS_UNSUCCESSFUL;
    }

    g_ldev = ldev;

    return STATUS_SUCCESS;

} // DbgAtaPreInit()

//DbgAtaInit()
