#pragma once

#define  PIF_OK                         0
#define  PIF_ERROR_COM_ACCESS_DENIED   -1
#define  PIF_ERROR_COM_ERRORS          -2
#define  PIF_ERROR_COM_TIMEOUT         -3

#define SIO32_STS_FRAMING           0x0800
#define SIO32_STS_PARITY            0x0400
#define SIO32_STS_OVERRUN           0x0200
#define SIO32_STS_RLSD              0x0080
#define SIO32_STS_DSR               0x0020
#define SIO32_STS_CTS               0x0010

class comportio
{
public:
	// The default COM setting is 8 data bits, 1 stop bit, no parity with DTR/DSR handshaking.
	enum com_parms { COM_BYTE_7_BITS_DATA = 0x001, COM_BYTE_8_BITS_DATA = 0x002, COM_BYTE_2_STOP_BITS = 0x004,
			COM_BYTE_ODD_PARITY = 0x008, COM_BYTE_EVEN_PARITY = 0x010,
			COM_BYTE_HANDSHAKE_XONOFF = 0x020, COM_BYTE_HANDSHAKE_RTS = 0x040, COM_BYTE_HANDSHAKE_CTS = 0x080, COM_BYTE_HANDSHAKE_DTRDSR = 0x100,
			COM_BYTE_SET_DEFAULT = 0x100 };

	comportio(void);
	~comportio(void);
	SHORT   OpenCom(USHORT usPortId, const comportio::com_parms pProtocol, USHORT usComBaud);
	SHORT   CloseCom (void);
	SHORT   ReadCom(VOID *pBuffer, USHORT usBytes);
	SHORT   WriteCom(CONST VOID *pBuffer, USHORT usBytes);
	SHORT   GetStatusCom(void);

	DWORD   m_dwError;
    HANDLE  m_hHandle;
	DWORD   m_dwBytesWritten;
    DWORD   m_dwBytesRead;
    DWORD   m_dwModem;
    DWORD   m_dwErrors;
    COMSTAT m_stat;

	TCHAR   m_wszPortName[16];
};
