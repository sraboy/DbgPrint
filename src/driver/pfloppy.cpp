#include    "pfloppy.h"

COMMAND_TABLE CommandTable[] = {

    { 0, 0, 0,  FALSE, FALSE },            // 00 not implemented
    { 0, 0, 0,  FALSE, FALSE },            // 01 not implemented
    { 8, 1, 7,  TRUE,  TRUE  },            // 02 read track
    { 2, 0, 0,  FALSE, TRUE  },            // 03 specify
    { 1, 0, 1,  FALSE, TRUE  },            // 04 sense drive status
    { 8, 1, 7,  TRUE,  TRUE  },            // 05 write
    { 8, 1, 7,  TRUE,  TRUE  },            // 06 read
    { 1, 0, 2,  TRUE,  TRUE  },            // 07 recalibrate
    { 0, 0, 2,  FALSE, TRUE  },            // 08 sense interrupt status
    { 0, 0, 0,  FALSE, FALSE },            // 09 not implemented
    { 1, 1, 7,  TRUE,  TRUE  },            // 0a read id
    { 0, 0, 0,  FALSE, FALSE },            // 0b not implemented
    { 0, 0, 0,  FALSE, FALSE },            // 0c not implemented
    { 5, 1, 7,  TRUE,  TRUE  },            // 0d format track
    { 0, 0, 10, FALSE, FALSE },            // 0e dump registers
    { 2, 0, 2,  TRUE,  TRUE  },            // 0f seek
    { 0, 0, 1,  FALSE, FALSE },            // 10 version
    { 0, 0, 0,  FALSE, FALSE },            // 11 not implemented
    { 1, 0, 0,  FALSE, FALSE },            // 12 perpendicular mode
    { 3, 0, 0,  FALSE, FALSE },            // 13 configure
    { 0, 0, 0,  FALSE, FALSE },            // 14 not implemented
    { 0, 0, 0,  FALSE, FALSE },            // 15 not implemented
    { 8, 1, 7,  TRUE,  FALSE },            // 16 verify
    { 1, 0, 1,  FALSE, FALSE },            // 17 Powerdown Mode
    { 0, 0, 1,  FALSE, FALSE }             // 18 Part ID
};

VOID
DbgFlopInit(
    PFLOPPYDRV hFloppy,
    ULONG DriveId,
    PDISK_GEOMETRY Geom
    )
{
} // end DbgFlopInit()

/*++

Routine Description:

    This routine is called to wait for interrupt.

Arguments:

    ControllerData - a pointer to our data area for this controller.

Return Value:

    STATUS_SUCCESS if controller is ready for data transfer;
    STATUS_TIMEOUT otherwise.

--*/
NTSTATUS
FlWaitDRQ(
    IN PFLOPPYDRV ControllerData,
    IN UCHAR      Direction // 0 - unspecified, or STATUS_xxx_READY
    )
{
    ULONG i = 0;
    ULONG mask;

    if(!Direction) {
        mask = STATUS_DATA_REQUEST;
        Direction = STATUS_DATA_REQUEST;
    } else {
        mask = STATUS_IO_READY_MASK;
    }

    // Sit in a tight loop for a while.  If the controller becomes ready,
    // read the byte.

    do {
        if((READ_CONTROLLER(&ControllerData->Port->Status) & mask) == Direction) {
            return STATUS_SUCCESS;
        }
        i++;

    } while (i < FIFO_TIGHTLOOP_RETRY_COUNT);

    // We hope that in most cases the FIFO will become ready very quickly
    // and the above loop will have read the byte.  But if the FIFO
    // is not yet ready, we'll loop a few times delaying for 10ms and then
    // trying it again.

    i = 0;

    while(i < FIFO_DELAY_RETRY_COUNT) {

        KdPrint(
            ("Floppy: waiting for 10ms for controller data ready\n")
            );

        KeStallExecutionProcessor(10*1000); // 10ms

        i++;

        if((READ_CONTROLLER(&ControllerData->Port->Status) & mask) == Direction) {
            return STATUS_SUCCESS;
        }
    }

    return STATUS_TIMEOUT;

} // end FlWaitDRQ()


/*++

Routine Description:

    This routine is called to send a byte to the controller.  It won't
    send the byte unless the controller is ready to receive a byte; if
    it's not ready after checking FIFO_TIGHTLOOP_RETRY_COUNT times, we
    delay for the minimum possible time (10ms) and then try again.  It
    should always be ready after waiting 10ms.

Arguments:

    ByteToSend - the byte to send to the controller.

    ControllerData - a pointer to our data area for this controller.

Return Value:

    STATUS_SUCCESS if the byte was sent to the controller;
    STATUS_DEVICE_NOT_READY otherwise.

--*/
NTSTATUS
FlSendByte(
    IN UCHAR ByteToSend,
    IN PFLOPPYDRV ControllerData
    )
{
    if(!NT_SUCCESS(FlWaitDRQ(ControllerData, STATUS_WRITE_READY))) {
        return STATUS_DEVICE_NOT_READY;
    }

    WRITE_CONTROLLER(
        &ControllerData->Port->Fifo,
        ByteToSend );


    return STATUS_SUCCESS;

} // end FlSendByte()


