/*

Copyleft (L) 2006-2008  AlterWare Beast Inc.   >;->

Module Name:        CrossNt.cpp
Abstract:           
Authors:            Alter
Environment:        kernel mode only

 */

#include "crossnt.h"

ULONG  MajorVersion;
ULONG  MinorVersion;
ULONG  BuildNumber;
ULONG  SPVersion;

UNICODE_STRING g_SavedSPString;

BOOLEAN        g_CrNtInited = FALSE;
BOOLEAN        g_ReactOS = FALSE;

PLIST_ENTRY    g_LoadOrderListHead = NULL;
UNICODE_STRING g_CmCSDVersionString;

PCHAR          g_KeNumberProcessors = NULL;

HANDLE         g_hNtosKrnl = NULL;
UNICODE_STRING g_NtoskrnlNameString = {
    sizeof(L"NTOSKRNL.EXE")-sizeof(WCHAR),
    sizeof(L"NTOSKRNL.EXE"),
    L"NTOSKRNL.EXE"};

HANDLE         g_hHal = NULL;
UNICODE_STRING g_HalNameString = {
    sizeof(L"HAL.DLL")-sizeof(WCHAR),
    sizeof(L"HAL.DLL"),
    L"HAL.DLL"};

#ifdef _DEBUG

// NT3.51 doesn't export strlen() and strcmp()
// The same time, Release build doesn't depend no these functions since they are inlined

size_t __cdecl CrNtstrlen (
        const char * str
        )
{
        const char *eos = str;

        while( *eos++ ) ;

        return( (int)(eos - str - 1) );
}

int __cdecl CrNtstrcmp (
        const char * src,
        const char * dst
        )
{
        int ret = 0 ;

        while( ! (ret = *(unsigned char *)src - *(unsigned char *)dst) && *dst)
                ++src, ++dst;

        if ( ret < 0 )
                ret = -1 ;
        else if ( ret > 0 )
                ret = 1 ;

        return( ret );
}

#endif //_DEBUG

int __cdecl CrNtstricmp (
        const char * src,
        const char * dst
        )
{
        int ret = 0 ;

        while( ! (ret = ((*(unsigned char *)src - *(unsigned char *)dst)) & ~('a' ^ 'A')) && *dst)
                ++src, ++dst;

        if ( ret < 0 )
                ret = -1 ;
        else if ( ret > 0 )
                ret = 1 ;

        return( ret );
} // end CrNtstricmp()

PVOID
CrNtSkipImportStub(
    PVOID p
    )
{
    if(((PUCHAR)p)[0] == 0xff && ((PUCHAR)p)[1] == 0x25) {
        p = (PVOID)(*(PULONG)(*(PULONG)((ULONG)p+2)));
    }
    return p;
} // end CrNtSkipImportStub()

#define CROSSNT_DECL_STUB

#include "CrNtDecl.h"
#include "CrNtStubs.h"

#undef CROSSNT_DECL_STUB

extern "C"
ptrCrNtPsGetVersion  CrNtPsGetVersion = NULL;

extern "C"
ptrCrNtNtQuerySystemInformation  CrNtNtQuerySystemInformation = NULL;

NTSTATUS
CrNtNtQuerySystemInformation_impl(
    IN SYSTEM_INFORMATION_CLASS SystemInfoClass,
    OUT PVOID SystemInfoBuffer,
    IN ULONG SystemInfoBufferSize,
    OUT PULONG BytesReturned OPTIONAL
);

BOOLEAN
CrNtPsGetVersion_impl(
    PULONG MajorVersion OPTIONAL,
    PULONG MinorVersion OPTIONAL,
    PULONG BuildNumber OPTIONAL,
    PUNICODE_STRING CSDVersion OPTIONAL
    );

BOOLEAN
CrNtPsGetVersion_impl2(
    PULONG MajorVersion OPTIONAL,
    PULONG MinorVersion OPTIONAL,
    PULONG BuildNumber OPTIONAL,
    PUNICODE_STRING CSDVersion OPTIONAL
    );

