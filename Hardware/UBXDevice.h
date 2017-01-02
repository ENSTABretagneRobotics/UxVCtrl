// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef UBXDEVICE_H
#define UBXDEVICE_H

#include "OSMisc.h"
#include "RS232Port.h"

#ifndef DISABLE_UBXDEVICETHREAD
#include "OSThread.h"
#endif // DISABLE_UBXDEVICETHREAD

#include "UBXProtocol.h"
#include "NMEAProtocol.h"
#include "RTCM3Protocol.h"

//#pragma pack(show)

// Check for potential paddings in bitfields and structs, check their size and the sum of the size of their fields!

// To prevent unexpected padding in struct...
#pragma pack(push,1) 

#define TIMEOUT_MESSAGE_UBXDEVICE 4.0 // In s.
// Should be at least 2 * number of bytes to be sure to contain entirely the biggest desired message (or group of messages) + 1.
#define MAX_NB_BYTES_UBXDEVICE 4096

struct UBXDEVICE
{
	RS232PORT RS232Port;
	FILE* pfSaveFile; // Used to save raw data, should be handled specifically...
	UBXDATA LastUBXData;
	char szCfgFilePath[256];
	// Parameters.
	char szDevPath[256];
	int BaudRate;
	int timeout;
	BOOL bSaveRawData;
	BOOL bRevertToDefaultCfg;
	BOOL bSetBaseCfg;
	int SurveyMode;
	double svinMinDur;
	double svinAccLimit;
	double fixedLat;
	double fixedLon;
	double fixedAlt;
	double fixedPosAcc;
	BOOL bEnableGGA;
	BOOL bEnableRMC;
	BOOL bEnableGLL;
	BOOL bEnableVTG;
	BOOL bEnableHDG;
	BOOL bEnableMWV;
	BOOL bEnableMWD;
	BOOL bEnableMDA;
	BOOL bEnableVDM;
};
typedef struct UBXDEVICE UBXDEVICE;

