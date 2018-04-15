// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef RPLIDAR_H
#define RPLIDAR_H

#include "OSMisc.h"
#include "RS232Port.h"

#ifndef DISABLE_RPLIDARTHREAD
#include "OSThread.h"
#endif // !DISABLE_RPLIDARTHREAD

#define TIMEOUT_MESSAGE_RPLIDAR 4.0 // In s.
// Should be at least 2 * number of bytes to be sure to contain entirely the biggest desired message (or group of messages) + 1.
#define MAX_NB_BYTES_RPLIDAR 1024

#define MIN_BUF_LEN_RPLIDAR 2

#define NB_BYTES_CHECKSUM_RPLIDAR 1

#define START_FLAG1_RPLIDAR 0xA5
#define START_FLAG2_RPLIDAR 0x5A

#define SEND_MODE_MASK_RPLIDAR 0x03
#define DATA_RESPONSE_LENGTH_MASK_RPLIDAR (!SEND_MODE_MASK_RPLIDAR)
#define DATA_RESPONSE_LENGTH_SHIFT_RPLIDAR 2

// Single Request-Single Response Mode
// Single Request-Multiple Response Mode
// Single Request-No Response : need to wait a little bit after the request wince there will be no response...
#define SINGLE_REQUEST_SINGLE_RESPONSE_SEND_MODE_RPLIDAR 0x00
#define SINGLE_REQUEST_MULTIPLE_RESPONSE_SEND_MODE_RPLIDAR 0x01

// Commands without payload and response.
#define STOP_REQUEST_RPLIDAR 0x25
#define SCAN_REQUEST_RPLIDAR 0x20
#define FORCE_SCAN_REQUEST_RPLIDAR 0x21
#define RESET_REQUEST_RPLIDAR 0x40

// Commands without payload but have response
#define GET_INFO_REQUEST_RPLIDAR 0x50
#define GET_HEALTH_REQUEST_RPLIDAR 0x52
#define GET_SAMPLERATE_REQUEST_RPLIDAR 0x59 // Added in FW ver 1.17.

// Commands with payload and have response
#define EXPRESS_SCAN_REQUEST_RPLIDAR 0x82 // Added in FW ver 1.17.

// Undocumented...
// Commands added for A2 to set RPLIDAR motor PWM when using accessory board.
#define SET_MOTOR_PWM_REQUEST_RPLIDAR 0xF0
#define GET_ACC_BOARD_FLAG_REQUEST_RPLIDAR 0xFF

// Responses.
#define DEVINFO_RESPONSE_RPLIDAR 0x04
#define DEVHEALTH_RESPONSE_RPLIDAR 0x06
#define MEASUREMENT_RESPONSE_RPLIDAR 0x81
#define MEASUREMENT_CAPSULED_RESPONSE_RPLIDAR 0x82 // Added in FW ver 1.17.
#define SAMPLERATE_RESPONSE_RPLIDAR 0x15 // Added in FW ver 1.17.
#define ACC_BOARD_FLAG_RESPONSE_RPLIDAR 0xFF

#define NB_BYTES_SERIAL_NUMBER_RPLIDAR 16
#define MAX_BUF_LEN_SERIAL_NUMBER_RPLIDAR (2*NB_BYTES_SERIAL_NUMBER_RPLIDAR+1)

#define STATUS_OK_RPLIDAR 0x00
#define STATUS_WARNING_RPLIDAR 0x01
#define STATUS_ERROR_RPLIDAR 0x02

// Undocumented...
#define MAX_MOTOR_PWM_RPLIDAR 1023
#define DEFAULT_MOTOR_PWM_RPLIDAR 660

#define SCAN_MODE_RPLIDAR 0
#define EXPRESS_SCAN_MODE_RPLIDAR 1
#define FORCE_SCAN_MODE_RPLIDAR 2

#define CHECK_BIT_MASK_SCAN_DATA_RESPONSE_RPLIDAR 0x01
#define START_BIT_MASK_SCAN_DATA_RESPONSE_RPLIDAR 0x01
#define INVERTED_START_BIT_MASK_SCAN_DATA_RESPONSE_RPLIDAR 0x02

#define NB_BYTES_EXPRESS_SCAN_DATA_RESPONSE_RPLIDAR 84
#define NB_CABINS_EXPRESS_SCAN_DATA_RESPONSE_RPLIDAR 16
#define NB_MEASUREMENTS_EXPRESS_SCAN_DATA_RESPONSE_RPLIDAR (2*NB_CABINS_EXPRESS_SCAN_DATA_RESPONSE_RPLIDAR)

#define MAX_NB_MEASUREMENTS_PER_SCAN_RPLIDAR 2048

