/***************************************************************************************************************:')

I2CBus.c

I2C bus handling via local I2C bus or USB-ISS.

Fabrice Le Bars

Created : 2019-02-02

***************************************************************************************************************:)*/

// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef I2CBUS_H
#define I2CBUS_H

#include "OSComputerI2CBus.h"
#include "RS232Port.h"

/*
Debug macros specific to I2CBus.
*/
#ifdef _DEBUG_MESSAGES
#	define _DEBUG_MESSAGES_I2CBUS
#endif // _DEBUG_MESSAGES

#ifdef _DEBUG_WARNINGS
#	define _DEBUG_WARNINGS_I2CBUS
#endif // _DEBUG_WARNINGS

#ifdef _DEBUG_ERRORS
#	define _DEBUG_ERRORS_I2CBUS
#endif // _DEBUG_ERRORS

#ifdef _DEBUG_MESSAGES_I2CBUS
#	define PRINT_DEBUG_MESSAGE_I2CBUS(params) PRINT_DEBUG_MESSAGE(params)
#else
#	define PRINT_DEBUG_MESSAGE_I2CBUS(params)
#endif // _DEBUG_MESSAGES_I2CBUS

#ifdef _DEBUG_WARNINGS_I2CBUS
#	define PRINT_DEBUG_WARNING_I2CBUS(params) PRINT_DEBUG_WARNING(params)
#else
#	define PRINT_DEBUG_WARNING_I2CBUS(params)
#endif // _DEBUG_WARNINGS_I2CBUS

#ifdef _DEBUG_ERRORS_I2CBUS
#	define PRINT_DEBUG_ERROR_I2CBUS(params) PRINT_DEBUG_ERROR(params)
#else
#	define PRINT_DEBUG_ERROR_I2CBUS(params)
#endif // _DEBUG_ERRORS_I2CBUS

// To implement...

#endif // !I2CBUS_H
