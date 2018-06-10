// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef UBXPROTOCOL_H
#define UBXPROTOCOL_H

#include "OSMisc.h"

// Need to be undefined at the end of the file...
// min and max might cause incompatibilities with GCC...
#ifndef _MSC_VER
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif // !max
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif // !min
#endif // !_MSC_VER

//#pragma pack(show)

// Check for potential paddings in bitfields and structs, check their size and the sum of the size of their fields!
// Potential solution if it is not the case : uncomment the padding fields in the structures, and for copy operations
// change the size to copy to the corresponding XXX_LEN constants when needed...

// To prevent unexpected padding in struct...
#pragma pack(push,1) 

#pragma region UBX-SPECIFIC DEFINITIONS
#define SYNC_CHAR_1_UBX 0xB5
#define SYNC_CHAR_2_UBX 0x62

// Unlikely to be more...
#define MAX_PACKET_LENGTH_UBX 2048

#define MIN_PACKET_LENGTH_UBX 8

#define NB_BYTES_CHECKSUM_UBX 2

#define DISABLED_SURVEY_RECEIVER_MODE_UBX 0
#define SELF_SURVEY_IN_RECEIVER_MODE_UBX 1
#define FIXED_POSITION_RECEIVER_MODE_UBX 2

#define NMEA_VERSION_4_1_UBX 0x41
#define NMEA_VERSION_4_0_UBX 0x40
#define NMEA_VERSION_2_3_UBX 0x23
#define NMEA_VERSION_2_1_UBX 0x21

#define NAV_CLASS_UBX 0x01
#define RXM_CLASS_UBX 0x02
#define INF_CLASS_UBX 0x04
#define ACK_CLASS_UBX 0x05
#define CFG_CLASS_UBX 0x06
#define UPD_CLASS_UBX 0x09
#define MON_CLASS_UBX 0x0A
#define AID_CLASS_UBX 0x0B
#define TIM_CLASS_UBX 0x0D
#define ESF_CLASS_UBX 0x10
#define MGA_CLASS_UBX 0x13
#define LOG_CLASS_UBX 0x21
#define SEC_CLASS_UBX 0x27
#define HNR_CLASS_UBX 0x28
#define NMEA_STD_CLASS_UBX 0xF0
#define NMEA_PUBX_CLASS_UBX 0xF1
#define RTCM_CLASS_UBX 0xF5

#define ACK_ACK_ID_UBX 0x01
#define ACK_NAK_ID_UBX 0x00

#define CFG_CFG_ID_UBX 0x09
#define CFG_INF_ID_UBX 0x02
#define CFG_MSG_ID_UBX 0x01
#define CFG_NAV5_ID_UBX 0x24
#define CFG_NMEA_ID_UBX 0x17
#define CFG_PRT_ID_UBX 0x00
#define CFG_RATE_ID_UBX 0x08
#define CFG_RST_ID_UBX 0x04
#define CFG_TMODE2_ID_UBX 0x3D
#define CFG_TMODE3_ID_UBX 0x71
#define CFG_USB_ID_UBX 0x1B

#define NAV_DGPS_ID_UBX 0x31
#define NAV_DOP_ID_UBX 0x04
#define NAV_POSECEF_ID_UBX 0x01
#define NAV_POSLLH_ID_UBX 0x02
#define NAV_PVT_ID_UBX 0x07
#define NAV_SOL_ID_UBX 0x06
#define NAV_STATUS_ID_UBX 0x03
#define NAV_SVIN_ID_UBX 0x3B
#define NAV_VELECEF_ID_UBX 0x11
#define NAV_VELNED_ID_UBX 0x12

#define TIM_SVIN_ID_UBX 0x04

#define NMEA_STD_GGA_ID_UBX 0x00
#define NMEA_STD_GLL_ID_UBX 0x01
#define NMEA_STD_GSA_ID_UBX 0x02
#define NMEA_STD_GSV_ID_UBX 0x03
#define NMEA_STD_RMC_ID_UBX 0x04
#define NMEA_STD_VTG_ID_UBX 0x05
#define NMEA_STD_GRS_ID_UBX 0x06
#define NMEA_STD_GST_ID_UBX 0x07
#define NMEA_STD_ZDA_ID_UBX 0x08
#define NMEA_STD_GBS_ID_UBX 0x09
#define NMEA_STD_DTM_ID_UBX 0x0A
#define NMEA_STD_GNS_ID_UBX 0x0D
#define NMEA_STD_VLW_ID_UBX 0x0F
//#define NMEA_STD_TXT_ID_UBX 0x41

