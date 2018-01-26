//----------------

#include "crossnt.h"

extern "C" ptrMOV_DD_SWP _MOV_DD_SWP = NULL;
extern "C" ptrMOV_QD_SWP _MOV_QD_SWP = NULL;

__declspec (naked)
void
__fastcall
_MOV_DD_SWP_i486(
    void* a, // ECX
    void* b  // EDX
    )
{
  _asm {
    mov   eax,[edx]
    bswap eax
    mov   [ecx],eax
    ret
  }
}

__declspec (naked)
void
__fastcall
_MOV_DD_SWP_i386(
    void* a, // ECX
    void* b  // EDX
    )
{
  _asm {
    mov   eax,[edx]
    mov   [ecx+3],al
    rol   eax,8
    mov   [ecx],al
    rol   eax,8
    mov   [ecx+1],al
    rol   eax,8
    mov   [ecx+2],al
    ret
  }
}

__declspec (naked)
void
__fastcall
_MOV_QD_SWP_i486(
    void* a, // ECX
    void* b  // EDX
    )
{
  _asm {
    mov   eax,[edx]
    bswap eax
    mov   [ecx+4],eax
    mov   eax,[edx+4]
    bswap eax
    mov   [ecx],eax
    ret
  }
}

__declspec (naked)
void
__fastcall
_MOV_QD_SWP_i386(
    void* a, // ECX
    void* b  // EDX
    )
{
  _asm {
    mov   eax,[edx]
    mov   [ecx+7],al
    rol   eax,8
    mov   [ecx+4],al
    rol   eax,8
    mov   [ecx+5],al
    rol   eax,8
    mov   [ecx+6],al
    mov   eax,[edx+4]
    mov   [ecx+4],al
    rol   eax,8
    mov   [ecx],al
    rol   eax,8
    mov   [ecx+1],al
    rol   eax,8
    mov   [ecx+2],al
    ret
  }
}

extern "C"
__declspec (naked)
void
__fastcall
_MOV_DW_SWP(
    void* a, // ECX
    void* b  // EDX
    )
{
  _asm {
    mov   ax,[edx]
    rol   ax,8
    mov   [ecx],ax
    ret
  }
}

extern "C" ptrREVERSE_DD _REVERSE_DD = NULL;

__declspec (naked)
void
__fastcall
_REVERSE_DD_i486(
    void* a  // ECX
    )
{
  _asm {
    mov   eax,[ecx]
    bswap eax
    mov   [ecx],eax
    ret
  }
}

__declspec (naked)
void
__fastcall
_REVERSE_DD_i386(
    void* a  // ECX
    )
{
  _asm {
    mov   al,[ecx]
    rol   eax,8
    mov   al,[ecx+1]
    rol   eax,8
    mov   al,[ecx+2]
    rol   eax,8
    mov   al,[ecx+3]
    mov   [ecx],eax
    ret
  }
}

extern "C" 
__declspec (naked)
void
__fastcall
_REVERSE_DW(
    void* a  // ECX
    )
{
  _asm {
    mov   ax,[ecx]
    rol   ax,8
    mov   [ecx],ax
    ret
  }
}

extern "C" 
__declspec (naked)
void
__fastcall
_MOV_DW2DD_SWP(
    void* a, // ECX
    void* b  // EDX
    )
{
  _asm {
    mov   ax,[edx]
    rol   ax,8
    mov   [ecx+2],ax
    mov   [ecx],0
    ret
  }
}

extern "C" 
__declspec (naked)
void
__fastcall
_MOV_SWP_DW2DD(
    void* a, // ECX
    void* b  // EDX
    )
{
  _asm {
    xor   eax,eax
    mov   ax,[edx]
    rol   ax,8
    mov   [ecx],eax
    ret
  }
}

extern "C" 
__declspec (naked)
void
__fastcall
_MOV_MSF(
    void* a, // ECX
    void* b  // EDX
    )
{
  _asm {
    mov   eax,[edx]
    mov   [ecx],ax
    shr   eax,16
    mov   [ecx+2],al
    ret
  }
}

extern "C" ptrMOV_MSF_SWP _MOV_MSF_SWP = NULL;

__declspec (naked)
void
__fastcall
_MOV_MSF_SWP_i486(
    void* a, // ECX
    void* b  // EDX
    )
{
  _asm {
    mov   eax,[edx]
    mov   [ecx+2],al
    bswap eax
    shr   eax,8
    mov   [ecx],ax
    ret
  }
}

__declspec (naked)
void
__fastcall
_MOV_MSF_SWP_i386(
    void* a, // ECX
    void* b  // EDX
    )
{
  _asm {
    mov   eax,[edx]
    mov   [ecx+2],al
    shr   eax,8
    mov   [ecx+1],al
    shr   eax,8
    mov   [ecx],al
    ret
  }
}

extern "C" 
__declspec (naked)
void
__fastcall
_XCHG_DD(
    void* a, // ECX
    void* b  // EDX
    )
{
  _asm {
    mov   eax,[edx]
    xchg  eax,[ecx]
    mov   [edx],eax
    ret
  }
}


#pragma warning(push)
#pragma warning(disable:4035)               // re-enable below

__declspec (naked)
BOOLEAN
__fastcall
_GetBit(
    IN ULONG* arr, // ECX
    IN ULONG bit   // EDX
    )
{
    __asm {
        push edx
//        push ecx
//        mov  eax,bit
        mov  eax,edx
        shr  eax,3
        and  al,0fch
        add  eax,ecx // eax+arr
/*
        mov  eax,[eax]
        mov  cl,dl
        ror  eax,cl
        and  eax,1
*/
        and  dx,0x1f
        bt   dword ptr [eax],dx
        xor  eax,eax
        setc al

//        pop  ecx
        pop  edx
        ret
    }
} // end _GetBit()

__declspec (naked)
BOOLEAN
__fastcall
_GetBit_i386(
    IN ULONG* arr, // ECX
    IN ULONG bit   // EDX
    )
{
    __asm {
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
/*
        bt   [dword eax],dl
        setc
*/
        pop  ecx
        ret
    }
} // end _GetBit_i386()

__declspec (naked)
BOOLEAN
__fastcall
_SetBit(
    IN ULONG* arr, // ECX
    IN ULONG bit   // EDX
    )
{
    __asm {
        push edx
//        push ecx
//        mov  eax,bit
        mov  eax,edx
        shr  eax,3
        and  al,0fch
        add  eax,ecx // eax+arr
/*
        mov  ebx,1
        mov  cl,dl
        rol  ebx,cl
        or   [eax],ebx
*/
        and  dx,0x1f
        bts  dword ptr [eax],dx

//        pop  ecx
        pop  edx
        ret
    }
} // end _SetBit()

__declspec (naked)
BOOLEAN
__fastcall
_SetBit_i386(
    IN ULONG* arr, // ECX
    IN ULONG bit   // EDX
    )
{
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
/*
        bts   [dword eax],dl
*/
        pop  ecx
        pop  ebx
        ret
    }
} // end _SetBit_i386()


#pragma warning(pop)

