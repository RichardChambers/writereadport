========================================================================
    CONSOLE APPLICATION : writereadport Project Overview
========================================================================

This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

NO WARRANTY FOR SOURCE CODE

ALL OF THE SOURCE CODE ON THIS WEB SITE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT. THIS SOURCE CODE SHOULD NOT BE RELIED UPON AS
THE SOLE BASIS FOR SOLVING A PROBLEM WHOSE INCORRECT SOLUTION COULD RESULT IN INJURY TO
PERSON OR PROPERTY. THE ENTIRE RISK AS TO THE RESULTS AND PERFORMANCE OF THIS SOURCE CODE
IS ASSUMED BY THE USER. IN NO EVENT SHALL ANY OF THE CONTRIBUTORS TO THIS WEB SITE BE
LIABLE FOR ANY CLAIM, DAMAGES, OR OTHER LIABILITY ARISING FROM THE USE OF THIS SOURCE CODE.

Build          Date          Description of Changes
01.00.00.01    05/22/2017    Initial build for general use.


Description of Utility

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
 - writereadport -t 4
   open Serial Port COM4 and determine the current settings of the port and print them to stdout.
   The output is in a keyword=value format as in:
       Baud=9600
       DataBits=8
       Parity=none
       StopBits=1

Embedding Special Characters into Text
This utility has a special processing function that take an entered line of text and processes the
text looking for special embedded characters in the text string. Most of the standard C special
format indicators (\r, \n, \t, \xhhh where hhh are hex digits, and \ooo where ooo are octal digits)
are supported. These special characters allow the writereadport utility to be used with a receipt
printer to do print testing.


This file contains a summary of what you will find in each of the files that
make up your writereadport application.

writereadport.vcproj
    This is the main project file for VC++ projects generated using an Application Wizard. 
    It contains information about the version of Visual C++ that generated the file, and 
    information about the platforms, configurations, and project features selected with the
    Application Wizard.

writereadport.cpp
    This is the main application source file. It processes the command line arguments, creates
    a comportio object to access the Serial Communications Port, and then performs the actions
    requested by the command line arguments.

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

Build History

01.00.00.01    05/22/2017    Initial build for general use.
This is the first release build provided to Amtrak for testing in their environment.
The build was done with Visual Studio 2005.
It contains initial support for the following command line arguments:
  -s "9600,8,n,1" for setting the COM port paramters
  -t  for printing the original COM port parameters to stdout
  -c  for console mode that takes keyboard entry and sends to COM port
  -f  for file mode opens a text file and sends the text lines to the COM port
  "text to send" for a text string on the command line to send to the COM port
  
It also contains initial support for embedding special characters using many of the C standard
escape sequences such as \r (carriage return character), \n (new line character),
\t (tab character), \x12a (hex digits), \123 (octal digits).


