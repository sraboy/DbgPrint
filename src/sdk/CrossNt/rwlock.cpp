
#include "crossnt.h"

VOID
CrNtNdisInitializeReadWriteLock_impl(
    IN PNDIS_RW_LOCK Lock
    )
{
    RtlZeroMemory(Lock, sizeof(*Lock));
} // end CrNtNdisInitializeReadWriteLock_impl()


VOID
CrNtNdisAcquireReadWriteLock_impl(
    IN PNDIS_RW_LOCK Lock,
    IN BOOLEAN       fWrite,
    IN PLOCK_STATE   LockState
    )
{
    ULONG n, m, i, Ref;
/*
    // will raise assert if LockState is not initialized with zeros,
    // however, this is useful check for debugging.
    ASSERT(LockState->LockState == RWLOCK_STATE_FREE ||
           LockState->LockState == RWLOCK_STATE_RELEASED);
*/
    if(fWrite)
    {
       if(Lock->Context==KeGetCurrentThread())
       {
           // recursive acquisition from same thread (LockState must be different!)
           LockState->LockState=RWLOCK_STATE_RECURSIVE;
           return;
       }

       KeAcquireSpinLock(&Lock->SpinLock, &LockState->OldIrql);
       n=KeGetCurrentProcessorNumber();
       Ref=Lock->RefCount[n].RefCount; // save reference count for current CPU
       Lock->RefCount[n].RefCount=0;
       m = *g_KeNumberProcessors; // number of system processors

       i=0;
       do 
       {
           while(Lock->RefCount[i].RefCount==0)
           {
               i++;
               m--;
               if(m==0) // release lock (all CPUs have RefCount==0)
               {
                   Lock->RefCount[n].RefCount=Ref; // restore reference count for current CPU
                   Lock->Context=KeGetCurrentThread();
                   LockState->LockState=RWLOCK_STATE_WRITE_ACQUIRED;
                   return;
               }
           }
           for(volatile ULONG j=0;j<50;j++); // do nothing 50 times
       }while(TRUE);
       // never get here

    }
    else // read
    {
        LockState->OldIrql=KeGetCurrentIrql();
        if(LockState->OldIrql<DISPATCH_LEVEL) {
            CrNtKeRaiseIrqlToDpcLevel();
        }
        n=KeGetCurrentProcessorNumber();
        Ref=InterlockedIncrement((PLONG)&Lock->RefCount[n].RefCount);
        if(Lock->SpinLock && Ref==1 && Lock->Context!=KeGetCurrentThread())
        {
            InterlockedDecrement((PLONG)&Lock->RefCount[n].RefCount);
            KeAcquireSpinLockAtDpcLevel(&Lock->SpinLock);
            InterlockedIncrement((PLONG)&Lock->RefCount[n].RefCount);
            KeReleaseSpinLockFromDpcLevel(&Lock->SpinLock);
        }
        LockState->LockState=RWLOCK_STATE_READ_ACQUIRED;
        return;
    }
} // end CrNtNdisAcquireReadWriteLock_impl()

VOID
CrNtNdisReleaseReadWriteLock_impl(
    IN PNDIS_RW_LOCK Lock,
    IN PLOCK_STATE   LockState
    )
{
    switch(LockState->LockState) {
    case RWLOCK_STATE_READ_ACQUIRED:
        InterlockedDecrement((PLONG)&Lock->RefCount[KeGetCurrentProcessorNumber()].RefCount);
        LockState->LockState=RWLOCK_STATE_RELEASED;
        if(LockState->OldIrql<DISPATCH_LEVEL) {
            KeLowerIrql(LockState->OldIrql);
        }
        return;

    case RWLOCK_STATE_WRITE_ACQUIRED:
        LockState->LockState=RWLOCK_STATE_RELEASED;
        Lock->Context=0;
        KeReleaseSpinLock(&Lock->SpinLock, LockState->OldIrql);

    case RWLOCK_STATE_RECURSIVE:
        // do nothing
        return;

    default:
        ASSERT(FALSE);
        return;
    } // end switch(LockState->LockState)
} // end CrNtNdisReleaseReadWriteLock_impl()
