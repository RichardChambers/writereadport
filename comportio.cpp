#include "StdAfx.h"
#include "comportio.h"

comportio::comportio(void) :
	m_dwError(0),
    m_hHandle(INVALID_HANDLE_VALUE),
	m_dwBytesWritten(0),
	m_dwBytesRead(0),
	m_dwModem(0),
	m_dwErrors(0)
{
	memset(&m_wszPortName[0], 0, sizeof(m_wszPortName));
	memset(&m_stat, 0, sizeof(m_stat));
}

comportio::~comportio(void)
{
	if (m_hHandle != INVALID_HANDLE_VALUE) {
        BOOL fReturn = CloseHandle(m_hHandle);
        if (!fReturn) {
            m_dwError = GetLastError();
        }
	}
	m_hHandle = INVALID_HANDLE_VALUE;
}

SHORT comportio::OpenCom(USHORT usPortId, const comportio::com_parms Protocol, USHORT usComBaud)
{
    DCB dcb = {0};
    COMMTIMEOUTS comTimer = {0};
    DWORD   dwCommMasks;
    DWORD   dwBaudRate;                 // baud rate
    DWORD   nCharBits, nMultiple;
    BYTE    bByteSize;                  // number of bits/byte, 4-8
    BYTE    bParity;                    // 0-4 = no, odd, even, mark, space
    BYTE    bStopBits;                  // 0,1,2 = 1, 1.5, 2
    BOOL    fResult;
    USHORT  usFlag = 0;
    TCHAR  wszDisplay[128];
	USHORT  i;

	// see Microsoft document HOWTO: Specify Serial Ports Larger than COM9.
	// https://support.microsoft.com/en-us/kb/115831
	// CreateFile() can be used to get a handle to a serial port. The "Win32 Programmer's Reference" entry for "CreateFile()"
	// mentions that the share mode must be 0, the create parameter must be OPEN_EXISTING, and the template must be NULL. 
	//
	// CreateFile() is successful when you use "COM1" through "COM9" for the name of the file;
	// however, the value INVALID_HANDLE_VALUE is returned if you use "COM10" or greater. 
	//
	// If the name of the port is \\.\COM10, the correct way to specify the serial port in a call to
	// CreateFile() is "\\\\.\\COM10".
	//
	// NOTES: This syntax also works for ports COM1 through COM9. Certain boards will let you choose
	//        the port names yourself. This syntax works for those names as well.
    wsprintf(m_wszPortName, TEXT("\\\\.\\COM%d"), usPortId);

	/* Open the serial port. */
	/* avoid to failuer of CreateFile */
	for (i = 0; i < 10; i++) {
	    m_hHandle = CreateFile (m_wszPortName, /* Pointer to the name of the port, PifOpenCom() */
    	                  GENERIC_READ | GENERIC_WRITE,  /* Access (read-write) mode */
            	          0,            /* Share mode */
                	      NULL,         /* Pointer to the security attribute */
                    	  OPEN_EXISTING,/* How to open the serial port */
	                      0,            /* Port attributes */
    	                  NULL);        /* Handle to port with attribute */
        	                            /* to copy */

	    /* If it fails to open the port, return FALSE. */
   		if ( m_hHandle == INVALID_HANDLE_VALUE )   {    /* Could not open the port. */
       		m_dwError = GetLastError ();
			if (m_dwError == ERROR_FILE_NOT_FOUND || m_dwError == ERROR_INVALID_NAME || m_dwError == ERROR_ACCESS_DENIED) {
				// the COM port does not exist. probably a Virtual Serial Communications Port
				// from a USB device which was either unplugged or turned off.
				// or the COM port or Virtual Serial Communications port is in use by some other application.
				return PIF_ERROR_COM_ACCESS_DENIED;
			}
			Sleep(500);
		} else {
			break;
		}
    }
   	if ( m_hHandle == INVALID_HANDLE_VALUE )   {    /* Could not open the port. */
        wsprintf(wszDisplay, TEXT("CreateFile, COM%d, Last Error =%d\n"), usPortId, m_dwError);
        OutputDebugString(wszDisplay);
        return PIF_ERROR_COM_ERRORS;
    }

    /* clear the error and purge the receive buffer */
    m_dwError = (DWORD)(~0);                  // set all error code bits on
    ClearCommError(m_hHandle, &m_dwError, NULL);
    PurgeComm( m_hHandle, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR ) ;


    /* make up comm. parameters */
    dwBaudRate = usComBaud;
    if (dwBaudRate == 0) {
        CloseHandle(m_hHandle);  m_hHandle = INVALID_HANDLE_VALUE;
        wsprintf(wszDisplay, TEXT("Baud Rate == 0, COM%d"), usPortId);
        OutputDebugString(wszDisplay);
        return PIF_ERROR_COM_ERRORS;
    }

	switch (Protocol & (COM_BYTE_7_BITS_DATA | COM_BYTE_8_BITS_DATA)) {
		case COM_BYTE_7_BITS_DATA:
			bByteSize = 7;
			break;
		default:
			bByteSize = 8;
			break;
    }

    switch (Protocol & COM_BYTE_2_STOP_BITS) {
		case COM_BYTE_2_STOP_BITS:
			bStopBits = TWOSTOPBITS;
			break;
		default:
			bStopBits = ONESTOPBIT;
			break;
    }

	switch (Protocol & (COM_BYTE_ODD_PARITY | COM_BYTE_EVEN_PARITY)) {
		case COM_BYTE_EVEN_PARITY:
			bParity = EVENPARITY;
			break;
		case COM_BYTE_ODD_PARITY:
			bParity = ODDPARITY;
			break;
		default:
			bParity = NOPARITY;
			break;
	}

    /* Get the default port setting information. */
    GetCommState (m_hHandle, &dcb);

    /* set up no flow control as default */
    dcb.BaudRate = dwBaudRate;              // Current baud 
    dcb.fBinary = TRUE;                     // Binary mode; no EOF check 
    dcb.fParity = (bParity != NOPARITY);    // Enable parity checking 
    dcb.ByteSize = bByteSize;               // Number of bits/byte, 4-8
    dcb.Parity = bParity;                   // 0-4=no,odd,even,mark,space 
    dcb.StopBits = bStopBits;               // 0,1,2 = 1, 1.5, 2 
    dcb.fDsrSensitivity = FALSE;            // DSR sensitivity 

	switch (Protocol & (COM_BYTE_HANDSHAKE_XONOFF | COM_BYTE_HANDSHAKE_RTS | COM_BYTE_HANDSHAKE_CTS | COM_BYTE_HANDSHAKE_DTRDSR)) {
		case COM_BYTE_HANDSHAKE_XONOFF:
			dcb.fOutxCtsFlow = FALSE;         // No CTS output flow control 
			dcb.fOutxDsrFlow = TRUE;          // No DSR output flow control for 7161
			dcb.fTXContinueOnXoff = TRUE;     /* XOFF continues Tx */
			dcb.fOutX             = TRUE;     /* XON/XOFF out flow control */
			dcb.fInX              = TRUE;     /* XON/XOFF in flow control */
			dcb.XonChar			  = 0x11;	  //The default value for this property is the ASCII/ANSI value 17
			dcb.XoffChar		  = 0x13;	  //The default value for this property is the ASCII/ANSI value 19
			break;

		case (COM_BYTE_HANDSHAKE_RTS | COM_BYTE_HANDSHAKE_CTS):
			/* set up RTS/CTS flow control by option in device configulation*/
			dcb.fOutxCtsFlow = TRUE;          // CTS output flow control 
			dcb.fOutxDsrFlow = FALSE;         // No DSR output flow control 
			dcb.fTXContinueOnXoff = FALSE;    // XOFF continues Tx heee
			dcb.fOutX = FALSE;                // No XON/XOFF out flow control 
			dcb.fInX = FALSE;                 // No XON/XOFF in flow control 
			dcb.fRtsControl = RTS_CONTROL_HANDSHAKE; 
			dcb.fDsrSensitivity = FALSE;      // DSR sensitivity 
			break;

		case COM_BYTE_HANDSHAKE_RTS:
			//Set up RTS flow control by option in device configulation
			dcb.fOutxCtsFlow = FALSE;         // no CTS output flow control 
			dcb.fOutxDsrFlow = FALSE;         // No DSR output flow control 
			dcb.fTXContinueOnXoff = FALSE;    // XOFF continues Tx 
			dcb.fOutX = FALSE;                // No XON/XOFF out flow control 
			dcb.fInX = FALSE;                 // No XON/XOFF in flow control 
			dcb.fRtsControl = RTS_CONTROL_HANDSHAKE; 
			dcb.fDsrSensitivity = FALSE;      // DSR sensitivity
			break;

		case COM_BYTE_HANDSHAKE_CTS:
			//Set up CTS flow control by option in device configulation
			dcb.fOutxCtsFlow = TRUE;          // CTS output flow control 
			dcb.fOutxDsrFlow = FALSE;         // No DSR output flow control 
			dcb.fTXContinueOnXoff = FALSE;    // XOFF continues Tx 
			dcb.fOutX = FALSE;                // No XON/XOFF out flow control 
			dcb.fInX = FALSE;                 // No XON/XOFF in flow control 
			dcb.fRtsControl = FALSE; 
			dcb.fDsrSensitivity = FALSE;      // DSR sensitivity
			break;

		case COM_BYTE_HANDSHAKE_DTRDSR:
			//Set up DSR/DTR flow control by option in device configulation
			dcb.fOutxCtsFlow = FALSE;         // CTS output flow control 
			dcb.fOutxDsrFlow = TRUE;          // DSR output flow control 
			dcb.fDtrControl = DTR_CONTROL_HANDSHAKE; //
			dcb.fTXContinueOnXoff = FALSE;    // XOFF continues Tx 
			dcb.fOutX = FALSE;                // No XON/XOFF out flow control 
			dcb.fInX = FALSE;                 // No XON/XOFF in flow control 
			dcb.fRtsControl = FALSE; 
			dcb.fDsrSensitivity = FALSE;      // DSR sensitivity
			break;
	}

    /* Configure the port according to the specifications of the DCB */
    /* structure. */
    fResult = SetCommState (m_hHandle, &dcb);
    if (!fResult) {
        /* Could not create the read thread. */
        m_dwError = GetLastError ();
        CloseHandle(m_hHandle);  m_hHandle = INVALID_HANDLE_VALUE;
        wsprintf (wszDisplay, TEXT("SetCommState, COM%d, Last Error =%d"), usPortId, m_dwError);
        OutputDebugString (wszDisplay);
        return PIF_ERROR_COM_ERRORS;
    }

    /* set up default time out value */

    /* compute no. of bits / data */
    nCharBits  = 1 + bByteSize;
    nCharBits += (bParity   == NOPARITY)   ? 0 : 1;
    nCharBits += (bStopBits == ONESTOPBIT) ? 1 : 2;
    nMultiple = (2 * CBR_9600 / dwBaudRate);

    fResult = GetCommTimeouts(m_hHandle, &comTimer);
    //Windows CE default values are listed next to the variables
	//These were verified on Windows CE
	//These default values were different on Windows XP
	//so set the values to the CE defaults
    comTimer.ReadIntervalTimeout         = 250;         /* 250 default of CE Emulation driver */
    comTimer.ReadTotalTimeoutMultiplier  = 10;          /* 10 default of CE Emulation driver */
    comTimer.ReadTotalTimeoutConstant    = 2000;        /* read within 2000 msec (100 default of CE Emulation driver) */
    comTimer.WriteTotalTimeoutMultiplier = 0;           /* 0 default of CE Emulation driver */
    comTimer.WriteTotalTimeoutConstant   = 1000;        /* allow 1000 msec to write (0 default of CE Emulation driver) */

    fResult = SetCommTimeouts(m_hHandle, &comTimer);
    if (! fResult)
    {
        m_dwError = GetLastError();
        CloseHandle(m_hHandle);  m_hHandle = INVALID_HANDLE_VALUE;
        wsprintf(wszDisplay, TEXT("SetCommTimeouts, COM%d, Last Error =%d"), usPortId, m_dwError);
        OutputDebugString(wszDisplay);
        return PIF_ERROR_COM_ERRORS;
    }

    /* Direct the port to perform extended functions SETDTR and SETRTS */
    /* SETDTR: Sends the DTR (data-terminal-ready) signal. */
    /* SETRTS: Sends the RTS (request-to-send) signal. */
    EscapeCommFunction (m_hHandle, SETDTR);
    EscapeCommFunction (m_hHandle, SETRTS);
    
    /* make up comm. event mask */
    dwCommMasks = EV_CTS | EV_DSR | EV_ERR | EV_RLSD | EV_RXCHAR;

    /* set up comm. event mask */
    fResult = SetCommMask(m_hHandle, dwCommMasks);
    if (! fResult)
    {
        m_dwError = GetLastError();
        CloseHandle(m_hHandle);  m_hHandle = INVALID_HANDLE_VALUE;
        wsprintf(wszDisplay, TEXT("SetCommMask, COM%d, Last Error =%d"), usPortId, m_dwError);
        OutputDebugString(wszDisplay);
        return PIF_ERROR_COM_ERRORS;
    }

    /* clear the error and purge the receive buffer */

//  dwErrors = (DWORD)(~0);                 // set all error code bits on
//  fResult  = ClearCommError(hComm, &dwErrors, NULL);
//  fResult  = PurgeComm(hComm, PURGE_RXABORT | PURGE_RXCLEAR);

   
    return PIF_OK;
}

