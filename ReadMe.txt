========================================================================
    CONSOLE APPLICATION : writereadport Project Overview
========================================================================

This console application is a command line utility for accessing and using a Serial
Communication Port. The application has three different working modes:
 - single shot to send a text string to a Serial Port
 - console mode to allow the user to start a session to send a series of text lines to a Serial Port
 - file mode to allow the user to specify a file to read from for the series of text lines to send  

 All three modes allow for an optional settings argument that must be the first option on the command line.
 The settings option is specified with the -s flag with the next argument being a string of comma separated
 values that specify:
  - the baud rate as a value such as 9600
  - the number of data bits as a value such as 7 or 8
  - the parity indicator: e (even parity), o (odd parity), n (no parity)
  - the number of stop bits as a value such as 1 or 2

  Examples of the -s option arguments:
   - "9600,8,n,1" means 9600 baud, 8 data bits, no parity, 1 stop bit
   - "4800,7,e,1" means 4800 baud, 7 data bits, even parity, 1 stop bit

 Examples of the writereadport command line:
  - writereadport -s "9600,8,n,1" "this is line 1\rthis is line 2\rthis is line 3\r" 2
    open Serial Port COM2 with settings 9600 baud, 8 data, no parity, 1 stop bit and send the text
	"this is line 1\rthis is line 2\rthis is line 3\r"
 - writereadport -s "19200,8,n,1" -c 3
   open Serial Port COM3 with settings 19.2K baud, 8 data, no parity, 1 stop bit then read a series
   of text lines from the standard input until an end of file is found.
   The standard input may be redirected using the "<" operator on the command line.


This file contains a summary of what you will find in each of the files that
make up your writereadport application.


writereadport.vcproj
    This is the main project file for VC++ projects generated using an Application Wizard. 
    It contains information about the version of Visual C++ that generated the file, and 
    information about the platforms, configurations, and project features selected with the
    Application Wizard.

writereadport.cpp
    This is the main application source file.

comportio.cpp
	This is the comportio class source file for the interface to the Serial Communication Port.
	It contains the source code for the class which encapsulates the Windows API used to open
	a Serial Communication Port, provision the necessary settings for baud rate, etc. and to
	perform the read and write operations from/to the Serial Communication Port.

/////////////////////////////////////////////////////////////////////////////
Other standard files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named writereadport.pch and a precompiled types file named StdAfx.obj.

/////////////////////////////////////////////////////////////////////////////
Other notes:

AppWizard uses "TODO:" comments to indicate parts of the source code you
should add to or customize.

/////////////////////////////////////////////////////////////////////////////
