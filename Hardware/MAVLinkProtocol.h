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
/*
// Convert UNIX time in us to GPS time of week in ms and week number.
inline void UNIX2GPSTOW(uint64_t unix, unsigned int* pGMS, unsigned int* pGWk)
{
	time_t tt;
	struct tm* pt = NULL;
	struct tm gpsepoch;
	int y = 1980, m = 1, d = 6;

	*pGMS = (unix/1000)%1000;
	tt = unix/1000000;
	pt = gmtime(&tt);

	memset(&gpsepoch, 0, sizeof(gpsepoch));
	gpsepoch.tm_year = y-1900;
	gpsepoch.tm_mon = m-1;
	gpsepoch.tm_mday = d;
	gpsepoch.tm_isdst = 0;

	//*pGWk?
}
*/
// Convert GPS time of week in ms and week number to UNIX time in us.
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

inline size_t fwrite_tlog_gpstow(unsigned int GMS, unsigned int GWk, mavlink_message_t msg, FILE* file)
{
	uint64_t datearray = 0;
	unsigned char sendbuf[sizeof(datearray)+MAX_PACKET_LEN_MAVLINK];
	int sendbuflen = 0;

	memset(sendbuf, 0, sizeof(sendbuf));
	datearray = GPSTOW2UNIX(GMS, GWk);
	SwapBytes((unsigned char*)&datearray, sizeof(datearray));
	memcpy(sendbuf, &datearray, sizeof(datearray));
	sendbuflen += sizeof(datearray);
	sendbuflen += mavlink_msg_to_send_buffer(sendbuf+sendbuflen, &msg);
	return fwrite(sendbuf, 1, sendbuflen, file);
}

inline size_t fwrite_tlog_unix(uint64_t unix_time_us, mavlink_message_t msg, FILE* file)
{
	unsigned char sendbuf[sizeof(unix_time_us)+MAX_PACKET_LEN_MAVLINK];
	int sendbuflen = 0;

	memset(sendbuf, 0, sizeof(sendbuf));
	SwapBytes((unsigned char*)&unix_time_us, sizeof(unix_time_us));
	memcpy(sendbuf, &unix_time_us, sizeof(unix_time_us));
	sendbuflen += sizeof(unix_time_us);
	sendbuflen += mavlink_msg_to_send_buffer(sendbuf+sendbuflen, &msg);
	return fwrite(sendbuf, 1, sendbuflen, file);
}

inline size_t fwrite_tlog(mavlink_message_t msg, FILE* file)
{
	uint64_t unix_time_us = 0;
	struct timeval tv;
	unsigned char sendbuf[sizeof(unix_time_us)+MAX_PACKET_LEN_MAVLINK];
	int sendbuflen = 0;

	if (gettimeofday(&tv, NULL) != EXIT_SUCCESS) { tv.tv_sec = 0; tv.tv_usec = 0; }
	unix_time_us = (uint64_t)tv.tv_sec*(uint64_t)1000000+(uint64_t)tv.tv_usec;
	
	memset(sendbuf, 0, sizeof(sendbuf));
	SwapBytes((unsigned char*)&unix_time_us, sizeof(unix_time_us));
	memcpy(sendbuf, &unix_time_us, sizeof(unix_time_us));
	sendbuflen += sizeof(unix_time_us);
	sendbuflen += mavlink_msg_to_send_buffer(sendbuf+sendbuflen, &msg);
	return fwrite(sendbuf, 1, sendbuflen, file);
}

#endif // MAVLINKPROTOCOL_H
