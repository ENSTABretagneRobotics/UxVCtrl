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

#ifndef __cplusplus
#ifdef inline
#ifdef __GNUC__
#undef inline
#endif // __GNUC__
#endif // inline
#endif // !__cplusplus

#ifdef _MSC_VER
// Disable some Visual Studio warnings.
#pragma warning(disable : 4201) 
#pragma warning(disable : 4244) 
#pragma warning(disable : 4459) 
#pragma warning(disable : 4214) 
#endif // _MSC_VER

#ifdef __GNUC__
// Disable some GCC warnings.
#if (__GNUC__ >= 9)
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
#endif // (__GNUC__ >= 9)
#if (__GNUC__ >= 8)
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#endif // (__GNUC__ >= 8)
#if (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4))
#pragma GCC diagnostic push
#endif // (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4))
#endif // __GNUC__

#include "mavlink/common/mavlink.h"
#include "mavlink/ardupilotmega/mavlink_msg_rangefinder.h"
#include "mavlink/ardupilotmega/mavlink_msg_hwstatus.h"
#include "mavlink/ardupilotmega/mavlink_msg_wind.h"

#ifdef __GNUC__
// Restore the GCC warnings previously disabled.
#if (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4))
#pragma GCC diagnostic pop
#else
//#if (__GNUC__ >= 8)
//#pragma GCC diagnostic warning "-Wignored-qualifiers"
//#endif // (__GNUC__ >= 8)
//#if (__GNUC__ >= 9)
//#pragma GCC diagnostic warning "-Waddress-of-packed-member"
//#endif // (__GNUC__ >= 9)
#endif // (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4))
#endif // __GNUC__

#ifdef _MSC_VER
// Restore the Visual Studio warnings previously disabled.
#pragma warning(default : 4214) 
#pragma warning(default : 4459) 
#pragma warning(default : 4244) 
#pragma warning(default : 4201) 
#endif // _MSC_VER

#ifndef __cplusplus
#ifndef inline
#ifdef _MSC_VER
#define inline __inline
#endif // _MSC_VER
#ifdef __BORLANDC__
#define inline __inline
#endif // __BORLANDC__
#ifdef __GNUC__
// extern __inline__ in ws2tcpip.h for GNU?
#ifdef _WIN32
#if (__MINGW_GNUC_PREREQ(4, 3) && __STDC_VERSION__ >= 199901L) || (defined (__clang__))
// Problem with the use of inline in _mingw.h for WS2TCPIP_INLINE...
#include <ws2tcpip.h>
#define inline static __inline__
#else
#define inline static __inline__
#endif // (__MINGW_GNUC_PREREQ(4, 3) && __STDC_VERSION__ >= 199901L) || (defined (__clang__))
#else
#define inline static __inline__
#endif // _WIN32
#endif // __GNUC__
#endif // !inline
#endif // !__cplusplus

// For older versions of MAVLink headers...
#ifndef GPS_FIX_TYPE_NO_GPS
#define GPS_FIX_TYPE_NO_GPS 0
#endif // !GPS_FIX_TYPE_NO_GPS
#ifndef GPS_FIX_TYPE_NO_FIX
#define GPS_FIX_TYPE_NO_FIX 1
#endif // !GPS_FIX_TYPE_NO_FIX
#ifndef GPS_FIX_TYPE_2D_FIX
#define GPS_FIX_TYPE_2D_FIX 2
#endif // !GPS_FIX_TYPE_2D_FIX
#ifndef GPS_FIX_TYPE_3D_FIX
#define GPS_FIX_TYPE_3D_FIX 3
#endif // !GPS_FIX_TYPE_3D_FIX
#ifndef GPS_FIX_TYPE_DGPS
#define GPS_FIX_TYPE_DGPS 4
#endif // !GPS_FIX_TYPE_DGPS
#ifndef GPS_FIX_TYPE_RTK_FLOAT
#define GPS_FIX_TYPE_RTK_FLOAT 5
#endif // !GPS_FIX_TYPE_RTK_FLOAT
#ifndef GPS_FIX_TYPE_RTK_FIXED
#define GPS_FIX_TYPE_RTK_FIXED 6
#endif // !GPS_FIX_TYPE_RTK_FIXED
#ifndef GPS_FIX_TYPE_STATIC
#define GPS_FIX_TYPE_STATIC 7
#endif // !GPS_FIX_TYPE_STATIC
#ifndef GPS_FIX_TYPE_PPP
#define GPS_FIX_TYPE_PPP 8
#endif // !GPS_FIX_TYPE_PPP

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

	// *pGWk?
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

#endif // !MAVLINKPROTOCOL_H