//inline int GetUBXPacketWithMIDUBXDevice(UBXDEVICE* pUBXDevice, int mclass, int mid, int* ppacketlen, unsigned char* databuf, int databuflen)
inline int GetUBXPacketWithMIDUBXDevice(UBXDEVICE* pUBXDevice, int mclass, int mid, UBXDATA* pUBXData)
{
	unsigned char recvbuf[MAX_NB_BYTES_UBXDEVICE];
	int BytesReceived = 0, recvbuflen = 0, res = EXIT_FAILURE, nbBytesToRequest = 0, nbBytesDiscarded = 0;
	unsigned char* ptr = NULL;
	int packetlen = 0;
	CHRONO chrono;

	StartChrono(&chrono);

	// Prepare the buffers.
	memset(recvbuf, 0, sizeof(recvbuf));
	recvbuflen = MAX_NB_BYTES_UBXDEVICE-1; // The last character must be a 0 to be a valid string for sscanf.
	BytesReceived = 0;

	// Suppose that there are not so many data to discard.
	// First try to get directly the desired message...

	nbBytesToRequest = MIN_PACKET_LENGTH_UBX;
	if (ReadAllRS232Port(&pUBXDevice->RS232Port, recvbuf, nbBytesToRequest) != EXIT_SUCCESS)
	{
		printf("Error reading data from a UBXDevice. \n");
		return EXIT_FAILURE;
	}
	BytesReceived += nbBytesToRequest;
	
	for (;;)
	{
		res = FindPacketWithMIDUBX(recvbuf, BytesReceived, mclass, mid, &packetlen, &nbBytesToRequest, &ptr, &nbBytesDiscarded);
		if (res == EXIT_SUCCESS) break;
		if (res == EXIT_FAILURE)
		{
			nbBytesToRequest = nbBytesDiscarded;
		}	
		memmove(recvbuf, recvbuf+nbBytesDiscarded, BytesReceived-nbBytesDiscarded);
		BytesReceived -= nbBytesDiscarded;
		if (BytesReceived+nbBytesToRequest > recvbuflen)
		{
			printf("Error reading data from a UBXDevice : Invalid data. \n");
			return EXIT_INVALID_DATA;
		}
		if (ReadAllRS232Port(&pUBXDevice->RS232Port, recvbuf+BytesReceived, nbBytesToRequest) != EXIT_SUCCESS)
		{
			printf("Error reading data from a UBXDevice. \n");
			return EXIT_FAILURE;
		}
		BytesReceived += nbBytesToRequest;
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_UBXDEVICE)
		{
			printf("Error reading data from a UBXDevice : Packet timeout. \n");
			return EXIT_TIMEOUT;
		}
	}

	if (BytesReceived-nbBytesDiscarded-packetlen > 0)
	{
		printf("Warning getting data from a UBXDevice : Unexpected data after a packet. \n");
	}

	//// Get data bytes.

	//memset(databuf, 0, databuflen);

	//// Check the number of data bytes before copy.
	//if (databuflen < *ppacketlen)
	//{
	//	printf("Error getting data from a UBXDevice : Too small data buffer. \n");
	//	return EXIT_FAILURE;
	//}

	//// Copy the data bytes of the message.
	//if (*ppacketlen > 0)
	//{
	//	memcpy(databuf, ptr, *ppacketlen);
	//}

	if (ProcessPacketUBX(ptr, packetlen, mclass, mid, pUBXData) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

// TODO...
// regarde si c'est du nmea, de l'ubx, et met à jour pUBXDevice->LastData
//inline int GetDataUBXDevice(UBXDEVICE* pUBXDevice, unsigned char* databuf, int databuflen)
inline int GetDataUBXDevice(UBXDEVICE* pUBXDevice, UBXDATA* pUBXData)
{
	unsigned char recvbuf[MAX_NB_BYTES_UBXDEVICE];
	int BytesReceived = 0, recvbuflen = 0, res = EXIT_FAILURE, nbBytesToRequest = 0, nbBytesDiscarded = 0;
	unsigned char* ptr = NULL;
	int packetlen = 0;
	int mclass = 0, mid = 0;
	CHRONO chrono;

	StartChrono(&chrono);

	// Prepare the buffers.
	memset(recvbuf, 0, sizeof(recvbuf));
	recvbuflen = MAX_NB_BYTES_UBXDEVICE-1; // The last character must be a 0 to be a valid string for sscanf.
	BytesReceived = 0;

	// Suppose that there are not so many data to discard.
	// First try to get directly the desired message...

	nbBytesToRequest = min(MIN_PACKET_LENGTH_UBX, MIN_NB_BYTES_SENTENCE_NMEA);
	if (ReadAllRS232Port(&pUBXDevice->RS232Port, recvbuf, nbBytesToRequest) != EXIT_SUCCESS)
	{
		printf("Error reading data from a UBXDevice. \n");
		return EXIT_FAILURE;
	}
	BytesReceived += nbBytesToRequest;
	
	for (;;)
	{
		res = FindPacketUBX(recvbuf, BytesReceived, &mclass, &mid, &packetlen, &nbBytesToRequest, &ptr, &nbBytesDiscarded);
		if (res == EXIT_SUCCESS) break;
		if (res == EXIT_FAILURE)
		{
			nbBytesToRequest = nbBytesDiscarded;
		}	
		memmove(recvbuf, recvbuf+nbBytesDiscarded, BytesReceived-nbBytesDiscarded);
		BytesReceived -= nbBytesDiscarded;
		if (BytesReceived+nbBytesToRequest > recvbuflen)
		{
			printf("Error reading data from a UBXDevice : Invalid data. \n");
			return EXIT_INVALID_DATA;
		}
		if (ReadAllRS232Port(&pUBXDevice->RS232Port, recvbuf+BytesReceived, nbBytesToRequest) != EXIT_SUCCESS)
		{
			printf("Error reading data from a UBXDevice. \n");
			return EXIT_FAILURE;
		}
		BytesReceived += nbBytesToRequest;
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_UBXDEVICE)
		{
			printf("Error reading data from a UBXDevice : Packet timeout. \n");
			return EXIT_TIMEOUT;
		}
	}

	if (BytesReceived-nbBytesDiscarded-packetlen > 0)
	{
		printf("Warning getting data from a UBXDevice : Unexpected data after a packet. \n");
	}

	//// Get data bytes.

	//memset(databuf, 0, databuflen);

	//// Check the number of data bytes before copy.
	//if (databuflen < *ppacketlen)
	//{
	//	printf("Error getting data from a UBXDevice : Too small data buffer. \n");
	//	return EXIT_FAILURE;
	//}

	//// Copy the data bytes of the message.
	//if (*ppacketlen > 0)
	//{
	//	memcpy(databuf, ptr, *ppacketlen);
	//}

	if (ProcessPacketUBX(ptr, packetlen, mclass, mid, pUBXData) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
/*
// We suppose that read operations return when a message has just been completely sent, and not randomly.
inline int GetLatestDataUBXDevice(UBXDEVICE* pUBXDevice, UBXDATA* pUBXData)
{
	char recvbuf[2*MAX_NB_BYTES_UBXDEVICE];
	char savebuf[MAX_NB_BYTES_UBXDEVICE];
	int BytesReceived = 0, Bytes = 0, recvbuflen = 0;
	char* ptr_GPGGA = NULL;
	char* ptr_GPRMC = NULL;
	char* ptr_GPGLL = NULL;
	char* ptr_GPVTG = NULL;
	char* ptr_HCHDG = NULL;
	char* ptr_IIMWV = NULL;
	char* ptr_WIMWV = NULL;
	char* ptr_WIMWD = NULL;
	char* ptr_WIMDA = NULL;
	char* ptr_AIVDM = NULL;
	int i = 0;
	CHRONO chrono;

	StartChrono(&chrono);

	// Prepare the buffers.
	memset(recvbuf, 0, sizeof(recvbuf));
	memset(savebuf, 0, sizeof(savebuf));
	recvbuflen = MAX_NB_BYTES_UBXDEVICE-1; // The last character must be a 0 to be a valid string for sscanf.
	BytesReceived = 0;

	if (ReadRS232Port(&pUBXDevice->RS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
	{
		printf("Error reading data from a UBXDevice. \n");
		return EXIT_FAILURE;
	}
	if ((pUBXDevice->bSaveRawData)&&(pUBXDevice->pfSaveFile))
	{
		fwrite(recvbuf, Bytes, 1, pUBXDevice->pfSaveFile);
		fflush(pUBXDevice->pfSaveFile);
	}
	BytesReceived += Bytes;

	if (BytesReceived >= recvbuflen)
	{
		// If the buffer is full and if the device always sends data, there might be old data to discard...

		while (Bytes == recvbuflen)
		{
			if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_UBXDEVICE)
			{
				printf("Error reading data from a UBXDevice : Message timeout. \n");
				return EXIT_TIMEOUT;
			}
			memcpy(savebuf, recvbuf, Bytes);
			if (ReadRS232Port(&pUBXDevice->RS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
			{
				printf("Error reading data from a UBXDevice. \n");
				return EXIT_FAILURE;
			}
			if ((pUBXDevice->bSaveRawData)&&(pUBXDevice->pfSaveFile)) 
			{
				fwrite(recvbuf, Bytes, 1, pUBXDevice->pfSaveFile);
				fflush(pUBXDevice->pfSaveFile);
			}
			BytesReceived += Bytes;
		}

		// The desired message should be among all the data gathered, unless there was 
		// so many other messages sent after that the desired message was in the 
		// discarded data, or we did not wait enough...

		memmove(recvbuf+recvbuflen-Bytes, recvbuf, Bytes);
		memcpy(recvbuf, savebuf+Bytes, recvbuflen-Bytes);

		// Only the last recvbuflen bytes received should be taken into account in what follows.
		BytesReceived = recvbuflen;
	}

	// The data need to be analyzed and we must check if we need to get more data from 
	// the device to get the desired message.
	// But normally we should not have to get more data unless we did not wait enough
	// for the desired message...

	if (pUBXDevice->bEnableGGA) ptr_GPGGA = FindLatestSentenceNMEA("$GPGGA", recvbuf);
	if (pUBXDevice->bEnableRMC) ptr_GPRMC = FindLatestSentenceNMEA("$GPRMC", recvbuf);
	if (pUBXDevice->bEnableGLL) ptr_GPGLL = FindLatestSentenceNMEA("$GPGLL", recvbuf);
	if (pUBXDevice->bEnableVTG) ptr_GPVTG = FindLatestSentenceNMEA("$GPVTG", recvbuf);
	if (pUBXDevice->bEnableHDG) ptr_HCHDG = FindLatestSentenceNMEA("$HCHDG", recvbuf);
	if (pUBXDevice->bEnableIIMWV) ptr_IIMWV = FindLatestSentenceNMEA("$IIMWV", recvbuf);
	if (pUBXDevice->bEnableMWV) ptr_WIMWV = FindLatestSentenceNMEA("$WIMWV", recvbuf);
	if (pUBXDevice->bEnableMWD) ptr_WIMWD = FindLatestSentenceNMEA("$WIMWD", recvbuf);
	if (pUBXDevice->bEnableMDA) ptr_WIMDA = FindLatestSentenceNMEA("$WIMDA", recvbuf);
	if (pUBXDevice->bEnableVDM) ptr_AIVDM = FindLatestSentenceNMEA("!AIVDM", recvbuf);

	while (
		(pUBXDevice->bEnableGGA&&!ptr_GPGGA)||
		(pUBXDevice->bEnableRMC&&!ptr_GPRMC)||
		(pUBXDevice->bEnableGLL&&!ptr_GPGLL)||
		(pUBXDevice->bEnableVTG&&!ptr_GPVTG)||
		(pUBXDevice->bEnableHDG&&!ptr_HCHDG)||
		(pUBXDevice->bEnableIIMWV&&!ptr_IIMWV)||
		(pUBXDevice->bEnableMWV&&!ptr_WIMWV)||
		(pUBXDevice->bEnableMWD&&!ptr_WIMWD)||
		(pUBXDevice->bEnableMDA&&!ptr_WIMDA)||
		(pUBXDevice->bEnableVDM&&!ptr_AIVDM)
		)
	{
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_UBXDEVICE)
		{
			printf("Error reading data from a UBXDevice : Message timeout. \n");
			return EXIT_TIMEOUT;
		}
		// The last character must be a 0 to be a valid string for sscanf.
		if (BytesReceived >= 2*MAX_NB_BYTES_UBXDEVICE-1)
		{
			printf("Error reading data from a UBXDevice : Invalid data. \n");
			return EXIT_INVALID_DATA;
		}
		if (ReadRS232Port(&pUBXDevice->RS232Port, (unsigned char*)recvbuf+BytesReceived, 2*MAX_NB_BYTES_UBXDEVICE-1-BytesReceived, &Bytes) != EXIT_SUCCESS)
		{
			printf("Error reading data from a UBXDevice. \n");
			return EXIT_FAILURE;
		}
		if ((pUBXDevice->bSaveRawData)&&(pUBXDevice->pfSaveFile)) 
		{
			fwrite((unsigned char*)recvbuf+BytesReceived, Bytes, 1, pUBXDevice->pfSaveFile);
			fflush(pUBXDevice->pfSaveFile);
		}
		BytesReceived += Bytes;
		if (pUBXDevice->bEnableGGA) ptr_GPGGA = FindLatestSentenceNMEA("$GPGGA", recvbuf);
		if (pUBXDevice->bEnableRMC) ptr_GPRMC = FindLatestSentenceNMEA("$GPRMC", recvbuf);
		if (pUBXDevice->bEnableGLL) ptr_GPGLL = FindLatestSentenceNMEA("$GPGLL", recvbuf);
		if (pUBXDevice->bEnableVTG) ptr_GPVTG = FindLatestSentenceNMEA("$GPVTG", recvbuf);
		if (pUBXDevice->bEnableHDG) ptr_HCHDG = FindLatestSentenceNMEA("$HCHDG", recvbuf);
		if (pUBXDevice->bEnableIIMWV) ptr_IIMWV = FindLatestSentenceNMEA("$IIMWV", recvbuf);
		if (pUBXDevice->bEnableMWV) ptr_WIMWV = FindLatestSentenceNMEA("$WIMWV", recvbuf);
		if (pUBXDevice->bEnableMWD) ptr_WIMWD = FindLatestSentenceNMEA("$WIMWD", recvbuf);
		if (pUBXDevice->bEnableMDA) ptr_WIMDA = FindLatestSentenceNMEA("$WIMDA", recvbuf);
		if (pUBXDevice->bEnableVDM) ptr_AIVDM = FindLatestSentenceNMEA("!AIVDM", recvbuf);
	}

	// Analyze data.

	ProcessSentenceNMEA();
	pUBXDevice->LastUBXData = *pNMEAData;

	return EXIT_SUCCESS;
}*/
/*
inline int GetLatestPacketWithMIDUBX(UBXDEVICE* pUBXDevice, int mclass, int mid, int* ppacketlen, unsigned char* databuf, int databuflen)
{

	return EXIT_SUCCESS;
}

inline int ProcessPacketsUBX(unsigned char* buf, int buflen, int* pnbBytesToRequest, int* pnbBytesDiscarded)
{
	unsigned char* ptr = buf;
	int res = EXIT_FAILURE, nbBytesDiscarded = 0, mclass_tmp= 0, mid_tmp = 0, packetlen = 0;

	packetlen = 0;
	*pnbBytesToRequest = -1;
	*pnbBytesDiscarded = 0;

	for (;;) 
	{
		res = FindPacketUBX(ptr+packetlen, buflen-(*pnbBytesDiscarded)-packetlen, 
			&mclass_tmp, &mid_tmp, &packetlen, pnbBytesToRequest, &ptr, &nbBytesDiscarded);
		switch (res)
		{
		case EXIT_SUCCESS:
			(*pnbBytesDiscarded) += nbBytesDiscarded;
			break;
		case EXIT_OUT_OF_MEMORY:
			(*pnbBytesDiscarded) += nbBytesDiscarded;
			return EXIT_OUT_OF_MEMORY;
		default:
			(*pnbBytesDiscarded) += nbBytesDiscarded;
			return EXIT_FAILURE;
		}

		ProcessPacketUBX(ptr, packetlen, mclass_tmp, mid_tmp);
	}
}
*/
inline int RevertToDefaultCfgUBXDevice(UBXDEVICE* pUBXDevice)
{
	unsigned char sendbuf[32];
	int sendbuflen = 0;
	struct CFG_CFG_PL_UBX cfg_cfg_pl;

	memset(&cfg_cfg_pl, 0, sizeof(cfg_cfg_pl));	
	cfg_cfg_pl.clearMask.ioPort = 1;
	cfg_cfg_pl.clearMask.msgConf = 1;
	cfg_cfg_pl.clearMask.infMsg = 1;
	cfg_cfg_pl.clearMask.navConf = 1;
	cfg_cfg_pl.clearMask.rxmConf = 1;
	cfg_cfg_pl.clearMask.senConf = 1;
	cfg_cfg_pl.clearMask.rinvConf = 1;
	cfg_cfg_pl.clearMask.antConf = 0;
	cfg_cfg_pl.clearMask.logConf = 1;
	cfg_cfg_pl.clearMask.ftsConf = 1;
	cfg_cfg_pl.loadMask.ioPort = 1;
	cfg_cfg_pl.loadMask.msgConf = 1;
	cfg_cfg_pl.loadMask.infMsg = 1;
	cfg_cfg_pl.loadMask.navConf = 1;
	cfg_cfg_pl.loadMask.rxmConf = 1;
	cfg_cfg_pl.loadMask.senConf = 1;
	cfg_cfg_pl.loadMask.rinvConf = 1;
	cfg_cfg_pl.loadMask.antConf = 1;
	cfg_cfg_pl.loadMask.logConf = 1;
	cfg_cfg_pl.loadMask.ftsConf = 1;
	cfg_cfg_pl.deviceMask.devBBR = 1;
	cfg_cfg_pl.deviceMask.devFlash = 1;
	cfg_cfg_pl.deviceMask.devEEPROM = 1;
	cfg_cfg_pl.deviceMask.devSpiFlash = 1;
	EncodePacketUBX(sendbuf, &sendbuflen, CFG_CLASS_UBX, CFG_CFG_ID_UBX, (unsigned char*)&cfg_cfg_pl, sizeof(cfg_cfg_pl));

	if (WriteAllRS232Port(&pUBXDevice->RS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		printf("Error writing data to a UBXDevice. \n");
		return EXIT_FAILURE;
	}
	if ((pUBXDevice->bSaveRawData)&&(pUBXDevice->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, pUBXDevice->pfSaveFile);
		fflush(pUBXDevice->pfSaveFile);
	}

	// Should check ACK...

	return EXIT_SUCCESS;
}

inline int SetBaseCfgUBXDevice(UBXDEVICE* pUBXDevice)
{
	unsigned char sendbuf[512];
	int sendbuflen = 0;
	int offset = 0;
	unsigned char packet[64];
	int packetlen = 0;
	unsigned char cfg_msg_pl[8];
	struct CFG_TMODE3_PL_UBX cfg_tmode3_pl;

	// Enable 1005, 1077, 1087, 1127 RTCM messages on UART1 and USB.
	memset(cfg_msg_pl, 0, sizeof(cfg_msg_pl));
	cfg_msg_pl[0] = RTCM_CLASS_UBX;
	cfg_msg_pl[1] = RTCM_1005_ID_UBX;
	cfg_msg_pl[3] = 1; // UART1 rate.
	cfg_msg_pl[5] = 1; // USB rate.
	EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, sizeof(cfg_msg_pl));
	memcpy(sendbuf+offset, packet, packetlen);
	offset += packetlen;
	cfg_msg_pl[1] = RTCM_1077_ID_UBX;
	EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, sizeof(cfg_msg_pl));
	memcpy(sendbuf+offset, packet, packetlen);
	offset += packetlen;
	cfg_msg_pl[1] = RTCM_1087_ID_UBX;
	EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, sizeof(cfg_msg_pl));
	memcpy(sendbuf+offset, packet, packetlen);
	offset += packetlen;
	cfg_msg_pl[1] = RTCM_1127_ID_UBX;
	EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, sizeof(cfg_msg_pl));
	memcpy(sendbuf+offset, packet, packetlen);
	offset += packetlen;
	// Disable GGA, GLL, GSA, GSV, RMC, VTG, TXT NMEA messages.
	memset(cfg_msg_pl, 0, sizeof(cfg_msg_pl));
	cfg_msg_pl[0] = NMEA_STD_CLASS_UBX;
	cfg_msg_pl[1] = NMEA_STD_GGA_ID_UBX;
	EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, sizeof(cfg_msg_pl));
	memcpy(sendbuf+offset, packet, packetlen);
	offset += packetlen;
	cfg_msg_pl[1] = NMEA_STD_GLL_ID_UBX;
	EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, sizeof(cfg_msg_pl));
	memcpy(sendbuf+offset, packet, packetlen);
	offset += packetlen;
	cfg_msg_pl[1] = NMEA_STD_GSA_ID_UBX;
	EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, sizeof(cfg_msg_pl));
	memcpy(sendbuf+offset, packet, packetlen);
	offset += packetlen;
	cfg_msg_pl[1] = NMEA_STD_GSV_ID_UBX;
	EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, sizeof(cfg_msg_pl));
	memcpy(sendbuf+offset, packet, packetlen);
	offset += packetlen;
	cfg_msg_pl[1] = NMEA_STD_RMC_ID_UBX;
	EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, sizeof(cfg_msg_pl));
	memcpy(sendbuf+offset, packet, packetlen);
	offset += packetlen;
	cfg_msg_pl[1] = NMEA_STD_VTG_ID_UBX;
	EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, sizeof(cfg_msg_pl));
	memcpy(sendbuf+offset, packet, packetlen);
	offset += packetlen;
	cfg_msg_pl[1] = NMEA_STD_TXT_ID_UBX;
	EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, sizeof(cfg_msg_pl));
	memcpy(sendbuf+offset, packet, packetlen);
	offset += packetlen;

	//UBX-CFG-PORT set RTCM 3 as Output Protocol

	// Activate Self-Survey-In or Fixed-Position-Mode.
	memset(&cfg_tmode3_pl, 0, sizeof(cfg_tmode3_pl));
	cfg_tmode3_pl.flags.lla = 1;
	cfg_tmode3_pl.flags.mode = (unsigned short)pUBXDevice->SurveyMode;
	cfg_tmode3_pl.ecefXOrLat = (long)(pUBXDevice->fixedLat*10000000);
	cfg_tmode3_pl.ecefYOrLon = (long)(pUBXDevice->fixedLon*10000000);
	cfg_tmode3_pl.ecefZOrAlt = (long)(pUBXDevice->fixedAlt*100);
	cfg_tmode3_pl.ecefXOrLatHP = (char)((pUBXDevice->fixedLat*10000000-cfg_tmode3_pl.ecefXOrLat)*100);
	cfg_tmode3_pl.ecefYOrLonHP = (char)((pUBXDevice->fixedLon*10000000-cfg_tmode3_pl.ecefYOrLon)*100);
	cfg_tmode3_pl.ecefZOrAltHP = (char)((pUBXDevice->fixedAlt*100-cfg_tmode3_pl.ecefZOrAlt)*100);
	cfg_tmode3_pl.fixedPosAcc = (unsigned long)(pUBXDevice->fixedPosAcc*10000);
	cfg_tmode3_pl.svinMinDur = (unsigned long)pUBXDevice->svinMinDur;
	cfg_tmode3_pl.svinAccLimit = (unsigned long)(pUBXDevice->svinAccLimit*10000);
	EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_TMODE3_ID_UBX, (unsigned char*)&cfg_tmode3_pl, sizeof(cfg_tmode3_pl));
	memcpy(sendbuf+offset, packet, packetlen);
	offset += packetlen;
	
	sendbuflen = offset;

	if (WriteAllRS232Port(&pUBXDevice->RS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		printf("Error writing data to a UBXDevice. \n");
		return EXIT_FAILURE;
	}
	if ((pUBXDevice->bSaveRawData)&&(pUBXDevice->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, pUBXDevice->pfSaveFile);
		fflush(pUBXDevice->pfSaveFile);
	}

	// Should check ACK...

	return EXIT_SUCCESS;
}

