// writereadport.cpp : Defines the entry point for the console application.
//

/*
 * Contributors:
 *    Richard Chambers, May 2017
 *
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * IN NO EVENT SHALL CONTRIBUTORS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL,
 * INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE
 * USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF CONTRIBUTORS HAVE BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 * CONTRIBUTORS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS
 * PROVIDED "AS IS". CONTRIBUTIRS HAVE NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT,
 * UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/

#include "stdafx.h"
#include "comportio.h"

#include <cctype>

/*
 *   writereadport "string to write"  portno
 *        writereadport opens the specified serial port with the default settings. The specified string
 *        is then written to the serial port followed by a read. Any data read is printed as ANSI characters
 *        to the standard output.
 *
 *   writereadport -f textfile portno
 *        writereadport opens the specified serial port with the default settings ahd the specified text file.
 *        The text file is read a text line at a time which is then written to the serial port and after each
 *        text line is written to the serial port a read on the serial port is done. Any data read is printed
 *        as ANSI characters to the standard output.
 *
 *   writereadport -c portno
 *        writereadport opens the specified serial port with the default settings. A console is session is then
 *        established and any text lines read from standard input are written to the serial port followed by a
 *        read. An data read is printed as ANSI characters to the standard output.

 *   writereadport -s "9600,8,n,1" otherarguments
 *        writereadport opens the specified serial port with the specified settings. The other arguments specified
 *        are then processed. Other arguments can be one of the variations of writereadport above.
 *
*/