SHORT comportio::CloseCom (void)
{
	if (m_hHandle != INVALID_HANDLE_VALUE) {
        BOOL fReturn = CloseHandle(m_hHandle);
        if (fReturn == 0) {
            m_dwError = GetLastError();
        }
	}
	m_hHandle = INVALID_HANDLE_VALUE;

	return 0;
}

SHORT   comportio::ReadCom(VOID *pBuffer, USHORT usBytes)
{
    BOOL fResult = ReadFile(m_hHandle, pBuffer, (DWORD)usBytes, &m_dwBytesRead, NULL);
    if (fResult) {
        if (m_dwBytesRead == 0) return PIF_ERROR_COM_TIMEOUT;
        return (SHORT)m_dwBytesRead;
    } else {
        m_dwError = GetLastError();
        return (PIF_ERROR_COM_ERRORS);
    }
}

SHORT   comportio::WriteCom(CONST VOID *pBuffer, USHORT usBytes)
{
	BOOL fResult = WriteFile(m_hHandle, pBuffer, (DWORD)usBytes, &m_dwBytesWritten, NULL);
    if (fResult) {
        if ((usBytes != m_dwBytesWritten) && (m_dwBytesWritten == 0)) {
            PurgeComm(m_hHandle, PURGE_TXCLEAR);
            return PIF_ERROR_COM_TIMEOUT;
        }
        return (SHORT)m_dwBytesWritten;
    } else {
        m_dwError = GetLastError();
        return PIF_ERROR_COM_ERRORS;
    }
}