struct RPLIDAR
{
	RS232PORT RS232Port;
	int model;
	int hardware;
	int firmware_major;
	int firmware_minor;
	char SerialNumber[MAX_BUF_LEN_SERIAL_NUMBER_RPLIDAR];
	int Tstandard; // Time in us used when RPLIDAR takes a single laser ranging in SCAN mode.
	int Texpress; // Time in us used when RPLIDAR takes a single laser ranging in EXPRESS_SCAN mode.
	unsigned char esdata_prev[NB_BYTES_EXPRESS_SCAN_DATA_RESPONSE_RPLIDAR];
	FILE* pfSaveFile; // Used to save raw data, should be handled specifically...
	//RPLIDARDATA LastRPLIDARData;
	char szCfgFilePath[256];
	// Parameters.
	char szDevPath[256];
	int BaudRate;
	int timeout;
	BOOL bSaveRawData;
	int ScanMode;
	int motordelay;
	int maxhist;
	double alpha_max_err;
	double d_max_err;
};
typedef struct RPLIDAR RPLIDAR;

inline double AngleDiffRPLIDAR(double start_angle_1, double start_angle_2)
{
	if (start_angle_1 <= start_angle_2) return start_angle_2-start_angle_1;
	else return 360+start_angle_2-start_angle_1;
}

// req must contain a valid request of reqlen bytes.
inline unsigned char ComputeChecksumRPLIDAR(unsigned char* req, int reqlen)
{
	int i = 0;
	unsigned char res = 0;

	while (i < reqlen-NB_BYTES_CHECKSUM_RPLIDAR)
	{
		res ^= req[i];
		i++;
	}
	return res;
}

// req must contain a valid request of reqlen bytes.
inline int GetSendModeFromResponseDescriptorRPLIDAR(unsigned char* req, int reqlen)
{
	UNREFERENCED_PARAMETER(reqlen);
	return (SEND_MODE_MASK_RPLIDAR & req[2]);
}

// req must contain a valid request of reqlen bytes.
inline int GetDataResponseLengthFromResponseDescriptorRPLIDAR(unsigned char* req, int reqlen)
{
	int len = 0;
	UNREFERENCED_PARAMETER(reqlen);
	len = ((req[5]<<24)|(req[4]<<16)|(req[3]<<8)|(req[2]))>>DATA_RESPONSE_LENGTH_SHIFT_RPLIDAR;
	return len;
}

inline int StopRequestRPLIDAR(RPLIDAR* pRPLIDAR)
{
	unsigned char reqbuf[] = {START_FLAG1_RPLIDAR,STOP_REQUEST_RPLIDAR};

	// Send request.
	if (WriteAllRS232Port(&pRPLIDAR->RS232Port, reqbuf, sizeof(reqbuf)) != EXIT_SUCCESS)
	{
		printf("Error writing data to a RPLIDAR. \n");
		return EXIT_FAILURE;
	}

	// Host systems should wait for at least 1 millisecond (ms) before sending another request.
	mSleep(1);

	return EXIT_SUCCESS;
}

inline int ResetRequestRPLIDAR(RPLIDAR* pRPLIDAR)
{
	unsigned char reqbuf[] = {START_FLAG1_RPLIDAR,RESET_REQUEST_RPLIDAR};

	// Send request.
	if (WriteAllRS232Port(&pRPLIDAR->RS232Port, reqbuf, sizeof(reqbuf)) != EXIT_SUCCESS)
	{
		printf("Error writing data to a RPLIDAR. \n");
		return EXIT_FAILURE;
	}

	// Host systems should wait for at least 2 milliseconds (ms) before sending another request.
	mSleep(2);

	return EXIT_SUCCESS;
}

