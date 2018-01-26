#include "stdafx.h"

POSTMORT_HANDLE
PMort_OpenFileW(
    IN PWCHAR FileName
    )
{
    HANDLE fh = NULL;
    POSTMORT_HANDLE pmh = NULL;

    // Alloc dump manipulation context (handle)
    pmh = (POSTMORT_HANDLE)GlobalAlloc(GMEM_FIXED, sizeof(PMORT_HANDLE));
    if(!pmh) {
        goto exit;
    }
    memset(pmh, 0, sizeof(PMORT_HANDLE));

    // Open dump file
    fh = CreateFileW(FileName,
                     GENERIC_READ, FILE_SHARE_READ, NULL,
                     OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if(!fh || fh == (HANDLE)(-1)) {
        fh = NULL;
        goto exit;
    }

    // Map dump to memory
    pmh->hMap = CreateFileMapping(fh, NULL, PAGE_READONLY, 0, 0, NULL);

    if(!pmh->hMap) {
        goto exit;
    }

    pmh->pMap = (PUCHAR)MapViewOfFile(pmh->hMap, FILE_MAP_READ, 0, 0, 0);

    if(!pmh->pMap) {
        goto exit;
    }

    pmh->hdr = (PDUMP_HEADER)(pmh->pMap);

    pmh->FileSize = GetFileSize(fh, NULL);
    if(pmh->FileSize < sizeof(DUMP_HEADER)) {
        goto exit;
    }

    // Check if kernel mode dump
    if(memcmp(pmh->hdr->Signature, DUMP_SIG_KERNELMODE, 8) == 8) {
        pmh->KernelDump = TRUE;
    } else
    // Check if user mode dump
    if(memcmp(pmh->hdr->Signature, DUMP_SIG_USERMODE, 8) == 8) {
        // not supported yet, exit
        goto exit;
    }

    pmh->PhysMemDesc = (PPHYSMEM_DESCRIPTOR)(pmh->pMap + 100);

    if(pmh->hdr->MachineImageType != IMAGE_FILE_MACHINE_I386) {
        // not supported yet, exit
        goto exit;
    }

    pmh->PdePage = (PULONG)PMort_GetPhPageAddr(pmh, (pmh->hdr->DirTable & PFN_MASK) >> PFN_SHIFT);
    if(!pmh->PdePage) {
        goto exit;
    }

    // OK
    return pmh;

exit:

    if(fh) {
        CloseHandle(fh);
    }
    PMort_Close(pmh);

    return NULL;
} // PMort_OpenFile

VOID
PMort_Close(
    POSTMORT_HANDLE pmh     // crash dump handle
    )
{
    if(!pmh)
        return;
    if(pmh->hMap) {
        CloseHandle(pmh->hMap);
    }
    if(pmh->pMap) {
        UnmapViewOfFile(pmh->pMap);
    }
    GlobalFree(pmh);
} // end PMort_Close()

PVOID
PMort_GetPhPageAddr(
    IN POSTMORT_HANDLE pmh, // crash dump handle
    IN ULONG Page           // physical page number
    )
{
    ULONG lim;
    ULONG i;
    ULONG offs = 1;
    PPHYSMEM_EXTENT ext;

    lim = pmh->PhysMemDesc->ExtentCount;
    offs = 1;
    ext = &(pmh->PhysMemDesc->Extent[0]);
    for(i=0; i < lim; i++) {
        if ((Page >= ext[i].BasePage) &&
            (Page < (ext[i].BasePage +
                     ext[i].PageCount))) {
            offs += Page - ext[i].BasePage;
            return (PVOID)(pmh->pMap + (offs * PAGE_SIZE));
        }
        offs += ext[i].PageCount;
    }
    return NULL;
} // end PMort_GetPhPageAddr()

ULONG
PMort_ReadPhMemory(
    IN POSTMORT_HANDLE pmh, // crash dump handle
    IN ULONG PhAddr,        // physical address
   OUT PVOID Buffer,        // caller's buffer
    IN ULONG Size           // buffer size (bytes)
    )
{
    PUCHAR addr;
    ULONG offs=0;
    ULONG to_read;
    ULONG d;

    while(Size) {
        addr = (PUCHAR)PMort_GetPhPageAddr(pmh, (PhAddr+offs)/PAGE_SIZE);
        if(!addr)
            return offs;
        d = (PhAddr+offs) & (PAGE_SIZE-1);
        addr += d;
        to_read = min(Size, PAGE_SIZE-d);
        memcpy((PUCHAR)Buffer + offs, addr, to_read);
        Size -= to_read;
        offs += to_read;
    }

    return offs;
} // end PMort_ReadPhMemory()