SHORT comportio::GetStatusCom(void)
{
    BOOL    bResult;
    SHORT   sStatus = 0;

    bResult = ClearCommError(m_hHandle, &m_dwErrors, &m_stat);
    if (! bResult)
    {
        m_dwError = GetLastError();
        return PIF_ERROR_COM_ERRORS;                    // exit ...
    }

    // make up comm. status
    if (m_dwErrors & CE_FRAME)
        sStatus |= SIO32_STS_FRAMING;
    if (m_dwErrors & CE_OVERRUN || m_dwErrors & CE_RXOVER)
        sStatus |= SIO32_STS_OVERRUN;
    if (m_dwErrors & CE_RXPARITY)
        sStatus |= SIO32_STS_PARITY;

    bResult = GetCommModemStatus(m_hHandle, &m_dwModem);
    if (! bResult)
    {
        m_dwError = GetLastError();
        return PIF_ERROR_COM_ERRORS;                    // exit ...
    }

    if (m_dwModem & MS_CTS_ON)
        sStatus |= SIO32_STS_CTS;
    if (m_dwModem & MS_DSR_ON)
        sStatus |= SIO32_STS_DSR;
    if (m_dwModem & MS_RLSD_ON)
        sStatus |= SIO32_STS_RLSD;
        
    return sStatus;
}
