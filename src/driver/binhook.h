#ifndef __HOOK_BIN_PATCH__H__
#define __HOOK_BIN_PATCH__H__

PVOID
HookAtPtr(
    PVOID _ptr,
    PVOID hook_entry
    );

NTSTATUS
UnHookAtPtr(
    PVOID _ctx,
    ULONG timeout // seconds
    );

NTSTATUS
ReleaseHookAtPtr(
    PVOID _ctx
    );

PUCHAR
PtrRelativeToAbs(
    PVOID f_ptr
    );

#define HOOK_JMP_DIRECT(fname) \
{  \
    __asm mov esp,ebp \
    __asm pop ebp \
    __asm jmp [h##fname] \
}

#endif //__HOOK_BIN_PATCH__H__
