#include    "pserial.h"

extern "C" {

extern "C"
BOOLEAN
HalQueryRealTimeClock (
    OUT PTIME_FIELDS TimeFields
    );

};

UCHAR
DbgSerReadLsr (
    PCOMPORT hPort,
    UCHAR);

ULONG DbgSerStdPortAddr[] = { 0, 
    0x3f8, 0x2f8, 0x3e8, 0x2e8 };

VOID
DbgSerInit (
    PCOMPORT         hPort,
    PCOM_PORT_LAYOUT PortAddr,
    ULONG            Rate
    )
{
    if (!PortAddr) {
        return;
    }
    if ((ULONG)PortAddr <= 4) {
        PortAddr = (PCOM_PORT_LAYOUT)(DbgSerStdPortAddr[(ULONG)PortAddr]);
    }

    hPort->Baud = 0; // will be initialized below
    hPort->PortAddr = PortAddr;
    hPort->LastLsr = 0;
    hPort->LastMsr = 0;
    hPort->TimeOut = 1024*256;
    hPort->Delay   = 0;

    DbgSerSetBaud(hPort, Rate);

    // Assert DTR, RTS.

    WRITE_PORT_UCHAR(&PortAddr->ModemCtl, SERIAL_MCR_DTR | SERIAL_MCR_RTS);
    WRITE_PORT_UCHAR(&PortAddr->Intr, 0);

    return;
} // end DbgSerInit()


VOID
DbgSerSetBaud(
    PCOMPORT  hPort,
    ULONG     Rate
    )
{
    PCOM_PORT_LAYOUT PortAddr;
    ULONG   divisor;
    UCHAR   lcr;

    // compute the divsor
    divisor = UART_CLOCK / Rate;

    PortAddr = hPort->PortAddr;

    // set divisor latch access bit (DLAB) in LCR
    lcr = READ_PORT_UCHAR(&PortAddr->LineCtl);
    lcr |= SERIAL_LCR_DLAB;
    WRITE_PORT_UCHAR(&PortAddr->LineCtl, lcr);

    // set the divisor latch value.
    WRITE_PORT_UCHAR(&PortAddr->Div[1], (UCHAR)((divisor >> 8) & 0xff));
    WRITE_PORT_UCHAR(&PortAddr->Div[0], (UCHAR)(divisor & 0xff));

    // Set transfer mode.
    WRITE_PORT_UCHAR(&PortAddr->LineCtl, SERIAL_8_DATA | SERIAL_1_STOP | SERIAL_NONE_PARITY);

    // Remember the baud rate
    hPort->Baud = Rate;
} // end DbgSerSetBaud()


VOID
DbgSerSendModemString(
    PCOMPORT    hPort,
    IN PUCHAR   String
    )
{
    TIME_FIELDS     Time;
    UCHAR   i;
    ULONG   l;

    if(hPort->Flags & COMPORT_SENDSTRING)
        return ;

    hPort->Flags |= COMPORT_SENDSTRING;
    if(!hPort->Delay) {
        // calibrate on 1 second delay
        HalQueryRealTimeClock (&Time);
        l = Time.Second;
        while(l == (ULONG) Time.Second) {
            DbgSerReadLsr(hPort, 0);
            HalQueryRealTimeClock (&Time);
            hPort->Delay++;
        }
        hPort->Delay /= 3;
    }

    l = hPort->Delay;
    while(*String) {
        HalQueryRealTimeClock (&Time);
        i = DbgSerReadLsr (hPort, 0);
        if(i & SERIAL_LSR_THRE) {
            if((--l) == 0) {
                WRITE_PORT_UCHAR(&hPort->PortAddr->Data, *String);
                String++;
                l = hPort->Delay;
            }
        }
        if(i & SERIAL_LSR_DR) {
            READ_PORT_UCHAR(&hPort->PortAddr->Data);
        }
    }
    hPort->Flags &= ~COMPORT_SENDSTRING;
} // end DbgSerSendModemString()

static const UCHAR ModemString_AT[] = "\n\rAT\n\r";

UCHAR
DbgSerReadLsr(
    PCOMPORT hPort,
    UCHAR    waiting
    )
{
    UCHAR   ringflag = 0;
    TIME_FIELDS Time;
    UCHAR   lsr, msr;

    lsr = READ_PORT_UCHAR(&hPort->PortAddr->LineStat);

    if(lsr & waiting) {
        hPort->LastLsr = ~SERIAL_LSR_DR | (lsr & SERIAL_LSR_DR);
        return lsr;
    }

    msr = READ_PORT_UCHAR (&hPort->PortAddr->ModemStat);

    if(!(hPort->Flags & COMPORT_USINGMODEM)) {
        return lsr;
    }

    if(msr & SERIAL_MSR_DCD) {

        // In modem control mode with carrier detect
        // Reset carrier lost time
        hPort->Flags |= COMPORT_CARRIEROK | COMPORT_WASCARRIER;

    } else {

        // In modem control mode, but no carrier detect.  After
        // 60 seconds drop out of modem control mode
        if(hPort->Flags & COMPORT_CARRIEROK) {
            HalQueryRealTimeClock (&hPort->CarrierLostTime);
            hPort->Flags &= ~COMPORT_CARRIEROK;
            ringflag = 0;
        }

        HalQueryRealTimeClock (&Time);
        if(Time.Minute != hPort->CarrierLostTime.Minute  &&
            Time.Second >= hPort->CarrierLostTime.Second) {

            // It's been at least 60 seconds - drop out of
            // modem control mode until next RI
            hPort->Flags &= ~COMPORT_USINGMODEM;
            DbgSerSendModemString (hPort, (PUCHAR)ModemString_AT);
        }

        if(hPort->Flags & COMPORT_WASCARRIER) {

            // We had a connection - if it's the connection has been
            // down for a few seconds, then send a string to the modem
            if(Time.Second < hPort->CarrierLostTime.Second)
                Time.Second += 60;

            if(Time.Second > hPort->CarrierLostTime.Second + 10) {
                hPort->Flags &= ~COMPORT_WASCARRIER;
                DbgSerSendModemString (hPort, (PUCHAR)ModemString_AT);
            }
        }
    }

    return lsr;
} // end DbgSerReadLsr()


