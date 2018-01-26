#include "regtools.h"

ULONG
RegCheckParameterValue(
    IN PUNICODE_STRING RegistryPath,
    IN PWSTR PathSuffix,
    IN PWSTR Name,
    IN ULONG Default
    )
{
#define ITEMS_TO_QUERY 2 // always 1 greater than what is searched 

    NTSTATUS          status;
    LONG              zero = Default;

    RTL_QUERY_REGISTRY_TABLE parameters[ITEMS_TO_QUERY];

//    LONG              tmp = 0;
    LONG              doRun = 0;

    UNICODE_STRING    paramPath;

    paramPath.Length = 0;
    paramPath.MaximumLength = RegistryPath->Length +
        (wcslen(PathSuffix)+2)*sizeof(WCHAR);
    paramPath.Buffer = (PUSHORT)ExAllocatePool(NonPagedPool, paramPath.MaximumLength);
    if(!paramPath.Buffer) {
        KdPrint(("CheckRegValue: couldn't allocate paramPath\n"));
        return Default;
    }

    RtlZeroMemory(paramPath.Buffer, paramPath.MaximumLength);
    RtlAppendUnicodeToString(&paramPath, RegistryPath->Buffer);
    RtlAppendUnicodeToString(&paramPath, L"\\");
    RtlAppendUnicodeToString(&paramPath, PathSuffix);

    // Check for the Xxx value.
    RtlZeroMemory(parameters, (sizeof(RTL_QUERY_REGISTRY_TABLE)*ITEMS_TO_QUERY));

    parameters[0].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    parameters[0].Name          = Name;
    parameters[0].EntryContext  = &doRun;
    parameters[0].DefaultType   = REG_DWORD;
    parameters[0].DefaultData   = &zero;
    parameters[0].DefaultLength = sizeof(ULONG);

    status = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
                                    paramPath.Buffer, parameters, NULL, NULL);
    KdPrint(( "CheckRegValue: %ws -> %ws is %#x\n", paramPath.Buffer, Name, doRun));

    ExFreePool(paramPath.Buffer);

    if(!NT_SUCCESS(status)) {
        doRun = Default;
    }

    return doRun;

#undef ITEMS_TO_QUERY

} // end RegCheckParameterValue()


NTSTATUS
RegGetKeyHandle(
    IN HANDLE hRootKey,
    IN PWCHAR KeyName,
    OUT HANDLE* hKey
    )
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING NameString;
    NTSTATUS status;

    RtlInitUnicodeString(
        &NameString,
        KeyName
        );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &NameString,
        OBJ_CASE_INSENSITIVE,
        hRootKey,
        NULL
        );

    status = ZwOpenKey(
                hKey,
                KEY_WRITE | KEY_READ,
                &ObjectAttributes
                );

    if(!NT_SUCCESS(status)) {
        *hKey = NULL;
    }

    return status;
} // end RegGetKeyHandle()

BOOLEAN
RegGetDword(
    IN HANDLE hRootKey,
    IN PWSTR RegistryPath,
    IN PWSTR Name,
    IN PULONG pUlong
    )
{
    UNICODE_STRING NameString;
    PKEY_VALUE_PARTIAL_INFORMATION ValInfo;
    ULONG len;
    NTSTATUS status;
    HANDLE hKey;
    BOOLEAN retval = FALSE;
    BOOLEAN free_h = FALSE;

    if(RegistryPath && RegistryPath[0]) {
        status = RegGetKeyHandle(hRootKey, RegistryPath, &hKey);
        if(!NT_SUCCESS(status))
            return FALSE;
        free_h = TRUE;
    } else {
        hKey = hRootKey;
    }
    if(!hKey)
        return FALSE;

    len = sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(ULONG) + 0x20;
    ValInfo = (PKEY_VALUE_PARTIAL_INFORMATION)
        ExAllocatePool(NonPagedPool, len);
    if(!ValInfo) {
        if(free_h) {
            ZwClose(hKey);
        }
        return FALSE;
    }

    NameString.Buffer = Name;
    NameString.Length = wcslen(Name)*sizeof(WCHAR);
    NameString.MaximumLength = NameString.Length + sizeof(WCHAR);

    status = ZwQueryValueKey(hKey,
                             &NameString,
                             KeyValuePartialInformation,
                             ValInfo,
                             len,
                             &len);
    if(NT_SUCCESS(status) &&
       ValInfo->DataLength == sizeof(ULONG)) {
        RtlCopyMemory(pUlong, ValInfo->Data, sizeof(ULONG));
        retval = TRUE;
    }

    ExFreePool(ValInfo);

    if(free_h) {
        ZwClose(hKey);
    }
    return retval;
} // end RegGetDword()

BOOLEAN
RegGetStringValue(
    IN HANDLE hRootKey,
    IN PWSTR RegistryPath,
    IN PWSTR Name,
    IN PWCHAR pStr,
    IN ULONG MaxLen
    )
{
    UNICODE_STRING NameString;
    PKEY_VALUE_PARTIAL_INFORMATION ValInfo;
    ULONG len;
    NTSTATUS status;
    HANDLE hKey;
    BOOLEAN retval = FALSE;
    BOOLEAN free_h = FALSE;

    if(RegistryPath && RegistryPath[0]) {
        status = RegGetKeyHandle(hRootKey, RegistryPath, &hKey);
        if(!NT_SUCCESS(status))
            return FALSE;
        free_h = TRUE;
    } else {
        hKey = hRootKey;
    }
    if(!hKey)
        return FALSE;

    pStr[0] = 0;

    len = sizeof(KEY_VALUE_PARTIAL_INFORMATION) + MaxLen + 0x20;
    ValInfo = (PKEY_VALUE_PARTIAL_INFORMATION)
        ExAllocatePool(NonPagedPool, len);
    if(!ValInfo) {
        if(free_h) {
            ZwClose(hKey);
        }
        return FALSE;
    }

    NameString.Buffer = Name;
    NameString.Length = wcslen(Name)*sizeof(WCHAR);
    NameString.MaximumLength = NameString.Length + sizeof(WCHAR);

    status = ZwQueryValueKey(hKey,
                             &NameString,
                             KeyValuePartialInformation,
                             ValInfo,
                             len,
                             &len);
    if(NT_SUCCESS(status) &&
       ValInfo->DataLength) {
        RtlCopyMemory(pStr, ValInfo->Data, min(ValInfo->DataLength, MaxLen) );
        if(pStr[ValInfo->DataLength/sizeof(WCHAR)-1]) {
            pStr[ValInfo->DataLength/sizeof(WCHAR)-1] = 0;
        }
        retval = TRUE;
    }

    ExFreePool(ValInfo);

    if(free_h) {
        ZwClose(hKey);
    }
    return retval;
} // end RegGetStringValue()