#define RTCM_1005_ID_UBX 0x05
#define RTCM_1077_ID_UBX 0x4D
#define RTCM_1087_ID_UBX 0x57
#define RTCM_1127_ID_UBX 0x7F

union uShort_UBX
{
	unsigned short v;
	unsigned char c[2];
};
typedef union uShort_UBX uShort_UBX;

#define LEN_ACK_ACK_PL_UBX 2
struct ACK_ACK_PL_UBX
{
	unsigned char clsID;
	unsigned char msgID;	
	//unsigned char padding[6];
};
#define LEN_ACK_NAK_PL_UBX 2
struct ACK_NAK_PL_UBX
{
	unsigned char clsID;
	unsigned char msgID;	
	//unsigned char padding[6];
};
// 32 bits = 1*sizeof(unsigned int).
struct actionMask_CFG_CFG_PL_UBX
{
	unsigned int ioPort : 1;
	unsigned int msgConf : 1;
	unsigned int infMsg : 1;
	unsigned int navConf : 1;
	unsigned int rxmConf : 1;
	unsigned int reserved1 : 3;
	unsigned int senConf : 1;
	unsigned int rinvConf : 1;
	unsigned int antConf : 1;
	unsigned int logConf : 1;
	unsigned int ftsConf : 1;
	unsigned int reserved2 : 19;
};
// 8 bits = 1*sizeof(unsigned char).
struct deviceMask_CFG_CFG_PL_UBX
{
	unsigned char devBBR : 1;
	unsigned char devFlash : 1;
	unsigned char devEEPROM : 1;
	unsigned char reserved1 : 1;
	unsigned char devSpiFlash : 1;
	unsigned char reserved2 : 3;
};
#define LEN_CFG_CFG_PL_UBX 13
struct CFG_CFG_PL_UBX
{
	struct actionMask_CFG_CFG_PL_UBX clearMask;
	struct actionMask_CFG_CFG_PL_UBX saveMask;
	struct actionMask_CFG_CFG_PL_UBX loadMask;
	struct deviceMask_CFG_CFG_PL_UBX deviceMask;
	//unsigned char padding[3];
};
//inline int cfg_cfg_pl_pack_UBX(unsigned char* dest, struct CFG_CFG_PL_UBX src)
//{
//	dest[0] = ;
//	dest[1] = ;
//	dest[2] = ;
//	return 13;
//}
#define LEN_CFG_MSG_PL_UBX 8
struct CFG_MSG_PL_UBX
{
	unsigned char msgClass;
	unsigned char msgID;
	unsigned char rate[6]; // I2C, UART1, UART2, USB, SPI, unknown.
};
// 16 bits = 1*sizeof(unsigned short).
struct flags_CFG_TMODE3_PL_UBX
{
	unsigned short mode : 8;
	unsigned short lla : 1;
	unsigned short reserved1 : 7;
};
#define LEN_CFG_TMODE3_PL_UBX 40
struct CFG_TMODE3_PL_UBX
{
	unsigned char version;
	unsigned char reserved1;
	struct flags_CFG_TMODE3_PL_UBX flags;
	int ecefXOrLat;
	int ecefYOrLon;
	int ecefZOrAlt;
	char ecefXOrLatHP;
	char ecefYOrLonHP;
	char ecefZOrAltHP;
	unsigned char reserved2;
	unsigned int fixedPosAcc;
	unsigned int svinMinDur;
	unsigned int svinAccLimit;
	unsigned char reserved3[8];
};
// 8 bits = 1*sizeof(unsigned char).
struct filter_CFG_NMEA_PL_UBX
{
	unsigned char posFilt : 1;
	unsigned char mskPosFilt : 1;
	unsigned char timeFilt : 1;
	unsigned char dateFilt : 1;
	unsigned char gpsOnlyFilter : 1;
	unsigned char trackFilt : 1;
	unsigned char reserved1 : 2;
};
// 8 bits = 1*sizeof(unsigned char).
struct flags_CFG_NMEA_PL_UBX
{
	unsigned char compat : 1;
	unsigned char consider : 1;
	unsigned char limit82 : 1;
	unsigned char highPrec : 1;
	unsigned char reserved1 : 4;
};
// 32 bits = 1*sizeof(unsigned int).
struct gnssToFilter_CFG_NMEA_PL_UBX
{
	unsigned int gps : 1;
	unsigned int sbas : 1;
	unsigned int reserved1 : 2;
	unsigned int qzss : 1;
	unsigned int glonass : 1;
	unsigned int beidou : 1;
	unsigned int reserved2 : 25;
};
#define LEN_CFG_NMEA_PL_UBX 20
struct CFG_NMEA_PL_UBX
{
	struct filter_CFG_NMEA_PL_UBX filter;
	unsigned char nmeaVersion;
	unsigned char numSV;
	struct flags_CFG_NMEA_PL_UBX flags;
	struct gnssToFilter_CFG_NMEA_PL_UBX gnssToFilter;
	unsigned char svNumbering;
	unsigned char mainTalkerId;
	unsigned char gsvTalkerId;
	unsigned char version;
	char bdsTalkerId[2];
	unsigned char reserved1[6];
	//unsigned char padding[4];
};
#define LEN_NAV_POSLLH_PL_UBX 28
struct NAV_POSLLH_PL_UBX
{
	unsigned int iTOW; // In ms.
	int lon; // In 1e-7 deg.
	int lat; // In 1e-7 deg.
	int height; // In mm.
	int hMSL; // In mm.
	unsigned int hAcc; // In mm.
	unsigned int vAcc; // In mm.
	//unsigned char padding[4];
};
// 8 bits = 1*sizeof(unsigned char).
struct valid_NAV_PVT_PL_UBX
{
	unsigned char validDate : 1;
	unsigned char validTime : 1;
	unsigned char fullyResolved : 1;
	unsigned char reserved1 : 5;
};
// 8 bits = 1*sizeof(unsigned char).
struct flags_NAV_PVT_PL_UBX
{
	unsigned char gnssFixOK : 1;
	unsigned char diffSoln : 1;
	unsigned char psmState : 3;
	unsigned char headVehValid : 1;
	unsigned char carrSoln : 2;
};
// 8 bits = 1*sizeof(unsigned char).
struct flags2_NAV_PVT_PL_UBX
{
	unsigned char reserved1 : 5;
	unsigned char confirmedAvai : 1;
	unsigned char confirmedDate : 1;
	unsigned char confirmedTime : 1;
};
#define LEN_NAV_PVT_PL_UBX 92
struct NAV_PVT_PL_UBX
{
	unsigned int iTOW; // In ms.
	unsigned short year;
	unsigned char month;
	unsigned char day;
	unsigned char hour;
	unsigned char minute;
	unsigned char sec; // In s.
	struct valid_NAV_PVT_PL_UBX valid;
	unsigned int tAcc; // In ns.
	int nano; // In ns.
	unsigned char fixType;
	struct flags_NAV_PVT_PL_UBX flags;
	struct flags2_NAV_PVT_PL_UBX flags2;
	unsigned char numSV;
	int lon; // In 1e-7 deg.
	int lat; // In 1e-7 deg.
	int height; // In mm.
	int hMSL; // In mm.
	unsigned int hAcc; // In mm.
	unsigned int vAcc; // In mm.
	int velN; // In mm/s.
	int velE; // In mm/s.
	int velD; // In mm/s.
	int gSpeed; // In mm/s.
	int headMot; // In 1e-5 deg.
	unsigned int sAcc; // In mm/s.
	unsigned int headAcc; // In 1e-5 deg.
	unsigned short pDOP; // In 0.01.
	unsigned char reserved1[6];
	int headVeh; // In 1e-5 deg.
	unsigned char reserved2[4];
	//unsigned char padding[4];
};
// 8 bits = 1*sizeof(unsigned char).
struct flags_NAV_SOL_PL_UBX
{
	unsigned char GPSFixOK : 1;
	unsigned char DiffSoln : 1;
	unsigned char WKNSET : 1;
	unsigned char TOWSET : 1;
	unsigned char reserved1 : 4;
};
#define LEN_NAV_SOL_PL_UBX 52
struct NAV_SOL_PL_UBX
{
	unsigned int iTOW; // In ms.
	int fTOW; // In ns.
	short week;
	unsigned char gpsFix;
	struct flags_NAV_SOL_PL_UBX flags;
	int ecefX; // In cm.
	int ecefY; // In cm.
	int ecefZ; // In cm.
	unsigned int pAcc; // In cm.
	int ecefVX; // In cm/s.
	int ecefVY; // In cm/s.
	int ecefVZ; // In cm/s.
	unsigned int sAcc; // In cm/s.
	unsigned short pDOP; // In 0.01.
	unsigned char reserved1;
	unsigned char numSV;
	unsigned char reserved2[4];
	//unsigned char padding[4];
};
// 8 bits = 1*sizeof(unsigned char).
struct flags_NAV_STATUS_PL_UBX
{
	unsigned char gpsFixOK : 1;
	unsigned char diffSoln : 1;
	unsigned char wknSet : 1;
	unsigned char towSet : 1;
	unsigned char reserved1 : 4;
};
// 8 bits = 1*sizeof(unsigned char).
struct fixStat_NAV_STATUS_PL_UBX
{
	unsigned char diffCorr : 1;
	unsigned char reserved1 : 5;
	unsigned char mapMatching : 2;
};
// 8 bits = 1*sizeof(unsigned char).
struct flags2_NAV_STATUS_PL_UBX
{
	unsigned char psmState : 2;
	unsigned char reserved1 : 1;
	unsigned char spoofDetState : 2;
	unsigned char reserved2 : 3;
};
#define LEN_NAV_STATUS_PL_UBX 16
struct NAV_STATUS_PL_UBX
{
	unsigned int iTOW; // In ms.
	unsigned char gpsFix;
	struct flags_NAV_STATUS_PL_UBX flags;
	struct fixStat_NAV_STATUS_PL_UBX fixStat;
	struct flags2_NAV_STATUS_PL_UBX flags2;
	unsigned int ttff; // In ms.
	unsigned int msss; // In ms.
};
#define LEN_NAV_SVIN_PL_UBX 40
struct NAV_SVIN_PL_UBX
{
	unsigned char version;
	unsigned char reserved1[3];
	unsigned int iTOW; // In ms.
	unsigned int dur; // In s.
	int meanX; // In cm.
	int meanY; // In cm.
	int meanZ; // In cm.
	char meanXHP; // In 0.1 mm.
	char meanYHP; // In 0.1 mm.
	char meanZHP; // In 0.1 mm.
	unsigned char reserved2;
	unsigned int meanAcc; // In 0.1 mm.
	unsigned int obs;
	unsigned char valid;
	unsigned char active;
	unsigned char reserved3[2];
};
#define LEN_NAV_VELNED_PL_UBX 36
struct NAV_VELNED_PL_UBX
{
	unsigned int iTOW; // In ms.
	int velN; // In cm/s.
	int velE; // In cm/s.
	int velD; // In cm/s.
	unsigned int speed; // In cm/s.
	unsigned int gSpeed; // In cm/s.
	int heading; // In 1e-5 deg.
	unsigned int sAcc; // In cm/s.
	unsigned int cAcc; // In 1e-5 deg.
	//unsigned char padding[4];
};
#pragma endregion