/*++

Routine Description:

    This routine is called to get a byte from the controller.  It won't
    read the byte unless the controller is ready to send a byte; if
    it's not ready after checking FIFO_RETRY_COUNT times, we delay for
    the minimum possible time (10ms) and then try again.  It should
    always be ready after waiting 10ms.

Arguments:

    ByteToGet - the address in which the byte read from the controller
    is stored.

    ControllerData - a pointer to our data area for this controller.

Return Value:

    STATUS_SUCCESS if a byte was read from the controller;
    STATUS_DEVICE_NOT_READY otherwise.

--*/
NTSTATUS
FlGetByte(
    OUT PUCHAR ByteToGet,
    IN PFLOPPYDRV ControllerData
    )
{
    if(!NT_SUCCESS(FlWaitDRQ(ControllerData, STATUS_READ_READY))) {
        return STATUS_DEVICE_NOT_READY;
    }

    *ByteToGet = READ_CONTROLLER(
        &ControllerData->Port->Fifo );

    return STATUS_SUCCESS;

} // end FlGetByte()


/*++

Routine Description:

    This routine is called when the floppy controller returns an error.
    Status registers 1 and 2 are passed in, and this returns an appropriate
    error status.

Arguments:

    StatusRegister1 - the controller's status register #1.

    StatusRegister2 - the controller's status register #2.

Return Value:

    An NTSTATUS error determined from the status registers.

--*/
NTSTATUS
FlInterpretError(
    IN UCHAR StatusRegister1,
    IN UCHAR StatusRegister2
    )
{
    if ( ( StatusRegister1 & STREG1_CRC_ERROR ) ||
        ( StatusRegister2 & STREG2_CRC_ERROR ) ) {

        KdPrint(
            ("FlInterpretError: STATUS_CRC_ERROR\n")
            );
        return STATUS_CRC_ERROR;
    }

    if ( StatusRegister1 & STREG1_DATA_OVERRUN ) {

        KdPrint(
            ("FlInterpretError: STATUS_DATA_OVERRUN\n")
            );
        return STATUS_DATA_OVERRUN;
    }

    if ( ( StatusRegister1 & STREG1_SECTOR_NOT_FOUND ) ||
        ( StatusRegister1 & STREG1_END_OF_DISKETTE ) ) {

        KdPrint(
            ("FlInterpretError: STATUS_NONEXISTENT_SECTOR\n")
            );
        return STATUS_NONEXISTENT_SECTOR;
    }

    if ( ( StatusRegister2 & STREG2_DATA_NOT_FOUND ) ||
        ( StatusRegister2 & STREG2_BAD_CYLINDER ) ||
        ( StatusRegister2 & STREG2_DELETED_DATA ) ) {

        KdPrint(
            ("FlInterpretError: STATUS_DEVICE_DATA_ERROR\n")
            );
        return STATUS_DEVICE_DATA_ERROR;
    }

    if ( StatusRegister1 & STREG1_WRITE_PROTECTED ) {

        KdPrint(
            ("FlInterpretError: STATUS_MEDIA_WRITE_PROTECTED\n")
            );
        return STATUS_MEDIA_WRITE_PROTECTED;
    }

    if ( StatusRegister1 & STREG1_ID_NOT_FOUND ) {

        KdPrint(
            ("FlInterpretError: STATUS_FLOPPY_ID_MARK_NOT_FOUND\n")
            );
        return STATUS_FLOPPY_ID_MARK_NOT_FOUND;

    }

    if ( StatusRegister2 & STREG2_WRONG_CYLINDER ) {

        KdPrint(
            ("FlInterpretError: STATUS_FLOPPY_WRONG_CYLINDER\n")
            );
        return STATUS_FLOPPY_WRONG_CYLINDER;

    }

    // There's other error bits, but no good status values to map them
    // to.  Just return a generic one.

    KdPrint(
        ("FlInterpretError: STATUS_FLOPPY_UNKNOWN_ERROR\n")
        );
    return STATUS_FLOPPY_UNKNOWN_ERROR;
} // end FlInterpretError()

