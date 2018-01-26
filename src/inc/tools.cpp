#include "stdafx.h"

#ifdef USER_MODE
#include "windows.h"
#endif // USER_MODE

#include "tools.h"

#pragma warning(disable:4035)               // re-enable below

__declspec (naked)
BOOLEAN
__fastcall
EnvGetBit(
    IN PULONG arr, // ECX
    IN ULONG  bit   // EDX
    )
{
//    CheckAddr(arr);
//    ASSERT(bit < 300000);
    __asm {
        push ebx
        push ecx
//        mov  eax,bit
        mov  eax,edx
        shr  eax,3
        and  al,0fch
        add  eax,ecx // eax+arr
        mov  eax,[eax]
        mov  cl,dl
        ror  eax,cl
        and  eax,1

        pop  ecx
        pop  ebx
        ret
    }
} // end EnvGetBit()

__declspec (naked)
BOOLEAN
__fastcall
EnvSetBit(
    IN PULONG arr, // ECX
    IN ULONG  bit   // EDX
    )
{
//    CheckAddr(arr);
//    ASSERT(bit < 300000);
    __asm {
        push ebx
        push ecx
//        mov  eax,bit
        mov  eax,edx
        shr  eax,3
        and  al,0fch
        add  eax,ecx // eax+arr
        mov  ebx,1
        mov  cl,dl
        rol  ebx,cl
        or   [eax],ebx

        pop  ecx
        pop  ebx
        ret
    }
} // end EnvSetBit()

#pragma warning(default:4035)