// Transfer data (e.g. RTCM extracted from MAVLink) without any interpretation...
inline int TransferToUBXDevice(UBXDEVICE* pUBXDevice, unsigned char* sendbuf, int sendbuflen)
{
	if (WriteAllRS232Port(&pUBXDevice->RS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		printf("Error writing data to a UBXDevice. \n");
		return EXIT_FAILURE;
	}
	if ((pUBXDevice->bSaveRawData)&&(pUBXDevice->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, pUBXDevice->pfSaveFile);
		fflush(pUBXDevice->pfSaveFile);
	}

	return EXIT_SUCCESS;
}

// UBXDEVICE must be initialized to 0 before (e.g. UBXDEVICE ubxdevice; memset(&ubxdevice, 0, sizeof(UBXDEVICE));)!
inline int ConnectUBXDevice(UBXDEVICE* pUBXDevice, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];

	memset(pUBXDevice->szCfgFilePath, 0, sizeof(pUBXDevice->szCfgFilePath));
	sprintf(pUBXDevice->szCfgFilePath, "%.255s", szCfgFilePath);

	// If szCfgFilePath starts with "hardcoded://", parameters are assumed to be already set in the structure, 
	// otherwise it should be loaded from a configuration file.
	if (strncmp(szCfgFilePath, "hardcoded://", strlen("hardcoded://")) != 0)
	{
		memset(line, 0, sizeof(line));

		// Default values.
		memset(pUBXDevice->szDevPath, 0, sizeof(pUBXDevice->szDevPath));
		sprintf(pUBXDevice->szDevPath, "COM1");
		pUBXDevice->BaudRate = 9600;
		pUBXDevice->timeout = 1000;
		pUBXDevice->bSaveRawData = 1;
		pUBXDevice->bRevertToDefaultCfg = 1;
		pUBXDevice->bSetBaseCfg = 1;
		pUBXDevice->SurveyMode = 1;
		pUBXDevice->svinMinDur = 30;
		pUBXDevice->svinAccLimit = 10;
		pUBXDevice->fixedLat = 0;
		pUBXDevice->fixedLon = 0;
		pUBXDevice->fixedAlt = 0;
		pUBXDevice->fixedPosAcc = 10;

		// Load data from a file.
		file = fopen(szCfgFilePath, "r");
		if (file != NULL)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pUBXDevice->szDevPath) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pUBXDevice->BaudRate) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pUBXDevice->timeout) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pUBXDevice->bSaveRawData) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pUBXDevice->bRevertToDefaultCfg) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pUBXDevice->bSetBaseCfg) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pUBXDevice->SurveyMode) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pUBXDevice->svinMinDur) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pUBXDevice->svinAccLimit) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pUBXDevice->fixedLat) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pUBXDevice->fixedLon) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pUBXDevice->fixedAlt) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pUBXDevice->fixedPosAcc) != 1) printf("Invalid configuration file.\n");
			if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
		}
		else
		{
			printf("Configuration file not found.\n");
		}
	}

	// Used to save raw data, should be handled specifically...
	//pUBXDevice->pfSaveFile = NULL;

	memset(&pUBXDevice->LastUBXData, 0, sizeof(UBXDATA));

	if (OpenRS232Port(&pUBXDevice->RS232Port, pUBXDevice->szDevPath) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a UBXDevice.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsRS232Port(&pUBXDevice->RS232Port, pUBXDevice->BaudRate, NOPARITY, FALSE, 8, 
		ONESTOPBIT, (UINT)pUBXDevice->timeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a UBXDevice.\n");
		CloseRS232Port(&pUBXDevice->RS232Port);
		return EXIT_FAILURE;
	}

	if (pUBXDevice->bRevertToDefaultCfg)
	{
		if (RevertToDefaultCfgUBXDevice(pUBXDevice) != EXIT_SUCCESS)
		{
			printf("Unable to connect to a UBXDevice.\n");
			CloseRS232Port(&pUBXDevice->RS232Port);
			return EXIT_FAILURE;
		}
	}

	if (pUBXDevice->bSetBaseCfg)
	{
		if (SetBaseCfgUBXDevice(pUBXDevice) != EXIT_SUCCESS)
		{
			printf("Unable to connect to a UBXDevice.\n");
			CloseRS232Port(&pUBXDevice->RS232Port);
			return EXIT_FAILURE;
		}
	}

	printf("UBXDevice connected.\n");

	return EXIT_SUCCESS;
}

inline int DisconnectUBXDevice(UBXDEVICE* pUBXDevice)
{
	if (CloseRS232Port(&pUBXDevice->RS232Port) != EXIT_SUCCESS)
	{
		printf("UBXDevice disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("UBXDevice disconnected.\n");

	return EXIT_SUCCESS;
}

#ifndef DISABLE_UBXDEVICETHREAD
THREAD_PROC_RETURN_VALUE UBXDeviceThread(void* pParam);
#endif // DISABLE_UBXDEVICETHREAD

// Restore default alignment settings.
#pragma pack(pop) 

#endif // UBXDEVICE_H