/*++

Routine Description:

    This routine sends the command and all parameters to the controller,
    waits for the command to interrupt if necessary, and reads the result
    bytes from the controller, if any.

    Before calling this routine, the caller should put the parameters for
    the command in ControllerData->FifoBuffer[].  The result bytes will
    be returned in the same place.

    This routine runs off the CommandTable.  For each command, this says
    how many parameters there are, whether or not there is an interrupt
    to wait for, and how many result bytes there are.  Note that commands
    without result bytes actually have two, since the ISR will issue a
    SENSE INTERRUPT STATUS command on their behalf.

Arguments:

    Command - a byte specifying the command to be sent to the controller.

    DisketteExtension - a pointer to our data area for the drive being
    accessed (any drive if a controller command is being given).

Return Value:

    STATUS_SUCCESS if the command was sent and bytes received properly;
    appropriate error propogated otherwise.

--*/
NTSTATUS
FlIssueCommand(
    IN UCHAR Command,
    IN OUT PFLOPPYDRV controllerData,
    IN OUT PUCHAR     buffer,
    IN ULONG          length,
    IN BOOLEAN        read_direction
    )
{
    NTSTATUS ntStatus;
    NTSTATUS ntStatus2;
    UCHAR i;
    BOOLEAN CommandHasResultPhase = FALSE;

    KdPrint(
        ("Floppy: FloppyIssueCommand %2x...\n", Command)
        );

    // If this command causes an interrupt, set CurrentDeviceObject and
    // reset the interrupt event.
    if ( CommandTable[Command & COMMAND_MASK].InterruptExpected ) {

        CommandHasResultPhase =
            !!CommandTable[Command & COMMAND_MASK].FirstResultByte;
    }

    // Send the command to the controller.
    ntStatus = FlSendByte( Command, controllerData );

    // If the command was successfully sent, we can proceed.
    if(NT_SUCCESS(ntStatus)) {

        // Send the parameters as long as we succeed.
        for ( i = 0;
            ( i < CommandTable[Command & COMMAND_MASK].NumberOfParameters ) &&
                ( NT_SUCCESS( ntStatus ) );
            i++ ) {

            ntStatus = FlSendByte(
                controllerData->FifoBuffer[i],
                controllerData );
        }

        if(NT_SUCCESS(ntStatus)) {

            // If there is an interrupt, wait for it.
            if ( CommandTable[Command & COMMAND_MASK].InterruptExpected ) {

/*
                ntStatus = KeWaitForSingleObject(
                    &controllerData->InterruptEvent,
                    Executive,
                    KernelMode,
                    FALSE,
                    &controllerData->InterruptDelay );
*/
                //ntStatus = FlWaitDRQ(controllerData, );
                while(length) {
                    if(read_direction) {
                        ntStatus = FlGetByte(
                            buffer,
                            controllerData );
                    } else {
                        ntStatus = FlSendByte(
                            *buffer,
                            controllerData );
                    }
                    if(!NT_SUCCESS(ntStatus)) {
                        break;
                    }
                    buffer++;
                    length--;
                }
            }

            // If successful so far, get the result bytes.
            if(NT_SUCCESS(ntStatus)) {

                for ( i = CommandTable[Command & COMMAND_MASK].FirstResultByte;
                    ( i < CommandTable[Command & COMMAND_MASK].
                            NumberOfResultBytes ) && ( NT_SUCCESS( ntStatus ) );
                    i++ ) {

                    ntStatus = FlGetByte(
                        &controllerData->FifoBuffer[i],
                        controllerData );
                }
            } else {
                KdPrint(
                    ("FlIssueCommand: failure after issue %x\n", ntStatus)
                    );
            }
        }
    }

    // If there was a problem, check to see if it was caused by an
    // unimplemented command.

    if(!NT_SUCCESS(ntStatus)) {

        if ( (i==1) &&
            ( !CommandTable[Command & COMMAND_MASK].AlwaysImplemented ) ) {

            // This error is probably caused by a command that's not
            // implemented on this controller.  Read the error from the
            // controller, and we should be in a stable state.

            ntStatus2 = FlGetByte(
                &controllerData->FifoBuffer[0],
                controllerData );

            // If GetByte went as planned, we'll return the original error.
            if ( NT_SUCCESS( ntStatus2 ) ) {

                if ( controllerData->FifoBuffer[0] !=
                    STREG0_END_INVALID_COMMAND ) {

                    // Status isn't as we expect, so return generic error.
                    ntStatus = STATUS_FLOPPY_BAD_REGISTERS;

                    KdPrint(
                        ("FlIssueCommand: unexpected error value %2x\n",
                         controllerData->FifoBuffer[0])
                        );
                } else {
                    KdPrint(
                        ("FlIssueCommand: Invalid command error returned\n")
                        );
                }

            } else {

                // GetByte returned an error, so propogate THAT.
                KdPrint(
                    ("FlIssueCommand: FlGetByte returned error %x\n", ntStatus2)
                    );
                ntStatus = ntStatus2;
            }
        }
    }

    if ( !NT_SUCCESS( ntStatus ) ) {

        // Print an error message unless the command isn't always
        // implemented, ie CONFIGURE.
        if ( !( ( ntStatus == STATUS_DEVICE_NOT_READY ) &&
            ( !CommandTable[Command & COMMAND_MASK].AlwaysImplemented ) ) ) {

            KdPrint(
                ("Floppy: err %x "
                 "------  while giving command %x\n",
                 ntStatus,
                 Command)
                );
        }
    }

    return ntStatus;
} // end FlIssueCommand()