static char * processBuffer (char *pCharBuff)
{
	char *pBuffSave = pCharBuff;
	char *pBuff = pCharBuff;

	while (*pCharBuff) {
		if (*pCharBuff == '\\') {
			pCharBuff++;
			switch (*pCharBuff) {
				case 'r':
					*pBuff = '\r';
					break;
				case 'n':
					*pBuff = '\n';
					break;
				case 't':
					*pBuff = '\t';
					break;
				case 'v':
					*pBuff = '\v';
					break;
				case 'a':
					*pBuff = '\a';
					break;
				case 'b':
					*pBuff = '\b';
					break;
				case '\\':
					*pBuff = '\\';
					break;
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
					// octal digit string. may be up to 3 octal digits (0-7).
					if (std::isdigit(*pCharBuff) && *pCharBuff < '8') {
						int x = (*pCharBuff & 0x0f);
						pCharBuff++;
						if (std::isdigit(*pCharBuff) && *pCharBuff < '8') {
							x <<= 3;    // left shift one octal digit
							x += (*pCharBuff & 0x07);
							pCharBuff++;
						}
						if (std::isdigit(*pCharBuff) && *pCharBuff < '8') {
							x <<= 3;    // left shift one octal digit
							x += (*pCharBuff & 0x07);
							pCharBuff++;
						}
						pCharBuff--;
						*pBuff = x;
					}
					break;

				case 'x':
					// hex digit string. C standard says may be any length so use that.
					pCharBuff++;
					if (std::isxdigit(*pCharBuff)) {
						int x = 0;
						while (*pCharBuff && std::isxdigit(*pCharBuff)) {
							int t = (*pCharBuff & 0x0f);
							if (*pCharBuff & 0x40) t += 9;
							x <<= 4;
							x |= t;
							pCharBuff++;
						}
						pCharBuff--;
						*pBuff = x;
					}
					break;
				default:
					*pBuff = *pCharBuff;
					break;
			}
		} else {
			*pBuff = *pCharBuff;
		}
		pBuff++; pCharBuff++;
	}

	*pBuff = 0;
	return pBuffSave;
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 3) {
		// argv[0] = program name
		// argv[1] = first arg, either a command flag (-s, -t, -c, etc) or text string
		// argv[2] = second arg, either command flag argument or COM port number
		// other argv[] elements depends on number of arguments specified.
		printf ("writereadport - Invalid arguments. Must specify COM port number.\n");
		printf ("  writereadport -t portno\n  writereadport text_string portno\n  writereadport -f file portno\n  writereadport -c portno\n");
		printf ("\n  To set the COM port baud rate, etc. use the -s flag with argument\n    -s \"9600,8,n,1\"\n");
		return 0;
	}

	comportio  myCom;

	TCHAR  *argFile = NULL;
	TCHAR  *argSettings = _T("9600,8,n,1");   // default settings for the Serial Communications Port, same format as -s command line option.
	TCHAR  *argPortNo = argv[2];
	TCHAR  *argString = argv[1];
	bool   bPrintComSettings = false;
	bool   bConsole = false;

	if (argc > 1 && argv[1][0] == _T('-')) {
		int i = 1;
		while (i < argc && argv[i] && (argv[i][0] == _T('-') || argv[i][0] == _T('/'))) {
			// both dash (-) and forward slash (/) are considered viable command flags
			// in most Windows command line utilities
			switch (argv[i][1]) {
				case _T('s'):
					if (i + 1 < argc - 1) {
						// only one more argument and that must be the port number
						argString = NULL;
						argPortNo = argv[i+1];
					} else {
						// at least two more arguments so assume they are string and port number.
						// if it is a command flag instead, that code will modify these.
						argSettings = argv[i+1];
						argString = argv[i+2];
					}
					argFile = NULL;
					argPortNo = argv[i+3];
					i += 2;
					break;

				case _T('c'):
					argString = NULL;
					argFile = NULL;
					argPortNo = argv[i+1];
					bConsole = true;
					i += 1;
					break;

				case _T('t'):
					bPrintComSettings = true;
					if (i + 1 < argc - 1) {
						// at least two more arguments so assume they are string and port number.
						// if it is a command flag instead, that code will modify these.
						argString = argv[i+1];
						argPortNo = argv[i+2];
					} else {
						// only one more argument and that must be the port number
						argString = NULL;
						argPortNo = argv[i+1];
					}
					i += 1;
					break;

				case _T('f'):
					argString = NULL;
					argFile = argv[i+1];
					argPortNo = argv[i+2];
					i += 1;
					break;

				default:
					i += 1;
					break;
			}
		}
	}

	if (argPortNo == NULL) {
		printf ("Incorrect arguments. COM port number is invalid.\n");
		return 0;
	}


	comportio::com_parms  myParms = comportio::COM_BYTE_SET_DEFAULT;
	int                   myBaud = 230400;

	int iSetting = 0;
	while (argSettings && *argSettings && iSetting < 4) {
		switch (iSetting) {
			case 0:
				// first setting is the baud rate
				myBaud = _ttoi (argSettings);
				while (*argSettings && *argSettings != _T(',')) argSettings++;
				if (*argSettings)  argSettings++;    // skip over comma to next character
				iSetting = 1;
				break;

			case 1:
				// second setting is the number of data bits to use
				switch (*argSettings) {
					case _T('7'):
						myParms = static_cast<comportio::com_parms> (comportio::COM_BYTE_7_BITS_DATA | myParms);
						break;
					default:
						myParms = static_cast<comportio::com_parms> (comportio::COM_BYTE_8_BITS_DATA | myParms);
						break;
				}
				if (*argSettings)  argSettings++;    // skip over number of data bits to next character
				if (*argSettings)  argSettings++;    // skip over comma to next character
				iSetting = 2;
				break;

			case 2:
				// third setting is the parity to use
				switch (*argSettings) {
					case _T('O'):
					case _T('o'):
						myParms = static_cast<comportio::com_parms> (comportio::COM_BYTE_ODD_PARITY | myParms);
						break;
					case _T('E'):
					case _T('e'):
						myParms = static_cast<comportio::com_parms> (comportio::COM_BYTE_EVEN_PARITY | myParms);
						break;
					default:
						// no parity so nothing to set as it is the default.
						break;
				}
				if (*argSettings)  argSettings++;    // skip over parity to next character
				if (*argSettings)  argSettings++;    // skip over comma to next character
				iSetting = 3;
				break;

			case 3:
				// fourth setting is the number of stop bits to use
				switch (*argSettings) {
					case _T('2'):
						myParms = static_cast<comportio::com_parms> (comportio::COM_BYTE_2_STOP_BITS | myParms);
						break;
					default:
						// one stop bit so nothing to set as it is the default.
						break;
				}
				if (*argSettings)  argSettings++;    // skip over stop bits to next character
				iSetting = 4;
				break;
		}
	}

	int portno = _ttoi (argPortNo);

	if (portno < 1) {
		printf ("Incorrect arguments. COM port number is invalid.\n");
		return 0;
	}

	const int  bufferSize = 2048;
	char       myWriteText[bufferSize] = {0};
	char       myReadText[bufferSize] = {0};

	myCom.OpenCom (portno, myParms, myBaud);

	if (bPrintComSettings) {
		char  sParity[24] = {0};

		switch (myCom.m_dcbStart.Parity) {
			case NOPARITY:
				strcpy (sParity, "none");
				break;
			case ODDPARITY:
				strcpy (sParity, "odd");
				break;
			case EVENPARITY:
				strcpy (sParity, "even");
				break;
			case MARKPARITY:
				strcpy (sParity, "mark");
				break;
			case SPACEPARITY:
				strcpy (sParity, "space");
				break;
			default:
				strcpy (sParity, "unknown");
				break;
		}

		int iStopBits = myCom.m_dcbStart.StopBits;
		if (myCom.m_dcbStart.StopBits < ONE5STOPBITS) iStopBits = ONE5STOPBITS;

		printf ("Baud=%d\nDataBits=%d\nParity=%s\nStopBits=%d\n", myCom.m_dcbStart.BaudRate,
			myCom.m_dcbStart.ByteSize, sParity, iStopBits);
	}

	if (argString) {
		// convert the argument string from UNICODE to char.
		for (int i = 0; i < bufferSize && argString[i]; i++) {
			myWriteText[i] = argString[i];
		}
		processBuffer (myWriteText);
		myCom.WriteCom (myWriteText, strlen(myWriteText));
		Sleep(50);
		myCom.ReadCom (&myReadText[0], sizeof(myReadText));
		printf ("Read\n%s\n", myReadText);
		fflush(stdout);
	} else if (argFile) {
		// file mode so open the input file and just keep reading lines of text until end of file
		for (int i = 0; myWriteText[i] = argFile[i]; i++);  // convert file name from UNICODE to char.
		FILE *fp = fopen(myWriteText, "r");
		if (fp) {
			while (fgets(myWriteText, bufferSize, stdin)) {
				processBuffer (myWriteText);
				int iLen = strlen(myWriteText);
				if (iLen > 1) {
					iLen--;        // remove the trailing newline or \n (0x0a) from fgets().
					myCom.WriteCom(myWriteText, iLen);
				}
				myCom.WriteCom("\r", 1);      // write out a carriage return which is stripped by the fgets().
				Sleep(50);
				myCom.ReadCom(&myReadText[0], sizeof(myReadText));
				printf("%s\n", myReadText);
				fflush(stdout);
			}
			fclose(fp);
		} else {
			printf ("File open failed file %s\n", myWriteText);
		}
	} else if (bConsole) {
		// console mode so just keep reading lines of text from stdin until end of file
		printf ("> ");
		fflush(stdout);
		while (fgets (myWriteText, bufferSize, stdin)) {
			processBuffer (myWriteText);
			int iLen = strlen(myWriteText);
			if (iLen > 1) {
				iLen--;        // remove the trailing newline or \n (0x0a) from fgets().
				myCom.WriteCom (myWriteText, iLen);
			}
			myCom.WriteCom ("\r", 1);      // write out a carriage return which is stripped by the fgets().
			printf ("Reading\n");
			fflush(stdout);
			Sleep(50);
			myCom.ReadCom (&myReadText[0], sizeof(myReadText));
			printf ("%s\n> ", myReadText);
			fflush(stdout);
		}
	}

	myCom.CloseCom();

	return 0;
}

