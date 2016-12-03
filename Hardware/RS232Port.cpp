/***************************************************************************************************************:')

RS232Port.c

RS232 port handling via local COM port or TCP/IP converter.

Fabrice Le Bars

Created : 2013-08-16

***************************************************************************************************************:)*/

// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "RS232Port.h"
