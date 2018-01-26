#ifndef __NT_REG_TOOLS__H__
#define __NT_REG_TOOLS__H__

extern "C" {

#include <ntddk.h>

};
#include "stddef.h"
#include "..\inc\tools.h"

ULONG
RegCheckParameterValue(
    IN PUNICODE_STRING RegistryPath,
    IN PWSTR PathSuffix,
    IN PWSTR Name,
    IN ULONG Default
    );

NTSTATUS
RegGetKeyHandle(
    IN HANDLE hRootKey,
    IN PWCHAR KeyName,
    OUT HANDLE* hKey
    );

BOOLEAN
RegGetDword(
    IN HANDLE hRootKey,
    IN PWSTR RegistryPath,
    IN PWSTR Name,
    IN PULONG pUlong
    );

BOOLEAN
RegGetStringValue(
    IN HANDLE hRootKey,
    IN PWSTR RegistryPath,
    IN PWSTR Name,
    IN PWCHAR pStr,
    IN ULONG MaxLen
    );

#endif //__NT_REG_TOOLS__H__