#ifndef __DBGDUMP_POSTMORT_H__
#define __DBGDUMP_POSTMORT_H__

typedef struct _DUMP_HEADER {
    UCHAR Signature[8];
    ULONG OsVerMj;
    ULONG OsVerMn;
    ULONG DirTable;
    PULONG PfnData;
    PLIST_ENTRY LoadedModuleList;
    PLIST_ENTRY ActiveProcessHead;
    ULONG MachineImageType;
    ULONG nProcessors;
    ULONG BugCheckCode;
    ULONG BugCheckParameter1;
    ULONG BugCheckParameter2;
    ULONG BugCheckParameter3;
    ULONG BugCheckParameter4;
    CHAR  VersionStr[32];
    ULONG Reserved[2];
} DUMP_HEADER, *PDUMP_HEADER;

#define DUMP_SIG_USERMODE    "USERDUMP"
#define DUMP_SIG_KERNELMODE  "PAGEDUMP"

#ifndef PAGE_SIZE
  #define PAGE_SHIFT 12
  #define PAGE_SIZE  ((ULONG)1 << PAGE_SHIFT)
#endif //PAGE_SIZE

#define PFN_SHIFT 12
#define PFN_MASK  0xfffff000

typedef struct _PHYSMEM_EXTENT {
    ULONG BasePage;
    ULONG PageCount;
} PHYSMEM_EXTENT, *PPHYSMEM_EXTENT;

typedef struct _PHYSMEM_DESCRIPTOR {
    ULONG ExtentCount;
    ULONG PagesCount;
    PHYSMEM_EXTENT Extent[1];
} PHYSMEM_DESCRIPTOR, *PPHYSMEM_DESCRIPTOR;

typedef struct _PMORT_HANDLE {
    
    HANDLE   hMap; // memory mapping handle
    PUCHAR   pMap; // mapped base address
    PDUMP_HEADER hdr; // always === pMap
    LONGLONG FileSize;

    BOOLEAN  KernelDump;

    PPHYSMEM_DESCRIPTOR PhysMemDesc;
    PULONG   PdePage;

} PMORT_HANDLE, *POSTMORT_HANDLE;


POSTMORT_HANDLE
PMort_OpenFileW(
    IN PWCHAR FileName
    );

VOID
PMort_Close(
    POSTMORT_HANDLE pmh
    );

PVOID
PMort_GetPhPageAddr(
    IN POSTMORT_HANDLE pmh, // crash dump handle
    IN ULONG Page           // physical page number
    );

/*
PVOID
PMort_GetPhPageAddrByPhAddr(
    IN POSTMORT_HANDLE pmh, // crash dump handle
    IN ULONG Addr           // physical page number
    );
*/
#define PMort_GetPhPageAddrByPhAddr(h,a)   PMort_GetPhPageAddr(h, (a) >> PAGE_SHIFT )

ULONG
PMort_ReadPhMemory(
    IN POSTMORT_HANDLE pmh, // crash dump handle
    IN ULONG PhAddr,        // physical address
   OUT PVOID Buffer,        // caller's buffer
    IN ULONG Size           // buffer size (bytes)
    );

#endif //__DBGDUMP_POSTMORT_H__