// Undocumented...
inline int GetStartupMessageRPLIDAR(RPLIDAR* pRPLIDAR)
{
	//unsigned char databuf[57]; // A2...
	//unsigned char databuf[MAX_NB_BYTES_RPLIDAR];
	//int nbReadBytes = 0;

	//// Undocumented response for A2...
	//memset(databuf, 0, sizeof(databuf));
	//if (ReadAllRS232Port(&pRPLIDAR->RS232Port, databuf, sizeof(databuf)) != EXIT_SUCCESS)
	//{ 
	//	printf("A RPLIDAR is not responding correctly. \n");
	//	return EXIT_FAILURE;	
	//}

	// Undocumented response...
	mSleep(100);

	//memset(databuf, 0, sizeof(databuf));
	//if (ReadRS232Port(&pRPLIDAR->RS232Port, databuf, sizeof(databuf), &nbReadBytes) != EXIT_SUCCESS)
	//{ 
	//	//printf("Warning : A RPLIDAR might not be responding correctly. \n");
	//	//return EXIT_FAILURE;	
	//}

	if (PurgeRS232Port(&pRPLIDAR->RS232Port) != EXIT_SUCCESS)
	{
		printf("Warning : A RPLIDAR might not be responding correctly. \n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

inline int GetHealthRequestRPLIDAR(RPLIDAR* pRPLIDAR, BOOL* pbProtectionStop)
{
	unsigned char reqbuf[] = {START_FLAG1_RPLIDAR,GET_HEALTH_REQUEST_RPLIDAR};
	unsigned char descbuf[7];
	unsigned char databuf[3];

	// Send request.
	if (WriteAllRS232Port(&pRPLIDAR->RS232Port, reqbuf, sizeof(reqbuf)) != EXIT_SUCCESS)
	{
		printf("Error writing data to a RPLIDAR. \n");
		return EXIT_FAILURE;
	}

	// Receive the response descriptor.
	memset(descbuf, 0, sizeof(descbuf));
	if (ReadAllRS232Port(&pRPLIDAR->RS232Port, descbuf, sizeof(descbuf)) != EXIT_SUCCESS)
	{ 
		printf("A RPLIDAR is not responding correctly. \n");
		return EXIT_FAILURE;	
	}

	// Quick check of the response descriptor.
	if ((descbuf[2] != 0x03)||(descbuf[6] != DEVHEALTH_RESPONSE_RPLIDAR))
	{ 
		printf("A RPLIDAR is not responding correctly. \n");
		return EXIT_FAILURE;	
	}

	// Receive the data response.
	memset(databuf, 0, sizeof(databuf));
	if (ReadAllRS232Port(&pRPLIDAR->RS232Port, databuf, sizeof(databuf)) != EXIT_SUCCESS)
	{ 
		printf("A RPLIDAR is not responding correctly. \n");
		return EXIT_FAILURE;	
	}

	// Analyze the data response.
	*pbProtectionStop = (databuf[0] == STATUS_ERROR_RPLIDAR)? TRUE: FALSE;

	return EXIT_SUCCESS;
}

// SerialNumber must be a null-terminated string of at least MAX_BUF_LEN_SERIAL_NUMBER_RPLIDAR bytes, including 0.
inline int GetInfoRequestRPLIDAR(RPLIDAR* pRPLIDAR, int* pModelID, int* pHardwareVersion, int* pFirmwareMajor, int* pFirmwareMinor, char* SerialNumber)
{
	unsigned char reqbuf[] = {START_FLAG1_RPLIDAR,GET_INFO_REQUEST_RPLIDAR};
	unsigned char descbuf[7];
	unsigned char databuf[20];
	int i = 0;

	// Send request.
	if (WriteAllRS232Port(&pRPLIDAR->RS232Port, reqbuf, sizeof(reqbuf)) != EXIT_SUCCESS)
	{
		printf("Error writing data to a RPLIDAR. \n");
		return EXIT_FAILURE;
	}

	// Receive the response descriptor.
	memset(descbuf, 0, sizeof(descbuf));
	if (ReadAllRS232Port(&pRPLIDAR->RS232Port, descbuf, sizeof(descbuf)) != EXIT_SUCCESS)
	{ 
		printf("A RPLIDAR is not responding correctly. \n");
		return EXIT_FAILURE;	
	}

	// Quick check of the response descriptor.
	if ((descbuf[2] != 0x14)||(descbuf[6] != DEVINFO_RESPONSE_RPLIDAR))
	{ 
		printf("A RPLIDAR is not responding correctly. \n");
		return EXIT_FAILURE;	
	}

	// Receive the data response.
	memset(databuf, 0, sizeof(databuf));
	if (ReadAllRS232Port(&pRPLIDAR->RS232Port, databuf, sizeof(databuf)) != EXIT_SUCCESS)
	{ 
		printf("A RPLIDAR is not responding correctly. \n");
		return EXIT_FAILURE;	
	}

	// Analyze the data response.
	*pModelID = databuf[0];
	*pFirmwareMinor = databuf[1];
	*pFirmwareMajor = databuf[2];
	*pHardwareVersion = databuf[3];

	// 128bit unique serial number, when converting to text in hex, the Least Significant Byte prints first.
	memset(SerialNumber, 0, MAX_BUF_LEN_SERIAL_NUMBER_RPLIDAR);
	for (i = 0; i < NB_BYTES_SERIAL_NUMBER_RPLIDAR; i++)
	{
		sprintf(SerialNumber+2*i, "%02X", (int)(unsigned char)databuf[i+4]);
	}

	return EXIT_SUCCESS;
}

// Tstandard, Texpress : Time in us used when RPLIDAR takes a single laser ranging in SCAN and EXPRESS_SCAN modes.
inline int GetSampleRateRequestRPLIDAR(RPLIDAR* pRPLIDAR, int* pTstandard, int* pTexpress)
{
	unsigned char reqbuf[] = {START_FLAG1_RPLIDAR,GET_SAMPLERATE_REQUEST_RPLIDAR};
	unsigned char descbuf[7];
	unsigned char databuf[4];

	// Send request.
	if (WriteAllRS232Port(&pRPLIDAR->RS232Port, reqbuf, sizeof(reqbuf)) != EXIT_SUCCESS)
	{
		printf("Error writing data to a RPLIDAR. \n");
		return EXIT_FAILURE;
	}

	// Receive the response descriptor.
	memset(descbuf, 0, sizeof(descbuf));
	if (ReadAllRS232Port(&pRPLIDAR->RS232Port, descbuf, sizeof(descbuf)) != EXIT_SUCCESS)
	{ 
		printf("A RPLIDAR is not responding correctly. \n");
		return EXIT_FAILURE;	
	}

	// Quick check of the response descriptor.
	if ((descbuf[2] != 0x04)||(descbuf[6] != SAMPLERATE_RESPONSE_RPLIDAR))
	{ 
		printf("A RPLIDAR is not responding correctly. \n");
		return EXIT_FAILURE;	
	}

	// Receive the data response.
	memset(databuf, 0, sizeof(databuf));
	if (ReadAllRS232Port(&pRPLIDAR->RS232Port, databuf, sizeof(databuf)) != EXIT_SUCCESS)
	{ 
		printf("A RPLIDAR is not responding correctly. \n");
		return EXIT_FAILURE;	
	}

	// Analyze the data response.
	*pTstandard = (unsigned short)((databuf[1]<<8)|databuf[0]);
	*pTexpress = (unsigned short)((databuf[3]<<8)|databuf[2]);

	return EXIT_SUCCESS;
}

// Undocumented...
inline int CheckMotorControlSupportRequestRPLIDAR(RPLIDAR* pRPLIDAR)
{
	unsigned char reqbuf[] = {START_FLAG1_RPLIDAR,GET_ACC_BOARD_FLAG_REQUEST_RPLIDAR};
	//unsigned char databuf[MAX_NB_BYTES_RPLIDAR];
	//int nbReadBytes = 0;

	// Send request.
	if (WriteAllRS232Port(&pRPLIDAR->RS232Port, reqbuf, sizeof(reqbuf)) != EXIT_SUCCESS)
	{
		printf("Error writing data to a RPLIDAR. \n");
		return EXIT_FAILURE;
	}

	mSleep(100);

	//memset(databuf, 0, sizeof(databuf));
	//if (ReadRS232Port(&pRPLIDAR->RS232Port, databuf, sizeof(databuf), &nbReadBytes) != EXIT_SUCCESS)
	//{ 
	//	//printf("Warning : A RPLIDAR might not be responding correctly. \n");
	//	//return EXIT_FAILURE;	
	//}

	if (PurgeRS232Port(&pRPLIDAR->RS232Port) != EXIT_SUCCESS)
	{
		printf("Warning : A RPLIDAR might not be responding correctly. \n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

// Undocumented...
inline int SetMotorPWMRequestRPLIDAR(RPLIDAR* pRPLIDAR, int pwm)
{
	unsigned char reqbuf[] = {START_FLAG1_RPLIDAR,SET_MOTOR_PWM_REQUEST_RPLIDAR,0x02,0,0,0};

	reqbuf[3] = (unsigned char)pwm;
	reqbuf[4] = (unsigned char)(pwm>>8);
	reqbuf[5] = ComputeChecksumRPLIDAR(reqbuf, sizeof(reqbuf));

	// Send request.
	if (WriteAllRS232Port(&pRPLIDAR->RS232Port, reqbuf, sizeof(reqbuf)) != EXIT_SUCCESS)
	{
		printf("Error writing data to a RPLIDAR. \n");
		return EXIT_FAILURE;
	}

	mSleep(500);

	return EXIT_SUCCESS;
}

inline int StartScanRequestRPLIDAR(RPLIDAR* pRPLIDAR)
{
	unsigned char reqbuf[] = {START_FLAG1_RPLIDAR,SCAN_REQUEST_RPLIDAR};
	unsigned char descbuf[7];

	// Send request.
	if (WriteAllRS232Port(&pRPLIDAR->RS232Port, reqbuf, sizeof(reqbuf)) != EXIT_SUCCESS)
	{
		printf("Error writing data to a RPLIDAR. \n");
		return EXIT_FAILURE;
	}

	// Receive the response descriptor.
	memset(descbuf, 0, sizeof(descbuf));
	if (ReadAllRS232Port(&pRPLIDAR->RS232Port, descbuf, sizeof(descbuf)) != EXIT_SUCCESS)
	{ 
		printf("A RPLIDAR is not responding correctly. \n");
		return EXIT_FAILURE;	
	}

	// Quick check of the response descriptor.
	if ((descbuf[2] != 0x05)||(descbuf[5] != 0x40)||(descbuf[6] != MEASUREMENT_RESPONSE_RPLIDAR))
	{ 
		printf("A RPLIDAR is not responding correctly. \n");
		return EXIT_FAILURE;	
	}

	return EXIT_SUCCESS;
}

inline int StartForceScanRequestRPLIDAR(RPLIDAR* pRPLIDAR)
{
	unsigned char reqbuf[] = {START_FLAG1_RPLIDAR,FORCE_SCAN_REQUEST_RPLIDAR};
	unsigned char descbuf[7];

	// Send request.
	if (WriteAllRS232Port(&pRPLIDAR->RS232Port, reqbuf, sizeof(reqbuf)) != EXIT_SUCCESS)
	{
		printf("Error writing data to a RPLIDAR. \n");
		return EXIT_FAILURE;
	}

	// Receive the response descriptor.
	memset(descbuf, 0, sizeof(descbuf));
	if (ReadAllRS232Port(&pRPLIDAR->RS232Port, descbuf, sizeof(descbuf)) != EXIT_SUCCESS)
	{ 
		printf("A RPLIDAR is not responding correctly. \n");
		return EXIT_FAILURE;	
	}

	// Quick check of the response descriptor.
	if ((descbuf[2] != 0x05)||(descbuf[5] != 0x40)||(descbuf[6] != MEASUREMENT_RESPONSE_RPLIDAR))
	{ 
		printf("A RPLIDAR is not responding correctly. \n");
		return EXIT_FAILURE;	
	}

	return EXIT_SUCCESS;
}

inline int GetScanDataResponseRPLIDAR(RPLIDAR* pRPLIDAR, double* pDistance, double* pAngle, BOOL* pbNewScan, int* pQuality)
{
	unsigned char databuf[5];
	unsigned short angle_q6 = 0;
	unsigned short distance_q2 = 0;

	// Receive the data response.
	memset(databuf, 0, sizeof(databuf));
	if (ReadAllRS232Port(&pRPLIDAR->RS232Port, databuf, sizeof(databuf)) != EXIT_SUCCESS)
	{ 
		printf("A RPLIDAR is not responding correctly. \n");
		return EXIT_FAILURE;	
	}

	// Analyze the data response.
	*pbNewScan = (START_BIT_MASK_SCAN_DATA_RESPONSE_RPLIDAR & databuf[0]);
	if (*pbNewScan == ((INVERTED_START_BIT_MASK_SCAN_DATA_RESPONSE_RPLIDAR & databuf[0])>>1))
	{ 
		printf("A RPLIDAR is not responding correctly: Bad inversed start bit. \n");
		return EXIT_FAILURE;	
	}
	if ((CHECK_BIT_MASK_SCAN_DATA_RESPONSE_RPLIDAR & databuf[1]) != 1)
	{ 
		printf("A RPLIDAR is not responding correctly : Bad check bit. \n");
		return EXIT_FAILURE;	
	}
	*pQuality = (unsigned char)(databuf[0]>>2);
	angle_q6 = (databuf[2]<<7)|(databuf[1]>>1);
	distance_q2 = (databuf[4]<<8)|databuf[3];

	// Convert in rad.
	*pAngle = fmod_2PI_deg2rad(-angle_q6/64.0);

	// Convert in m.
	*pDistance = distance_q2/4000.0;

	return EXIT_SUCCESS;
}

inline int StartExpressScanRequestRPLIDAR(RPLIDAR* pRPLIDAR)
{
	unsigned char reqbuf[] = {START_FLAG1_RPLIDAR,EXPRESS_SCAN_REQUEST_RPLIDAR,0x05,0,0,0,0,0,0x22};
	unsigned char descbuf[7];
	unsigned char sync = 0;
	unsigned char ChkSum = 0;

	// Send request.
	if (WriteAllRS232Port(&pRPLIDAR->RS232Port, reqbuf, sizeof(reqbuf)) != EXIT_SUCCESS)
	{
		printf("Error writing data to a RPLIDAR. \n");
		return EXIT_FAILURE;
	}

	// Receive the response descriptor.
	memset(descbuf, 0, sizeof(descbuf));
	if (ReadAllRS232Port(&pRPLIDAR->RS232Port, descbuf, sizeof(descbuf)) != EXIT_SUCCESS)
	{ 
		printf("A RPLIDAR is not responding correctly. \n");
		return EXIT_FAILURE;	
	}

	// Quick check of the response descriptor.
	if ((descbuf[2] != 0x54)||(descbuf[5] != 0x40)||(descbuf[6] != MEASUREMENT_CAPSULED_RESPONSE_RPLIDAR))
	{ 
		printf("A RPLIDAR is not responding correctly. \n");
		return EXIT_FAILURE;	
	}

	// Receive the first data response (2 data responses needed for angles computation...).
	memset(pRPLIDAR->esdata_prev, 0, sizeof(pRPLIDAR->esdata_prev));
	if (ReadAllRS232Port(&pRPLIDAR->RS232Port, pRPLIDAR->esdata_prev, sizeof(pRPLIDAR->esdata_prev)) != EXIT_SUCCESS)
	{ 
		printf("A RPLIDAR is not responding correctly. \n");
		return EXIT_FAILURE;	
	}

	// Analyze the first data response.
	sync = (pRPLIDAR->esdata_prev[0] & 0xF0)|(pRPLIDAR->esdata_prev[1]>>4);
	if (sync != START_FLAG1_RPLIDAR)
	{ 
		printf("A RPLIDAR is not responding correctly : Bad sync1 or sync2. \n");
		return EXIT_FAILURE;	
	}

	ChkSum = (pRPLIDAR->esdata_prev[1]<<4)|(pRPLIDAR->esdata_prev[0] & 0x0F);
	// Force ComputeChecksumRPLIDAR() to compute until the last byte...
	if (ChkSum != ComputeChecksumRPLIDAR(pRPLIDAR->esdata_prev+2, sizeof(pRPLIDAR->esdata_prev)-1))
	{ 
		printf("A RPLIDAR is not responding correctly : Bad ChkSum. \n");
		return EXIT_FAILURE;	
	}

	return EXIT_SUCCESS;
}

// NB_MEASUREMENTS_EXPRESS_SCAN_DATA_RESPONSE_RPLIDAR distances, NB_MEASUREMENTS_EXPRESS_SCAN_DATA_RESPONSE_RPLIDAR angles...
inline int GetExpressScanDataResponseRPLIDAR(RPLIDAR* pRPLIDAR, double* pDistances, double* pAngles, BOOL* pbNewScan)
{
	unsigned char databuf[NB_BYTES_EXPRESS_SCAN_DATA_RESPONSE_RPLIDAR];
	unsigned char sync = 0;
	unsigned char ChkSum = 0;
	unsigned short start_angle_q6 = 0;
	unsigned short start_angle_q6_prev = 0;
	unsigned short distance1 = 0;
	unsigned short distance2 = 0;
	char dtheta1_q3 = 0;
	char dtheta2_q3 = 0;
	double dtheta1 = 0;
	double dtheta2 = 0;
	double theta1 = 0;
	double theta2 = 0;
	int j = 0;

	// Receive the data response.
	memset(databuf, 0, sizeof(databuf));
	if (ReadAllRS232Port(&pRPLIDAR->RS232Port, databuf, sizeof(databuf)) != EXIT_SUCCESS)
	{ 
		printf("A RPLIDAR is not responding correctly. \n");
		return EXIT_FAILURE;	
	}

	// Analyze the data response.
	sync = (databuf[0] & 0xF0)|(databuf[1]>>4);
	if (sync != START_FLAG1_RPLIDAR)
	{ 
		printf("A RPLIDAR is not responding correctly : Bad sync1 or sync2. \n");
		return EXIT_FAILURE;	
	}

	ChkSum = (databuf[1]<<4)|(databuf[0] & 0x0F);
	// Force ComputeChecksumRPLIDAR() to compute until the last byte...
	if (ChkSum != ComputeChecksumRPLIDAR(databuf+2, sizeof(databuf)-1))
	{ 
		printf("A RPLIDAR is not responding correctly : Bad ChkSum. \n");
		return EXIT_FAILURE;	
	}

	start_angle_q6 = ((databuf[3] & 0x7F)<<8) | databuf[2];

	// Analyze the previous data response.
	start_angle_q6_prev = ((pRPLIDAR->esdata_prev[3] & 0x7F)<<8) | pRPLIDAR->esdata_prev[2];
	*pbNewScan = pRPLIDAR->esdata_prev[3]>>7;

	//memset(pAngles, 0, NB_MEASUREMENTS_EXPRESS_SCAN_DATA_RESPONSE_RPLIDAR);
	//memset(pDistances, 0, NB_MEASUREMENTS_EXPRESS_SCAN_DATA_RESPONSE_RPLIDAR);
	for (j = 0; j < NB_CABINS_EXPRESS_SCAN_DATA_RESPONSE_RPLIDAR; j++)
	{
		distance1 = ((pRPLIDAR->esdata_prev[4+5*j+1]<<8)|pRPLIDAR->esdata_prev[4+5*j+0])>>2;
		distance2 = ((pRPLIDAR->esdata_prev[4+5*j+3]<<8)|pRPLIDAR->esdata_prev[5*j+2])>>2;
		dtheta1_q3 = ((pRPLIDAR->esdata_prev[4+5*j+0] & 0x03)<<4)|(pRPLIDAR->esdata_prev[4+5*j+4] & 0x0F);
		dtheta2_q3 = ((pRPLIDAR->esdata_prev[4+5*j+2] & 0x03)<<4)|(pRPLIDAR->esdata_prev[4+5*j+4]>>4);
		// Handle the sign bit...
		dtheta1 = ((dtheta1_q3 & 0x20)>>5)? (-~(dtheta1_q3 & 0x1F))/8.0: (dtheta1_q3 & 0x1F)/8.0;
		dtheta2 = ((dtheta2_q3 & 0x20)>>5)? (-~(dtheta2_q3 & 0x1F))/8.0: (dtheta2_q3 & 0x1F)/8.0;

		// k indexes in the formula of the documentation start at 1...
		theta1 = start_angle_q6_prev/64.0+AngleDiffRPLIDAR(start_angle_q6_prev/64.0, start_angle_q6/64.0)*(2*j+1)/(NB_MEASUREMENTS_EXPRESS_SCAN_DATA_RESPONSE_RPLIDAR)-dtheta1;
		theta2 = start_angle_q6_prev/64.0+AngleDiffRPLIDAR(start_angle_q6_prev/64.0, start_angle_q6/64.0)*(2*j+2)/(NB_MEASUREMENTS_EXPRESS_SCAN_DATA_RESPONSE_RPLIDAR)-dtheta2;

		// Convert in rad.
		pAngles[2*j+0] = fmod_2PI_deg2rad(-theta1);
		pAngles[2*j+1] = fmod_2PI_deg2rad(-theta2);

		// Convert in m.
		pDistances[2*j+0] = distance1/1000.0;
		pDistances[2*j+1] = distance2/1000.0;
	}

	memcpy(pRPLIDAR->esdata_prev, databuf, NB_BYTES_EXPRESS_SCAN_DATA_RESPONSE_RPLIDAR);

	return EXIT_SUCCESS;
}

// RPLIDAR must be initialized to 0 before (e.g. RPLIDAR rplidar; memset(&rplidar, 0, sizeof(RPLIDAR));)!
inline int ConnectRPLIDAR(RPLIDAR* pRPLIDAR, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];
	BOOL bProtectionStop = FALSE;

	memset(pRPLIDAR->szCfgFilePath, 0, sizeof(pRPLIDAR->szCfgFilePath));
	sprintf(pRPLIDAR->szCfgFilePath, "%.255s", szCfgFilePath);

	// If szCfgFilePath starts with "hardcoded://", parameters are assumed to be already set in the structure, 
	// otherwise it should be loaded from a configuration file.
	if (strncmp(szCfgFilePath, "hardcoded://", strlen("hardcoded://")) != 0)
	{
		memset(line, 0, sizeof(line));

		// Default values.
		memset(pRPLIDAR->szDevPath, 0, sizeof(pRPLIDAR->szDevPath));
		sprintf(pRPLIDAR->szDevPath, "COM1");
		pRPLIDAR->BaudRate = 115200;
		pRPLIDAR->timeout = 1000;
		pRPLIDAR->bSaveRawData = 1;
		pRPLIDAR->ScanMode = SCAN_MODE_RPLIDAR;
		pRPLIDAR->motordelay = 500;
		pRPLIDAR->maxhist = 1024;
		pRPLIDAR->alpha_max_err = 0.01;
		pRPLIDAR->d_max_err = 0.1;

		// Load data from a file.
		file = fopen(szCfgFilePath, "r");
		if (file != NULL)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pRPLIDAR->szDevPath) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRPLIDAR->BaudRate) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRPLIDAR->timeout) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRPLIDAR->bSaveRawData) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRPLIDAR->ScanMode) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRPLIDAR->motordelay) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRPLIDAR->maxhist) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRPLIDAR->alpha_max_err) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRPLIDAR->d_max_err) != 1) printf("Invalid configuration file.\n");
			if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
		}
		else
		{
			printf("Configuration file not found.\n");
		}
	}

	if (pRPLIDAR->ScanMode < 0)
	{
		printf("Invalid parameter : ScanMode.\n");
		pRPLIDAR->ScanMode = SCAN_MODE_RPLIDAR;
	}
	if (pRPLIDAR->motordelay < 0)
	{
		printf("Invalid parameter : motordelay.\n");
		pRPLIDAR->motordelay = 500;
	}
	if ((pRPLIDAR->maxhist < 0)||(pRPLIDAR->maxhist > MAX_NB_MEASUREMENTS_PER_SCAN_RPLIDAR))
	{
		printf("Invalid parameter : maxhist.\n");
		pRPLIDAR->maxhist = 1024;
	}

	// Used to save raw data, should be handled specifically...
	//pRPLIDAR->pfSaveFile = NULL;

	if (OpenRS232Port(&pRPLIDAR->RS232Port, pRPLIDAR->szDevPath) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a RPLIDAR.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsRS232Port(&pRPLIDAR->RS232Port, pRPLIDAR->BaudRate, NOPARITY, FALSE, 8, 
		ONESTOPBIT, (UINT)pRPLIDAR->timeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a RPLIDAR.\n");
		CloseRS232Port(&pRPLIDAR->RS232Port);
		return EXIT_FAILURE;
	}

	GetStartupMessageRPLIDAR(pRPLIDAR);

	memset(pRPLIDAR->SerialNumber, 0, sizeof(pRPLIDAR->SerialNumber));
	if (GetInfoRequestRPLIDAR(pRPLIDAR, &pRPLIDAR->model, &pRPLIDAR->hardware, &pRPLIDAR->firmware_major, &pRPLIDAR->firmware_minor, pRPLIDAR->SerialNumber) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a RPLIDAR : GET_INFO failure.\n");
		CloseRS232Port(&pRPLIDAR->RS232Port);
		return EXIT_FAILURE;
	}

	if (GetHealthRequestRPLIDAR(pRPLIDAR, &bProtectionStop) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a RPLIDAR : GET_HEALTH failure.\n");
		CloseRS232Port(&pRPLIDAR->RS232Port);
		return EXIT_FAILURE;
	}

	if (bProtectionStop)
	{
		if (ResetRequestRPLIDAR(pRPLIDAR) != EXIT_SUCCESS)
		{
			printf("Unable to connect to a RPLIDAR : RESET failure.\n");
			CloseRS232Port(&pRPLIDAR->RS232Port);
			return EXIT_FAILURE;
		}
		GetStartupMessageRPLIDAR(pRPLIDAR);
	}

	// Incompatible with old RPLIDAR...
	//if (GetSampleRateRequestRPLIDAR(pRPLIDAR, &pRPLIDAR->Tstandard, &pRPLIDAR->Texpress) != EXIT_SUCCESS)
	//{
	//	printf("Unable to connect to a RPLIDAR : GET_SAMPLERATE failure.\n");
	//	CloseRS232Port(&pRPLIDAR->RS232Port);
	//	return EXIT_FAILURE;
	//}

	if (SetMotorPWMRequestRPLIDAR(pRPLIDAR, DEFAULT_MOTOR_PWM_RPLIDAR) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a RPLIDAR : SET_MOTOR_PWM failure.\n");
		CloseRS232Port(&pRPLIDAR->RS232Port);
		return EXIT_FAILURE;
	}

	// Stop any currently running scan.
	if (StopRequestRPLIDAR(pRPLIDAR) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a RPLIDAR : STOP failure.\n");
		SetMotorPWMRequestRPLIDAR(pRPLIDAR, 0);
		CloseRS232Port(&pRPLIDAR->RS232Port);
		return EXIT_FAILURE;
	}

	memset(pRPLIDAR->esdata_prev, 0, sizeof(pRPLIDAR->esdata_prev));
	switch (pRPLIDAR->ScanMode)
	{
	case SCAN_MODE_RPLIDAR:
		if (StartScanRequestRPLIDAR(pRPLIDAR) != EXIT_SUCCESS)
		{
			printf("Unable to connect to a RPLIDAR : SCAN failure.\n");
			SetMotorPWMRequestRPLIDAR(pRPLIDAR, 0);
			CloseRS232Port(&pRPLIDAR->RS232Port);
			return EXIT_FAILURE;
		}
		break;
	case EXPRESS_SCAN_MODE_RPLIDAR:
		if (StartExpressScanRequestRPLIDAR(pRPLIDAR) != EXIT_SUCCESS)
		{
			printf("Unable to connect to a RPLIDAR : EXPRESS_SCAN failure.\n");
			SetMotorPWMRequestRPLIDAR(pRPLIDAR, 0);
			CloseRS232Port(&pRPLIDAR->RS232Port);
			return EXIT_FAILURE;
		}
		break;
	case FORCE_SCAN_MODE_RPLIDAR:
		if (StartForceScanRequestRPLIDAR(pRPLIDAR) != EXIT_SUCCESS)
		{
			printf("Unable to connect to a RPLIDAR : FORCE_SCAN failure.\n");
			SetMotorPWMRequestRPLIDAR(pRPLIDAR, 0);
			CloseRS232Port(&pRPLIDAR->RS232Port);
			return EXIT_FAILURE;
		}
		break;
	default:
		if (StartScanRequestRPLIDAR(pRPLIDAR) != EXIT_SUCCESS)
		{
			printf("Unable to connect to a RPLIDAR : SCAN failure.\n");
			SetMotorPWMRequestRPLIDAR(pRPLIDAR, 0);
			CloseRS232Port(&pRPLIDAR->RS232Port);
			return EXIT_FAILURE;
		}
		break;
	}

	// Wait for the motor rotation to become stable.
	mSleep(pRPLIDAR->motordelay);

	printf("RPLIDAR connected.\n");

	return EXIT_SUCCESS;
}

inline int DisconnectRPLIDAR(RPLIDAR* pRPLIDAR)
{		
	if (StopRequestRPLIDAR(pRPLIDAR) != EXIT_SUCCESS)
	{
		printf("RPLIDAR disconnection failed.\n");
		SetMotorPWMRequestRPLIDAR(pRPLIDAR, 0);
		CloseRS232Port(&pRPLIDAR->RS232Port);
		return EXIT_FAILURE;
	}

	if (SetMotorPWMRequestRPLIDAR(pRPLIDAR, 0) != EXIT_SUCCESS)
	{
		printf("RPLIDAR disconnection failed.\n");
		CloseRS232Port(&pRPLIDAR->RS232Port);
		return EXIT_FAILURE;
	}

	if (CloseRS232Port(&pRPLIDAR->RS232Port) != EXIT_SUCCESS)
	{
		printf("RPLIDAR disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("RPLIDAR disconnected.\n");

	return EXIT_SUCCESS;
}

#ifndef DISABLE_RPLIDARTHREAD
THREAD_PROC_RETURN_VALUE RPLIDARThread(void* pParam);
#endif // !DISABLE_RPLIDARTHREAD

#endif // !RPLIDAR_H