VOID
DbgSerWriteByte(
    PCOMPORT  hPort,
    UCHAR     Byte
    )
{
    UCHAR   msr, lsr;

    // If modem is used, check if DSR, CTS and CD are all set before sending data.
    while(hPort->Flags & COMPORT_USINGMODEM) {
        msr = READ_PORT_UCHAR(&hPort->PortAddr->ModemStat);
        if((msr & (SERIAL_MSR_DCD | SERIAL_MSR_CTS | SERIAL_MSR_DSR)) ==
               (SERIAL_MSR_DCD | SERIAL_MSR_CTS | SERIAL_MSR_DSR)) {
            break;
        }

        // If no CD, and there's a charactor ready, get it
        lsr = DbgSerReadLsr (hPort, 0);
        if(!(msr & SERIAL_MSR_DCD) && ((lsr & SERIAL_LSR_DR) == SERIAL_LSR_DR)) {
            READ_PORT_UCHAR(&hPort->PortAddr->Data);
        }
    }

    //  Wait for port busy release
    while(!(DbgSerReadLsr(hPort, SERIAL_LSR_THRE) & SERIAL_LSR_THRE));

    // Send the byte
    WRITE_PORT_UCHAR(&hPort->PortAddr->Data, Byte);
} // end DbgSerWriteByte()


USHORT
DbgSerReadByte (
    PCOMPORT  hPort,
    PUCHAR    Byte,
    BOOLEAN   WaitForByte
    )
{
    UCHAR   lsr;
    UCHAR   val;
    ULONG   lim;

    // Ensure DTR and CTS are set

    // Check if COMPORT is inited
    if(!hPort->PortAddr) {
        return(COMPORT_NODATA);
    }

    
    for(lim = WaitForByte ? hPort->TimeOut : 1; lim; lim--) {

        lsr = DbgSerReadLsr(hPort, SERIAL_LSR_DR);
        if((lsr & SERIAL_LSR_DR) == SERIAL_LSR_DR) {

            // Check for errors
            if(lsr & (SERIAL_LSR_FE | SERIAL_LSR_PE | SERIAL_LSR_OE)) {
                *Byte = 0;
                return(COMPORT_ERROR);
            }

            // read byte
            val = READ_PORT_UCHAR(&hPort->PortAddr->Data);

            if(hPort->Flags & COMPORT_USINGMODEM) {

                // Using modem. If no CD, skip this byte.
                if((READ_PORT_UCHAR(&hPort->PortAddr->ModemStat) & SERIAL_MSR_DCD) == 0) {
                    continue;
                }
            }

            *Byte = val & (UCHAR)0xff;
            return COMPORT_SUCCESS;
        }
    }

    hPort->LastLsr = 0;
    DbgSerReadLsr (hPort, 0);
    return COMPORT_NODATA;
} // end DbgSerReadByte()


BOOLEAN
DbgSerProbePort(
    IN PCOM_PORT_LAYOUT PortAddr
    )
{
    UCHAR OldModemStatus;
    UCHAR ModemStatus;
    BOOLEAN retval = TRUE;

    // Save previous value of MCR.
    OldModemStatus = READ_PORT_UCHAR(&PortAddr->ModemCtl);

    // Set the port into diagnostic mode.
    WRITE_PORT_UCHAR(
        &PortAddr->ModemCtl,
        SERIAL_MCR_LOOP
        );

    // Bang on it again to make sure that all the lower bits
    // are clear.
    WRITE_PORT_UCHAR(
        &PortAddr->ModemCtl,
        SERIAL_MCR_LOOP
        );

    // Read the modem status register.  The high for bits should
    // be clear.
    ModemStatus = READ_PORT_UCHAR(&PortAddr->ModemStat);

    if(ModemStatus & (SERIAL_MSR_CTS | SERIAL_MSR_DSR |
                       SERIAL_MSR_RI  | SERIAL_MSR_DCD)) {

        retval = FALSE;
        goto exit;
    }

    // Ok, turn on OUT1 in MCR
    // and this should turn on ring indicator in MSR.
    WRITE_PORT_UCHAR(
        &PortAddr->ModemCtl,
        (SERIAL_MCR_OUT1 | SERIAL_MCR_LOOP)
        );

    ModemStatus = READ_PORT_UCHAR(&PortAddr->ModemStat);

    if(!(ModemStatus & SERIAL_MSR_RI)) {
        retval = FALSE;
    }

exit:

    // Put the modem control back into a clean state.
    WRITE_PORT_UCHAR(
        &PortAddr->ModemCtl,
        OldModemStatus
        );

    return retval;
} // end DbgSerProbePort()