BOOLEAN
CrNtPsGetVersion_impl0(
    PULONG MajorVersion OPTIONAL,
    PULONG MinorVersion OPTIONAL,
    PULONG BuildNumber OPTIONAL,
    PUNICODE_STRING CSDVersion OPTIONAL
    );

NTSTATUS
CrNtInit(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
{
    PLIST_ENTRY Next;
    PLIST_ENTRY LoadOrderListHead;
    PLDR_DATA_TABLE_ENTRY LdrDataTableEntry;
    PLDR_DATA_TABLE_ENTRY LdrDataTableEntry0;
    PVOID p;
    NTSTATUS NtStatus;
    ULONG SystemInfoBufferSize = 0;
    ULONG i;
    WCHAR a;

    if(g_CrNtInited)
        return STATUS_SUCCESS;

    // We are called from boot-driver, DriverSection may be NULL :(

    // Locate NTOSKRNL.EXE
    KdPrint(("Locate NTOSKRNL.EXE\n"));
    p = (PVOID)NtBuildNumber;
    //p = CrNtSkipImportStub(p);
    g_hNtosKrnl = CrNtFindModuleBaseByPtr(p, "NtBuildNumber");
    if(!g_hNtosKrnl) {
        KdPrint(("  !g_hNtosKrnl\n"));
        return STATUS_UNSUCCESSFUL;
    }

    g_KeNumberProcessors = (PCHAR)CrNtGetProcAddress(g_hNtosKrnl, "KeNumberProcessors");

    // Locate HAL.DLL
    p = (PVOID)HalDisplayString;
    p = CrNtSkipImportStub(p);
    g_hHal = CrNtFindModuleBaseByPtr(p, "HalDisplayString");
    if(!g_hHal) {
        KdPrint(("  !g_hHal\n"));
        return STATUS_UNSUCCESSFUL;
    }

    CrNtNtQuerySystemInformation = (ptrCrNtNtQuerySystemInformation)CrNtGetProcAddress(g_hNtosKrnl, "ZwQuerySystemInformation");
    if(CrNtNtQuerySystemInformation) {
        // NT 4.0 or higher or ReactOS
        CrNtPsGetVersion = (ptrCrNtPsGetVersion)CrNtGetProcAddress(g_hNtosKrnl, "PsGetVersion");
        if(!CrNtPsGetVersion) {
            // unknown OS, don't know how to handle
            CrNtPsGetVersion = CrNtPsGetVersion_impl2;
        }
        // check for ReactOS
        NtStatus = CrNtNtQuerySystemInformation(SystemModuleInformation,
            &SystemInfoBufferSize,
            0,
            &SystemInfoBufferSize);
        if(NtStatus == STATUS_NOT_IMPLEMENTED || !SystemInfoBufferSize) {
            // seems, ReactOS has us
            g_ReactOS = TRUE;
        }
    } else {
        // looks like we are under 3.51
        KdPrint(("looks like we are under 3.51\n"));
        __try
        {
            if(!g_LoadOrderListHead) {
                LdrDataTableEntry0 = (PLDR_DATA_TABLE_ENTRY)DriverObject->DriverSection;

                if(LdrDataTableEntry0) {

                    Next = LdrDataTableEntry0->LoadOrder.Blink;
                    while (TRUE) {
                        LdrDataTableEntry = CONTAINING_RECORD( Next,
                                                               LDR_DATA_TABLE_ENTRY,
                                                               LoadOrder
                                                             );
                        __try {
                            Next = Next->Blink;
                        } __except(EXCEPTION_EXECUTE_HANDLER) {
                            return STATUS_UNSUCCESSFUL;
                        }
                        if(!LdrDataTableEntry->ModuleName.Buffer) {
                            //HalDisplayString((PUCHAR)"\nCrNtInit !LdrDataTableEntry->ModuleName.Buffer\n");
                            return STATUS_UNSUCCESSFUL;
                        }
                        __try {
                            if(RtlCompareUnicodeString(&LdrDataTableEntry->ModuleName, &g_NtoskrnlNameString, TRUE) == 0)
                            {
                                LoadOrderListHead = Next;
                                break;
                            }
                        } __except(EXCEPTION_EXECUTE_HANDLER) {
                        }

                        if(LdrDataTableEntry == LdrDataTableEntry0) {
                            return STATUS_UNSUCCESSFUL;
                        }
                    }
                    g_LoadOrderListHead = LoadOrderListHead;
                }
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            return STATUS_UNSUCCESSFUL;
        }

        if(!g_hNtosKrnl) {
            g_hNtosKrnl = CrNtGetModuleBase("NTOSKRNL.EXE");
        }
        if(!g_hNtosKrnl)
            return STATUS_UNSUCCESSFUL;
        if(!g_hHal) {
            g_hHal = CrNtGetModuleBase("HAL.DLL");
        }
        if(!g_hHal)
            return STATUS_UNSUCCESSFUL;

        CrNtPsGetVersion = CrNtPsGetVersion_impl;
        CrNtNtQuerySystemInformation = CrNtNtQuerySystemInformation_impl;
    }

    g_CmCSDVersionString.Buffer = L"";
    g_CmCSDVersionString.Length = 0;
    g_CmCSDVersionString.MaximumLength = 0;

    CrNtPsGetVersion(&MajorVersion, &MinorVersion, &BuildNumber, &g_CmCSDVersionString);
    if(g_ReactOS) {
        MinorVersion = 0x01;
    }

    if(g_SavedSPString.Length) {
        for(i=g_SavedSPString.Length/sizeof(WCHAR)-1; i>0; i--) {
            a = g_SavedSPString.Buffer[i];
            if(a >= '0' && a <= '9') {
                SPVersion = (a - '0') << 8;
                if(i != g_SavedSPString.Length/sizeof(WCHAR)-1) {
                    SPVersion++; // (0x0600 -> 0x0601) for sp6a
                }
                break;
            }
        }
    } else {
        SPVersion = 0;
    }
    CrNtPsGetVersion = CrNtPsGetVersion_impl0;

#define CROSSNT_INIT_STUB

#include "CrNtDecl.h"
#include "CrNtStubs.h"

#undef CROSSNT_INIT_STUB

    KdPrint(("check CPU Gen\n"));
    i = CrNtGetCPUGen();
    KdPrint(("CPU Gen %x\n", i));
    if(i < 4) {
        KdPrint(("we live on old i386\n"));
        if(*g_KeNumberProcessors > 1) {
            KdPrint(("  MP\n"));
            CrNtInterlockedIncrement       = CrNtInterlockedIncrement_impl_i386_MP;
            CrNtInterlockedDecrement       = CrNtInterlockedDecrement_impl_i386_MP;
            CrNtInterlockedExchangeAdd     = CrNtInterlockedExchangeAdd_impl_i386_MP;
            CrNtInterlockedCompareExchange = CrNtInterlockedCompareExchange_impl_i386_MP;
        } else {
            KdPrint(("  UP\n"));
            CrNtInterlockedIncrement       = CrNtInterlockedIncrement_impl_i386_UP;
            CrNtInterlockedDecrement       = CrNtInterlockedDecrement_impl_i386_UP;
            CrNtInterlockedExchangeAdd     = CrNtInterlockedExchangeAdd_impl_i386_UP;
            CrNtInterlockedCompareExchange = CrNtInterlockedCompareExchange_impl_i386_UP;
        }
        _MOV_QD_SWP  = _MOV_QD_SWP_i386;
        _MOV_DD_SWP  = _MOV_DD_SWP_i386;
        _REVERSE_DD  = _REVERSE_DD_i386;
        _MOV_MSF_SWP = _MOV_MSF_SWP_i386;
    } else {
        KdPrint(("ILOCK ok\n"));
        if(WinVer_Is351) {
            // special case for NT3.51, regardless of presence of native imlementation
            CrNtInterlockedIncrement       = CrNtInterlockedIncrement_impl;
            CrNtInterlockedDecrement       = CrNtInterlockedDecrement_impl;
        }
        _MOV_QD_SWP  = _MOV_QD_SWP_i486;
        _MOV_DD_SWP  = _MOV_DD_SWP_i486;
        _REVERSE_DD  = _REVERSE_DD_i486;
        _MOV_MSF_SWP = _MOV_MSF_SWP_i486;
    }

    g_CrNtInited = TRUE;

    return STATUS_SUCCESS;
} // end CrNtInit()

__declspec(naked)
ULONG
CrNtGetCPUGen()
{
    _asm {
        push   ebx
        push   ecx ; cpuid may change ecx
        push   edx ; cpuid may change edx

        pushfd
        cli

        pushfd
        pop    eax
        mov    ebx,eax
        xor    eax, 040000h ; toggle bit 18
        push   eax
        popfd
        pushfd
        pop    eax

        popfd

        and    eax, 040000h
        and    ebx, 040000h
        cmp    eax,ebx
        jne    test_486

        mov    eax,3
        jmp    short q_cpu_gen

test_486:

        pushfd
        cli

        pushfd
        pop    eax
        mov    ebx,eax
        xor    eax, 0200000h ; toggle bit 21
        push   eax
        popfd
        pushfd
        pop    eax

        popfd

        and    eax, 0200000h
        and    ebx, 0200000h
        cmp    eax,ebx
        jne    test_586

        mov    eax,4
        jmp    short q_cpu_gen

test_586:

        mov    eax,1
        cpuid
        and    eax, 0f00h
        shr    eax, 8
        mov    ebx,eax

        mov    eax,5
        cmp    ebx,5
        je     short q_cpu_gen

        mov    eax,ebx
        and    eax,0ffffh

q_cpu_gen:

        pop    edx
        pop    ecx
        pop    ebx
        ret
    }
} // end CrNtGetCPUGen()

PVOID
CrNtGetModuleBaseStd(
    PCHAR  pModuleName
    )
{
    PVOID pModuleBase = NULL;
    PULONG pSystemInfoBuffer = NULL;
    
    __try
    {
        NTSTATUS NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        ULONG    SystemInfoBufferSize = 0;
        
        NtStatus = CrNtNtQuerySystemInformation(SystemModuleInformation,
            &SystemInfoBufferSize,
            0,
            &SystemInfoBufferSize);
        if (SystemInfoBufferSize)
        {
            pSystemInfoBuffer = (PULONG)ExAllocatePool(NonPagedPool, SystemInfoBufferSize*2);
            
            if (pSystemInfoBuffer)
            {
                memset(pSystemInfoBuffer, 0, SystemInfoBufferSize*2);
                
                NtStatus = CrNtNtQuerySystemInformation(SystemModuleInformation,
                    pSystemInfoBuffer,
                    SystemInfoBufferSize*2,
                    &SystemInfoBufferSize);
                if (NT_SUCCESS(NtStatus))
                {
                    PSYSTEM_MODULE_ENTRY pSysModuleEntry =((PSYSTEM_MODULE_INFORMATION)(pSystemInfoBuffer))->Module;
                    ULONG i;
                    
                    for (i = 0; i < ((PSYSTEM_MODULE_INFORMATION)(pSystemInfoBuffer))->Count; i++)
                    {
                        if (CrNtstricmp(pSysModuleEntry[i].ModuleName + pSysModuleEntry[i].ModuleNameOffset, pModuleName) == 0)
                        {
                            pModuleBase = pSysModuleEntry[i].ModuleBaseAddress;
                            break;
                        }
                    }
                }
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        pModuleBase = NULL;
    }
    if(pSystemInfoBuffer) {
        ExFreePool(pSystemInfoBuffer);
    }
    
    return pModuleBase;
} // end CrNtGetModuleBaseStd()

PVOID
CrNtGetModuleBaseLdr(
    IN PCHAR  pModuleName
    )
{
    PVOID pModuleBase = NULL;
    PLIST_ENTRY Next;
    PLIST_ENTRY LoadOrderListHead;
    UNICODE_STRING uStr;
    PLDR_DATA_TABLE_ENTRY LdrDataTableEntry;
    ULONG len;
    BOOLEAN FreeUstr = FALSE;

    uStr.Buffer = NULL;

    __try
    {
        len = strlen(pModuleName);
        if(!len)
            return NULL;
        len = (len+1)*sizeof(WCHAR);

        uStr.MaximumLength = (USHORT)len;
        uStr.Length = (USHORT)len - sizeof(WCHAR);
        uStr.Buffer = (PWCHAR)ExAllocatePool(NonPagedPool, len);
        FreeUstr = TRUE;
        swprintf(uStr.Buffer, L"%S", pModuleName);

        if(!g_LoadOrderListHead) {
            ExFreePool(uStr.Buffer);
            return CrNtGetModuleBaseStd(pModuleName);
        }
        LoadOrderListHead = g_LoadOrderListHead;

        Next = LoadOrderListHead->Flink;
        while(Next != LoadOrderListHead) {
            LdrDataTableEntry = CONTAINING_RECORD(Next,
                                                  LDR_DATA_TABLE_ENTRY,
                                                  LoadOrder
                                                 );
            if(RtlCompareUnicodeString(&LdrDataTableEntry->ModuleName, &uStr, TRUE) == 0)
            {
                pModuleBase = LdrDataTableEntry->ModuleBaseAddress;
                break;
            }
            Next = Next->Flink;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        pModuleBase = NULL;
    }
    if(FreeUstr && uStr.Buffer) {
        ExFreePool(uStr.Buffer);
    }

    return pModuleBase;
} // CrNtGetModuleBaseLdr()

PVOID
CrNtGetModuleBase(
    IN PCHAR  pModuleName
    )
{
    UNICODE_STRING uStr;
    ULONG len;

    len = strlen(pModuleName);
    if(!len)
        return NULL;

    if(len == (sizeof("NTOSKRNL.EXE")-1) ||
       len == (sizeof("HAL.DLL")-1) ) {
        len = (len+1)*sizeof(WCHAR);

        uStr.MaximumLength = (USHORT)len;
        uStr.Length = (USHORT)len - sizeof(WCHAR);
        uStr.Buffer = (PWCHAR)ExAllocatePool(NonPagedPool, len);
        if(!uStr.Buffer)
            return NULL;
        swprintf(uStr.Buffer, L"%S", pModuleName);

        if(g_hNtosKrnl && (RtlCompareUnicodeString(&uStr, &g_NtoskrnlNameString, TRUE) == 0))
        {
            ExFreePool(uStr.Buffer);
            return g_hNtosKrnl;
        }
        if(g_hHal && (RtlCompareUnicodeString(&uStr, &g_HalNameString, TRUE) == 0))
        {
            ExFreePool(uStr.Buffer);
            return g_hHal;
        }
        ExFreePool(uStr.Buffer);
    }
    if(g_ReactOS || (WinVer_Id() < WinVer_NT)) {
        return CrNtGetModuleBaseLdr(pModuleName);
    }
    return CrNtGetModuleBaseStd(pModuleName);
} // end CrNtGetModuleBase()

extern "C"
PVOID
CrNtFindModuleBaseByPtr(
    IN PVOID  ptrInSection,
    IN PCHAR  ptrExportedName
    )
{
    PUCHAR p;
    PIMAGE_DOS_HEADER dos;
    PIMAGE_NT_HEADERS nt;

    KdPrint(("CrNtFindModuleBaseByPtr %#x, %s\n", ptrInSection, ptrExportedName));
    p = (PUCHAR)((ULONG)ptrInSection & ~(PAGE_SIZE-1));

    for(;p;p -= PAGE_SIZE) {
        __try
        {
            dos = (PIMAGE_DOS_HEADER)p;
            if(dos->e_magic != 0x5a4d)
                continue;

            nt = (PIMAGE_NT_HEADERS)((ULONG)dos + dos->e_lfanew);
            if((ULONG)nt >= (ULONG)ptrInSection)
                continue;
            if((ULONG)nt <= (ULONG)dos)
                continue;

            if(nt->Signature != 0x00004550)
                continue;
                
            if(!ptrExportedName) {
                break;
            } else {
                if(ptrInSection == CrNtGetProcAddress(p, ptrExportedName)) {
                    break;
                }
            }
            p = NULL;
            break;
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
        }
    }
    KdPrint(("CrNtFindModuleBaseByPtr: %#x\n", p));
    return p;
} // end CrNtFindModuleBaseByPtr()

PVOID
CrNtGetProcAddress(
    PVOID ModuleBase,
    PCHAR pFunctionName
    )
{
    PVOID pFunctionAddress = NULL;
    
    KdPrint(("CrNtGetProcAddress %#x, %s\n", ModuleBase, pFunctionName));
    if(!ModuleBase)
        return NULL;

    __try
    {
        PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)ModuleBase;
        PIMAGE_NT_HEADERS nt  = (PIMAGE_NT_HEADERS)((ULONG)ModuleBase + dos->e_lfanew);

        PIMAGE_DATA_DIRECTORY expdir = (PIMAGE_DATA_DIRECTORY)nt->OptionalHeader.DataDirectory + IMAGE_DIRECTORY_ENTRY_EXPORT;
        ULONG                 size   = expdir->Size;
        ULONG                 addr   = expdir->VirtualAddress;

        PIMAGE_EXPORT_DIRECTORY exports = (PIMAGE_EXPORT_DIRECTORY)((ULONG)ModuleBase + addr);

        PULONG functions = (PULONG)((ULONG)ModuleBase + exports->AddressOfFunctions);
        PSHORT ordinals  = (PSHORT)((ULONG)ModuleBase + exports->AddressOfNameOrdinals);
        PULONG names     = (PULONG)((ULONG)ModuleBase + exports->AddressOfNames);
        ULONG  max_name  = exports->NumberOfNames;
        ULONG  max_func  = exports->NumberOfFunctions;

        ULONG i;

        for (i = 0; i < max_name; i++)
        {
            ULONG ord = ordinals[i];
            if(i >= max_name || ord >= max_func) {
                KdPrint(("CrNtGetProcAddress: i %#x, names %#x, ord %#x, funcs %#x\n", i, max_name, ord, max_func));
                return NULL;
            }
            if (functions[ord] < addr || functions[ord] >= addr + size)
            {
                if (strcmp((PCHAR)ModuleBase + names[i], pFunctionName)  == 0)
                {
                    pFunctionAddress = (PVOID)((PCHAR)ModuleBase + functions[ord]);
                    break;
                }
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        KdPrint(("CrNtGetProcAddress: exception\n"));
        pFunctionAddress = NULL;
    }

    KdPrint(("CrNtGetProcAddress: %#x\n", pFunctionAddress));
    return pFunctionAddress;
} // end CrNtGetProcAddress()

BOOLEAN
CrNtPsGetVersion_impl(
    PULONG MajorVersion OPTIONAL,
    PULONG MinorVersion OPTIONAL,
    PULONG BuildNumber OPTIONAL,
    PUNICODE_STRING CSDVersion OPTIONAL
    )
{
    // Assume this is 3.51
    if (ARGUMENT_PRESENT(MajorVersion)) {
        *MajorVersion = 0x03;
    }
    if (ARGUMENT_PRESENT(MinorVersion)) {
        *MinorVersion = 0x51;
    }
    if (ARGUMENT_PRESENT(BuildNumber)) {
        *BuildNumber = NtBuildNumber & 0x3FFF;
    }
    if (ARGUMENT_PRESENT(CSDVersion)) {
        *CSDVersion = g_CmCSDVersionString;
    }
    return TRUE;
} // end CrNtGetVersion()

BOOLEAN
CrNtPsGetVersion_impl2(
    PULONG MajorVersion OPTIONAL,
    PULONG MinorVersion OPTIONAL,
    PULONG BuildNumber OPTIONAL,
    PUNICODE_STRING CSDVersion OPTIONAL
    )
{
    // Unknown OS
    if (ARGUMENT_PRESENT(MajorVersion)) {
        *MajorVersion = 0x00;
    }
    if (ARGUMENT_PRESENT(MinorVersion)) {
        *MinorVersion = 0x00;
    }
    if (ARGUMENT_PRESENT(BuildNumber)) {
        *BuildNumber = NtBuildNumber & 0x3FFF;
    }
    if (ARGUMENT_PRESENT(CSDVersion)) {
        *CSDVersion = g_CmCSDVersionString;
    }
    return FALSE;
} // end CrNtGetVersion()

BOOLEAN
CrNtPsGetVersion_impl0(
    PULONG _MajorVersion OPTIONAL,
    PULONG _MinorVersion OPTIONAL,
    PULONG _BuildNumber OPTIONAL,
    PUNICODE_STRING _CSDVersion OPTIONAL
    )
{
    if (ARGUMENT_PRESENT(_MajorVersion)) {
        *_MajorVersion = MajorVersion;
    }
    if (ARGUMENT_PRESENT(_MinorVersion)) {
        *_MinorVersion = MinorVersion;
    }
    if (ARGUMENT_PRESENT(_BuildNumber)) {
        *_BuildNumber = BuildNumber;
    }
    if (ARGUMENT_PRESENT(_CSDVersion)) {
        *_CSDVersion = g_CmCSDVersionString;
    }
    return TRUE;
} // end CrNtGetVersion()

__declspec (naked)
HANDLE
CrNtPsGetCurrentProcessId_impl() 
{
    _asm {
        mov eax,fs:[124h]
        mov eax,[eax+1e0h]
        ret
    }
}

__declspec (naked)
HANDLE
CrNtPsGetCurrentThreadId_impl() 
{
    _asm {
        mov eax,fs:[124h]
        mov eax,[eax+1e4h]
        ret
    }
}

NTSTATUS
CrNtNtQuerySystemInformation_impl(
    IN SYSTEM_INFORMATION_CLASS SystemInfoClass,
    OUT PVOID SystemInfoBuffer,
    IN ULONG SystemInfoBufferSize,
    OUT PULONG BytesReturned OPTIONAL
)
{
    return STATUS_UNSUCCESSFUL;
}

BOOLEAN
__fastcall
CrNtKeTestSpinLock_impl(
    IN PKSPIN_LOCK SpinLock
    )
{
    if(!*SpinLock)
    {
        return TRUE;
    }
    __asm pause;
    return FALSE;
} // end CrNtKeTestSpinLock_impl()

KIRQL
__stdcall
CrNtKeRaiseIrqlToDpcLevel_impl()
{
    KIRQL OldIrql;
    KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);
    return OldIrql;
} // end CrNtKeRaiseIrqlToDpcLevel_impl()

KIRQL
__stdcall
CrNtKeRaiseIrqlToSynchLevel_impl()
{
    KIRQL OldIrql;
    KeRaiseIrql(SYNCH_LEVEL, &OldIrql);
    return OldIrql;
} // end CrNtKeRaiseIrqlToSynchLevel_impl()