struct UBXDATA
{
	struct NAV_POSLLH_PL_UBX nav_posllh_pl;
	struct NAV_PVT_PL_UBX nav_pvt_pl;
	struct NAV_SOL_PL_UBX nav_sol_pl;
	struct NAV_STATUS_PL_UBX nav_status_pl;
	struct NAV_SVIN_PL_UBX nav_svin_pl;
	struct NAV_VELNED_PL_UBX nav_velned_pl;
	double Latitude; // In decimal degrees.
	double Longitude; // In decimal degrees.
	double Altitude; // In m.
	double SOG; // In m/s.
	double COG; // In rad.
	int year, month, day, hour, minute; 
	double second;
	double Heading; // In rad.
};
typedef struct UBXDATA UBXDATA;

// packet must contain a valid packet of packetlen bytes.
inline void SetChecksumUBX(unsigned char* packet, int packetlen)
{
	int i = 0;
	unsigned char CK_A = 0, CK_B = 0;

	for (i = 2; i < packetlen-2; i++)
	{
		CK_A = (unsigned char)(CK_A + packet[i]);
		CK_B = (unsigned char)(CK_B + CK_A);
	}
	packet[packetlen-2] = CK_A;
	packet[packetlen-1] = CK_B;
}

// packet must contain a valid packet of packetlen bytes.
inline int CheckChecksumUBX(unsigned char* packet, int packetlen)
{
	int i = 0;
	unsigned char CK_A = 0, CK_B = 0;

	for (i = 2; i < packetlen-2; i++)
	{
		CK_A = (unsigned char)(CK_A + packet[i]);
		CK_B = (unsigned char)(CK_B + CK_A);
	}
	if ((packet[packetlen-2] != CK_A)||(packet[packetlen-1] != CK_B))
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

// buf must contain the beginning of a valid packet of at least MIN_PACKET_LENGTH_UBX-NB_BYTES_CHECKSUM_UBX bytes.
inline int GetPacketLengthUBX(unsigned char* buf)
{
	uShort_UBX len;

	len.c[0] = buf[4];
	len.c[1] = buf[5];
	return len.v+MIN_PACKET_LENGTH_UBX;
}

inline void EncodePacketUBX(unsigned char* packet, int* ppacketlen, int mclass, int mid, unsigned char* payload, int payloadlen)
{
	uShort_UBX len;

	packet[0] = SYNC_CHAR_1_UBX;
	packet[1] = SYNC_CHAR_2_UBX;
	packet[2] = (unsigned char)mclass;
	packet[3] = (unsigned char)mid;
	len.v = (unsigned short)payloadlen;
	packet[4] = len.c[0];
	packet[5] = len.c[1];
	memcpy(&packet[MIN_PACKET_LENGTH_UBX-NB_BYTES_CHECKSUM_UBX], payload, payloadlen);
	*ppacketlen = payloadlen+MIN_PACKET_LENGTH_UBX;
	SetChecksumUBX(packet, *ppacketlen);
}

/*
Return : EXIT_SUCCESS if the beginning of buf contains a valid packet (there might be other data at the end), 
EXIT_OUT_OF_MEMORY if the packet is incomplete (check *pnbBytesToRequest to know how many additional bytes 
should be requested, -1 if unknown) or EXIT_FAILURE if there is an error (check *pnbBytesToDiscard to know how 
many bytes can be safely discarded).
*/
inline int AnalyzePacketUBX(unsigned char* buf, int buflen, int* pmclass, int* pmid, int* ppacketlen, 
							int* pnbBytesToRequest, int* pnbBytesToDiscard)
{
	*ppacketlen = 0;
	*pnbBytesToRequest = -1;
	*pnbBytesToDiscard = 0;
	if (buflen < MIN_PACKET_LENGTH_UBX)
	{
		*pnbBytesToRequest = MIN_PACKET_LENGTH_UBX-buflen;
		return EXIT_OUT_OF_MEMORY;
	}
	if ((buf[0] != SYNC_CHAR_1_UBX)||(buf[1] != SYNC_CHAR_2_UBX))
	{
		*pnbBytesToDiscard = 1; // We are only sure that the first sync byte can be discarded...
		return EXIT_FAILURE;
	}
	*pmclass = buf[2];
	*pmid = buf[3];
	*ppacketlen = GetPacketLengthUBX(buf);	
	if (buflen < *ppacketlen)
	{
		*pnbBytesToRequest = *ppacketlen-buflen;
		return EXIT_OUT_OF_MEMORY;
	}
	if (CheckChecksumUBX(buf, *ppacketlen) != EXIT_SUCCESS)
	{ 
		printf("Warning : UBX checksum error. \n");
		*pnbBytesToDiscard = 2; // We are only sure that the 2 sync bytes can be discarded...
		return EXIT_FAILURE;	
	}

	return EXIT_SUCCESS;
}

/*
Return : EXIT_SUCCESS if the beginning of *pFoundPacket contains a valid packet (there might be other data 
at the end), 
EXIT_OUT_OF_MEMORY if the packet is incomplete (check *pnbBytesToRequest to know how many additional bytes 
should be requested, -1 if unknown) or EXIT_FAILURE if no compatible packet could be found. 
Data in the beginning of buf might have been discarded (check *pnbBytesDiscarded to know how many bytes were discarded).
*/
inline int FindPacketUBX(unsigned char* buf, int buflen, int* pmclass, int* pmid, int* ppacketlen, 
						 int* pnbBytesToRequest, unsigned char** pFoundPacket, int* pnbBytesDiscarded)
{
	int res = EXIT_FAILURE, nbBytesToRequest = -1, nbBytesToDiscard = 0;

	*pnbBytesToRequest = -1;
	*pFoundPacket = buf;
	*pnbBytesDiscarded = 0;

	for (;;)
	{
		res = AnalyzePacketUBX(*pFoundPacket, buflen-(*pnbBytesDiscarded), pmclass, pmid, ppacketlen, &nbBytesToRequest, &nbBytesToDiscard);
		switch (res)
		{
		case EXIT_SUCCESS:
			return EXIT_SUCCESS;
		case EXIT_OUT_OF_MEMORY:
			(*pnbBytesToRequest) = nbBytesToRequest;
			return EXIT_OUT_OF_MEMORY;
		default:
			(*pFoundPacket) += nbBytesToDiscard;
			(*pnbBytesDiscarded) += nbBytesToDiscard;
			if (buflen-(*pnbBytesDiscarded) <= 0)
			{
				*pFoundPacket = NULL;
				return EXIT_FAILURE;
			}
			break;
		}
	} 
}

/*
This function is probably not really useful in practice...

Return : EXIT_SUCCESS if the beginning of *pFoundPacket contains the latest valid packet (there might be other data 
at the end), EXIT_OUT_OF_MEMORY if the packet is incomplete (check *pnbBytesToRequest to know how many additional bytes 
should be requested, -1 if unknown) or EXIT_FAILURE if no compatible packet could be found. 
Data in the beginning of buf might have been discarded, including valid packets (check *pnbBytesDiscarded 
to know how many bytes were discarded).
*/
inline int FindLatestPacketUBX(unsigned char* buf, int buflen, int* pmclass, int* pmid, int* ppacketlen, 
									  int* pnbBytesToRequest, unsigned char** pFoundPacket, int* pnbBytesDiscarded)
{
	unsigned char* ptr = NULL;
	int res = EXIT_FAILURE, nbBytesDiscarded = 0, packetlen = 0, nbBytesToRequest = 0;

	*pnbBytesToRequest = -1;
	*pnbBytesDiscarded = 0;

	res = FindPacketUBX(buf, buflen, pmclass, pmid, &packetlen, &nbBytesToRequest, &ptr, &nbBytesDiscarded);
	// Save the position of the beginning of the packet and its length.
	*pFoundPacket = ptr;
	*ppacketlen = packetlen;
	switch (res)
	{
	case EXIT_SUCCESS:
		(*pnbBytesDiscarded) += nbBytesDiscarded;
		break;
	case EXIT_OUT_OF_MEMORY:
		(*pnbBytesToRequest) = nbBytesToRequest;
		(*pnbBytesDiscarded) += nbBytesDiscarded;
		return EXIT_OUT_OF_MEMORY;
	default:
		*pFoundPacket = NULL;
		(*pnbBytesDiscarded) += nbBytesDiscarded;
		return EXIT_FAILURE;
	}

	for (;;) 
	{
		// Save the position of the beginning of the packet and its length.
		*pFoundPacket = ptr;
		*ppacketlen = packetlen;

		// Search just after the packet.
		res = FindPacketUBX(*pFoundPacket+*ppacketlen, buflen-*ppacketlen-(*pnbBytesDiscarded), 
			pmclass, pmid, &packetlen, &nbBytesToRequest, &ptr, &nbBytesDiscarded);
		switch (res)
		{
		case EXIT_SUCCESS:
			(*pnbBytesDiscarded) += (*ppacketlen+nbBytesDiscarded);
			break;
		case EXIT_OUT_OF_MEMORY:
		default:
			return EXIT_SUCCESS;
		}
	}
}

/*
Return : EXIT_SUCCESS if the beginning of buf contains a valid packet (there might be other data at the end), 
EXIT_OUT_OF_MEMORY if the packet is incomplete (check *pnbBytesToRequest to know how many additional bytes 
should be requested, -1 if unknown) or EXIT_FAILURE if there is an error (check *pnbBytesToDiscard to know how 
many bytes can be safely discarded).
*/
inline int AnalyzePacketWithMIDUBX(unsigned char* buf, int buflen, int mclass, int mid, int* ppacketlen, 
								   int* pnbBytesToRequest, int* pnbBytesToDiscard)
{
	*ppacketlen = 0;
	*pnbBytesToRequest = -1;
	*pnbBytesToDiscard = 0;
	if (buflen < MIN_PACKET_LENGTH_UBX)
	{
		*pnbBytesToRequest = MIN_PACKET_LENGTH_UBX-buflen;
		return EXIT_OUT_OF_MEMORY;
	}
	if ((buf[0] != SYNC_CHAR_1_UBX)||(buf[1] != SYNC_CHAR_2_UBX))
	{
		*pnbBytesToDiscard = 1; // We are only sure that the first sync byte can be discarded...
		return EXIT_FAILURE;
	}
	if ((buf[2] != mclass)||(buf[3] != mid))
	{
		*pnbBytesToDiscard = 2; // We are only sure that the 2 sync bytes can be discarded...
		return EXIT_FAILURE;
	}
	*ppacketlen = GetPacketLengthUBX(buf);	
	if (buflen < *ppacketlen)
	{
		*pnbBytesToRequest = *ppacketlen-buflen;
		return EXIT_OUT_OF_MEMORY;
	}
	if (CheckChecksumUBX(buf, *ppacketlen) != EXIT_SUCCESS)
	{ 
		printf("Warning : UBX checksum error. \n");
		*pnbBytesToDiscard = 2; // We are only sure that the 2 sync bytes can be discarded...
		return EXIT_FAILURE;	
	}

	return EXIT_SUCCESS;
}

/*
Return : EXIT_SUCCESS if the beginning of *pFoundPacket contains a valid packet (there might be other data 
at the end), 
EXIT_OUT_OF_MEMORY if the packet is incomplete (check *pnbBytesToRequest to know how many additional bytes 
should be requested, -1 if unknown) or EXIT_FAILURE if no compatible packet could be found. 
Data in the beginning of buf might have been discarded (check *pnbBytesDiscarded to know how many bytes were discarded).
*/
inline int FindPacketWithMIDUBX(unsigned char* buf, int buflen, int mclass, int mid, int* ppacketlen, 
								int* pnbBytesToRequest, unsigned char** pFoundPacket, int* pnbBytesDiscarded)
{
	int res = EXIT_FAILURE, nbBytesToRequest = -1, nbBytesToDiscard = 0;

	*pnbBytesToRequest = -1;
	*pFoundPacket = buf;
	*pnbBytesDiscarded = 0;

	for (;;)
	{
		res = AnalyzePacketWithMIDUBX(*pFoundPacket, buflen-(*pnbBytesDiscarded), mclass, mid, ppacketlen, &nbBytesToRequest, &nbBytesToDiscard);
		switch (res)
		{
		case EXIT_SUCCESS:
			return EXIT_SUCCESS;
		case EXIT_OUT_OF_MEMORY:
			(*pnbBytesToRequest) = nbBytesToRequest;
			return EXIT_OUT_OF_MEMORY;
		default:
			(*pFoundPacket) += nbBytesToDiscard;
			(*pnbBytesDiscarded) += nbBytesToDiscard;
			if (buflen-(*pnbBytesDiscarded) <= 0)
			{
				*pFoundPacket = NULL;
				return EXIT_FAILURE;
			}
			break;
		}
	} 
}

/*
Return : EXIT_SUCCESS if the beginning of *pFoundPacket contains the latest valid packet (there might be other data 
at the end), EXIT_OUT_OF_MEMORY if the packet is incomplete (check *pnbBytesToRequest to know how many additional bytes 
should be requested, -1 if unknown) or EXIT_FAILURE if no compatible packet could be found. 
Data in the beginning of buf might have been discarded, including valid packets (check *pnbBytesDiscarded 
to know how many bytes were discarded).
*/
inline int FindLatestPacketWithMIDUBX(unsigned char* buf, int buflen, int mclass, int mid, int* ppacketlen, 
									  int* pnbBytesToRequest, unsigned char** pFoundPacket, int* pnbBytesDiscarded)
{
	unsigned char* ptr = NULL;
	int res = EXIT_FAILURE, nbBytesDiscarded = 0, packetlen = 0, nbBytesToRequest = 0;

	*pnbBytesToRequest = -1;
	*pnbBytesDiscarded = 0;

	res = FindPacketWithMIDUBX(buf, buflen, mclass, mid, &packetlen, &nbBytesToRequest, &ptr, &nbBytesDiscarded);
	// Save the position of the beginning of the packet and its length.
	*pFoundPacket = ptr;
	*ppacketlen = packetlen;
	switch (res)
	{
	case EXIT_SUCCESS:
		(*pnbBytesDiscarded) += nbBytesDiscarded;
		break;
	case EXIT_OUT_OF_MEMORY:
		(*pnbBytesToRequest) = nbBytesToRequest;
		(*pnbBytesDiscarded) += nbBytesDiscarded;
		return EXIT_OUT_OF_MEMORY;
	default:
		*pFoundPacket = NULL;
		(*pnbBytesDiscarded) += nbBytesDiscarded;
		return EXIT_FAILURE;
	}

	for (;;) 
	{
		// Save the position of the beginning of the packet and its length.
		*pFoundPacket = ptr;
		*ppacketlen = packetlen;

		// Search just after the packet.
		res = FindPacketWithMIDUBX(*pFoundPacket+*ppacketlen, buflen-*ppacketlen-(*pnbBytesDiscarded), 
			mclass, mid, &packetlen, &nbBytesToRequest, &ptr, &nbBytesDiscarded);
		switch (res)
		{
		case EXIT_SUCCESS:
			(*pnbBytesDiscarded) += (*ppacketlen+nbBytesDiscarded);
			break;
		case EXIT_OUT_OF_MEMORY:
		default:
			return EXIT_SUCCESS;
		}
	}
}

// packet must contain a valid packet of packetlen bytes.
inline int ProcessPacketUBX(unsigned char* packet, int packetlen, int mclass, int mid, UBXDATA* pUBXData)
{
	unsigned char* payload = packet+MIN_PACKET_LENGTH_UBX-NB_BYTES_CHECKSUM_UBX;
	int payloadlen = packetlen-MIN_PACKET_LENGTH_UBX;

	UNREFERENCED_PARAMETER(payloadlen);

	//memset(pUBXData, 0, sizeof(UBXDATA));

	switch (mclass)
	{
	case ACK_CLASS_UBX:
		switch (mid)
		{
		case ACK_ACK_ID_UBX:
			break;
		case ACK_NAK_ID_UBX:
			printf("UBX packet not acknowledged : mclass = %02x, mid = %02x. \n", (int)(unsigned char)mclass, (int)(unsigned char)mid);
			return EXIT_FAILURE;
		default:
			// Unhandled...
			break;
		}
		break;
	case NAV_CLASS_UBX:
		switch (mid)
		{
		case NAV_POSLLH_ID_UBX:
			memcpy(&pUBXData->nav_posllh_pl, payload, LEN_NAV_POSLLH_PL_UBX);
			//memcpy(&pUBXData->nav_posllh_pl, payload, sizeof(pUBXData->nav_posllh_pl));
			pUBXData->Latitude = pUBXData->nav_posllh_pl.lat*0.0000001; // In decimal degrees.
			pUBXData->Longitude = pUBXData->nav_posllh_pl.lon*0.0000001; // In decimal degrees.
			pUBXData->Altitude = pUBXData->nav_posllh_pl.height*1000; // In m.
			break;
		case NAV_PVT_ID_UBX:
			memcpy(&pUBXData->nav_pvt_pl, payload, LEN_NAV_PVT_PL_UBX);
			//memcpy(&pUBXData->nav_pvt_pl, payload, sizeof(pUBXData->nav_pvt_pl));
			pUBXData->Latitude = pUBXData->nav_pvt_pl.lat*0.0000001; // In decimal degrees.
			pUBXData->Longitude = pUBXData->nav_pvt_pl.lon*0.0000001; // In decimal degrees.
			pUBXData->Altitude = pUBXData->nav_pvt_pl.height*1000; // In m.
			pUBXData->year = pUBXData->nav_pvt_pl.year;
			pUBXData->month = pUBXData->nav_pvt_pl.month;
			pUBXData->day = pUBXData->nav_pvt_pl.day;
			pUBXData->hour = pUBXData->nav_pvt_pl.hour;
			pUBXData->minute = pUBXData->nav_pvt_pl.minute; 
			pUBXData->second = pUBXData->nav_pvt_pl.sec+0.000000001*pUBXData->nav_pvt_pl.nano;	
			pUBXData->SOG = pUBXData->nav_pvt_pl.gSpeed/1000.0; // In m/s.
			pUBXData->COG = pUBXData->nav_pvt_pl.headMot*0.00001*M_PI/180.0; // In rad.
			break;
		case NAV_SOL_ID_UBX:
			memcpy(&pUBXData->nav_sol_pl, payload, LEN_NAV_SOL_PL_UBX);
			//memcpy(&pUBXData->nav_sol_pl, payload, sizeof(pUBXData->nav_sol_pl));
			break;
		case NAV_STATUS_ID_UBX:
			memcpy(&pUBXData->nav_status_pl, payload, LEN_NAV_STATUS_PL_UBX);
			//memcpy(&pUBXData->nav_status_pl, payload, sizeof(pUBXData->nav_status_pl));
			break;
		case NAV_SVIN_ID_UBX:
			memcpy(&pUBXData->nav_svin_pl, payload, LEN_NAV_SVIN_PL_UBX);
			//memcpy(&pUBXData->nav_svin_pl, payload, sizeof(pUBXData->nav_svin_pl));
			break;
		case NAV_VELNED_ID_UBX:
			memcpy(&pUBXData->nav_velned_pl, payload, LEN_NAV_VELNED_PL_UBX);
			//memcpy(&pUBXData->nav_velned_pl, payload, sizeof(pUBXData->nav_velned_pl));
			pUBXData->SOG = pUBXData->nav_velned_pl.gSpeed/100.0; // In m/s.
			pUBXData->COG = pUBXData->nav_velned_pl.heading*0.00001*M_PI/180.0; // In rad.
			break;
		default:
			// Unhandled...
			break;
		}
		break;
	default:
		// Unhandled...
		break;
	}

	return EXIT_SUCCESS;
}

// Restore default alignment settings.
#pragma pack(pop) 

// min and max might cause incompatibilities with GCC...
#ifndef _MSC_VER
#ifdef max
#undef max
#endif // max
#ifdef min
#undef min
#endif // min
#endif // !_MSC_VER

#endif // !UBXPROTOCOL_H
