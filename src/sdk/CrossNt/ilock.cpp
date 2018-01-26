#include "crossnt.h"

ULONG g_CrNtInterlock = 0;

__declspec(naked)
LONG
__fastcall
CrNtInterlockedIncrement_impl(
    IN OUT PLONG Addend
    )
{
    __asm mov          eax,1
    __asm lock xadd    [ecx], eax              ; exchange add
    __asm ret
} // end CrNtInterlockedIncrement_impl()

__declspec(naked)
LONG
__fastcall
CrNtInterlockedIncrement_impl_i386_MP(
    IN OUT PLONG Addend
    )
{
    __asm push ebx
    __asm pushfd
    __asm cli           ; acquire current CPU

    __asm mov eax,1
    __asm lea ebx,g_CrNtInterlock
retry_lock:
    __asm lock xchg [ebx], eax ; prevent interlocked operations on other CPUs
    __asm or eax,eax
    __asm jz short lock_ok
    __asm pause
    __asm jmp short retry_lock
lock_ok:

    __asm mov          eax,[ecx]              ; get original value
    __asm lock add     [ecx],1

    __asm push eax
    __asm xor  eax,eax
    __asm lock xchg [ebx], eax ; allow interlocked operations on other CPUs
    __asm pop eax

    __asm popfd
    __asm pop ebx
    __asm ret
} // end CrNtInterlockedIncrement_impl_MP()

__declspec(naked)
LONG
__fastcall
CrNtInterlockedIncrement_impl_i386_UP(
    IN OUT PLONG Addend
    )
{
    __asm pushfd
    __asm cli           ; acquire current CPU

    __asm mov          eax,[ecx]              ; get original value
    __asm lock add     [ecx],1

    __asm popfd
    __asm ret
} // end CrNtInterlockedIncrement_impl_UP()

/********************************************************/

__declspec(naked)
LONG
__fastcall
CrNtInterlockedDecrement_impl(
    IN OUT PLONG Addend
    )
{
    __asm mov          eax,-1
    __asm lock xadd    [ecx], eax              ; exchange add
    __asm ret
} // end CrNtInterlockedDecrement_impl()

__declspec(naked)
LONG
__fastcall
CrNtInterlockedDecrement_impl_i386_MP(
    IN OUT PLONG Addend
    )
{
    __asm push ebx
    __asm pushfd
    __asm cli           ; acquire current CPU

    __asm mov eax,1
    __asm lea ebx,g_CrNtInterlock
retry_lock:
    __asm lock xchg [ebx], eax ; prevent interlocked operations on other CPUs
    __asm or eax,eax
    __asm jz short lock_ok
    __asm pause
    __asm jmp short retry_lock
lock_ok:

    __asm mov          eax,[ecx]              ; get original value
    __asm lock add     [ecx],-1

    __asm push eax
    __asm xor  eax,eax
    __asm lock xchg [ebx], eax ; allow interlocked operations on other CPUs
    __asm pop eax

    __asm popfd
    __asm pop ebx
    __asm ret
} // end CrNtInterlockedDecrement_impl_MP()

__declspec(naked)
LONG
__fastcall
CrNtInterlockedDecrement_impl_i386_UP(
    IN OUT PLONG Addend
    )
{
    __asm pushfd
    __asm cli           ; acquire current CPU

    __asm mov          eax,[ecx]              ; get original value
    __asm lock add     [ecx],-1

    __asm popfd
    __asm ret
} // end CrNtInterlockedDecrement_impl_UP()

/********************************************************/

__declspec(naked)
LONG
__fastcall
CrNtInterlockedExchangeAdd_impl(
    IN OUT PLONG Addend,
    IN LONG Increment
    )
{
    __asm mov          eax,edx
    __asm lock xadd    [ecx], eax              ; exchange add
    __asm ret
} // end CrNtInterlockedExchangeAdd_impl()

__declspec(naked)
LONG
__fastcall
CrNtInterlockedExchangeAdd_impl_i386_MP(
    IN OUT PLONG Addend,
    IN LONG Increment
    )
{
    __asm push ebx
    __asm pushfd
    __asm cli           ; acquire current CPU

    __asm mov eax,1
    __asm lea ebx,g_CrNtInterlock
retry_lock:
    __asm lock xchg [ebx], eax ; prevent interlocked operations on other CPUs
    __asm or eax,eax
    __asm jz short lock_ok
    __asm pause
    __asm jmp short retry_lock
lock_ok:

    __asm mov          eax,[ecx]              ; get original value
    __asm lock add     [ecx],edx

    __asm push eax
    __asm xor  eax,eax
    __asm lock xchg [ebx], eax ; allow interlocked operations on other CPUs
    __asm pop eax

    __asm popfd
    __asm pop ebx
    __asm ret
} // end CrNtInterlockedExchangeAdd_impl_MP()

__declspec(naked)
LONG
__fastcall
CrNtInterlockedExchangeAdd_impl_i386_UP(
    IN OUT PLONG Addend,
    IN LONG Increment
    )
{
    __asm pushfd
    __asm cli           ; acquire current CPU

    __asm mov          eax,[ecx]              ; get original value
    __asm lock add     [ecx],edx

    __asm popfd
    __asm ret
} // end CrNtInterlockedExchangeAdd_impl_UP()

/********************************************************/

__declspec(naked)
PVOID
__fastcall
CrNtInterlockedCompareExchange_impl(
    IN OUT PVOID *Destination,
    IN PVOID ExChange,
    IN PVOID Comperand
    )
{
    __asm mov     eax, [esp + 4]               ; set comperand value
    __asm lock cmpxchg [ecx], edx              ; compare and exchange
    __asm ret 4
} // end InterlockedCompareExchange_impl()

__declspec(naked)
PVOID
__fastcall
CrNtInterlockedCompareExchange_impl_i386_MP(
    IN OUT PVOID *Destination,
    IN PVOID ExChange,
    IN PVOID Comperand
    )
{
    __asm push ebx
    __asm pushfd
    __asm cli           ; acquire current CPU

    __asm mov eax,1
    __asm lea ebx,g_CrNtInterlock
retry_lock:
    __asm lock xchg [ebx], eax ; prevent interlocked operations on other CPUs
    __asm or eax,eax
    __asm jz short lock_ok
    __asm pause
    __asm jmp short retry_lock
lock_ok:

    __asm mov  eax, [esp + 4]               
    __asm cmp  eax, [ecx]
    __asm jne  skip
    __asm lock xchg  [ecx], edx
    __asm jmp short quit
skip: 
    __asm mov eax, [ecx]                  
quit:

    __asm push eax
    __asm xor  eax,eax
    __asm lock xchg [ebx], eax ; allow interlocked operations on other CPUs
    __asm pop eax

    __asm popfd
    __asm pop ebx
    __asm ret 4
} // end InterlockedCompareExchange_impl_i386_MP()

__declspec(naked)
PVOID
__fastcall
CrNtInterlockedCompareExchange_impl_i386_UP(
    IN OUT PVOID *Destination,
    IN PVOID ExChange,
    IN PVOID Comperand
    )
{
    __asm pushfd
    __asm cli           ; acquire current CPU

    __asm mov  eax, [esp + 4]               
    __asm cmp  eax, [ecx]
    __asm jne  skip
    __asm mov  [ecx], edx
    __asm jmp short quit
skip: 
    __asm mov eax, [ecx]                  
quit:
    __asm popfd
    __asm ret 4
} // end InterlockedCompareExchange_impl_i386_UP()
