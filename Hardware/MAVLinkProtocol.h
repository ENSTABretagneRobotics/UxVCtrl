// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef MAVLINKPROTOCOL_H
#define MAVLINKPROTOCOL_H

#include "OSMisc.h"

#ifdef _MSC_VER
// Disable some Visual Studio warnings.
#pragma warning(disable : 4201) 
#pragma warning(disable : 4244) 
#pragma warning(disable : 4459) 
#pragma warning(disable : 4214) 
#endif // _MSC_VER
#include "mavlink/common/mavlink.h"
#include "mavlink/ardupilotmega/mavlink_msg_rangefinder.h"
#include "mavlink/ardupilotmega/mavlink_msg_hwstatus.h"
#ifdef _MSC_VER
// Restore the Visual Studio warnings previously disabled.
#pragma warning(default : 4214) 
#pragma warning(default : 4459) 
#pragma warning(default : 4244) 
#pragma warning(default : 4201) 
#endif // _MSC_VER

#define MAX_PACKET_LEN_MAVLINK 263
#define MIN_PACKET_LEN_MAVLINK 8

/*
	// From Mission Planner...
	// Should be nb us since 1970...?

	int tem = logplaybackfile.BaseStream.Read(datearray, 0, datearray.Length);

	Array.Reverse(datearray);

	DateTime date1 = new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc);

	UInt64 dateint = BitConverter.ToUInt64(datearray, 0);

	try
	{
		// array is reversed above
		if (datearray[7] == 254 || datearray[7] == 253)
		{
			//rewind 8bytes
			logplaybackfile.BaseStream.Seek(-8, SeekOrigin.Current);
		}
		else
		{
			if ((dateint/1000/1000/60/60) < 9999999)
			{
				date1 = date1.AddMilliseconds(dateint/1000);

				lastlogread = date1.ToLocalTime();
			}
		}
	}
*/

// Convert GPS time of week and week number to UNIX time in ms.
inline uint64_t GPSTOW2UNIX(unsigned int GMS, unsigned int GWk)
{	
	int y = 1980, m = 1, d = 6;
	struct tm t;
	time_t tt;

	memset(&t, 0, sizeof(t));
	t.tm_year = y-1900;
	t.tm_mon = m-1;
	t.tm_mday = d+GWk*7;
	t.tm_isdst = 0;
	tt = timegm(&t);
	return (uint64_t)GMS*1000+(uint64_t)tt*(uint64_t)1000000;
}

inline size_t fwrite_tlog(unsigned int GMS, unsigned int GWk, mavlink_message_t msg, FILE* file)
{
	size_t res = 0;
	unsigned char sendbuf[MAX_PACKET_LEN_MAVLINK];
	int sendbuflen = 0;
	uint64_t datearray = 0;

	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuflen = mavlink_msg_to_send_buffer(sendbuf, &msg);
	datearray = GPSTOW2UNIX(GMS, GWk);
	SwapBytes((unsigned char*)&datearray, sizeof(datearray));
	res += fwrite(&datearray, 1, sizeof(datearray), file);
	if (res <= 0) return res;
	res += fwrite(sendbuf, 1, sendbuflen, file);
	return res;
}

#endif // MAVLINKPROTOCOL_H
