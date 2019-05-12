// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef MS580314BA_H
#define MS580314BA_H

#include "OSMisc.h"
#include "RS232Port.h"

#ifndef DISABLE_MS580314BATHREAD
#include "OSThread.h"
#endif // !DISABLE_MS580314BATHREAD

#define TIMEOUT_MESSAGE_MS580314BA 4.0 // In s.
// Should be at least 2 * number of bytes to be sure to contain entirely the biggest desired message (or group of messages) + 1.
#define MAX_NB_BYTES_MS580314BA 256

struct MS580314BA
{
	RS232PORT RS232Port;
	double Ca[10];
	//double PressureIni;
	FILE* pfSaveFile; // Used to save raw data, should be handled specifically...
	double LastPressure;
	char szCfgFilePath[256];
	// Parameters.
	char szDevPath[256];
	int BaudRate;
	int timeout;
	int threadperiod;
	BOOL bSaveRawData;
	double PressureRef;
	double WaterDensity;
};
typedef struct MS580314BA MS580314BA;

/*
Initialize a MS580314BA.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int InitMS580314BA(MS580314BA* pMS580314BA)
{
	uint8 writebuf[5];
	
	// Sequence for I2c and serial communication
	writebuf[0] = (uint8)0x5A; // start configuration
	writebuf[1] = (uint8)0x02; // iss mode
	writebuf[2] = (uint8)0x20; // I2C_S_100KHZ + Serial 0x61
	writebuf[3] = (uint8)0x00; // (115200)
	writebuf[4] = (uint8)0x19; // Baudrate

	if (WriteAllRS232Port(&pMS580314BA->RS232Port, writebuf, 5) != EXIT_SUCCESS)
	{ 
		printf("Error writing data to a MS580314BA. \n");
		return EXIT_FAILURE;
	}
	
	mSleep(10);

	return EXIT_SUCCESS;
}

inline int CalibrateMS580314BA(MS580314BA* pMS580314BA)
{
	uint8 writebuf[4];
	uint8 readbuf[50];
	int nbReadBytes = 0;

	// Missing error checking...

	memset(pMS580314BA->Ca, 0, sizeof(pMS580314BA->Ca));

	writebuf[0] = (uint8)0x54; // start configuration
	writebuf[1] = (uint8)0xEE; // I2C address
	writebuf[2] = (uint8)0x01; // Number of bytes
	writebuf[3] = (uint8)0x1E; // Data 

	if (WriteAllRS232Port(&pMS580314BA->RS232Port, writebuf, 4) != EXIT_SUCCESS)
	{
		printf("Error writing data to a MS580314BA. \n");
		return EXIT_FAILURE;
	}

	mSleep(10);

	for (int i = 0; i<6; i++)
	{
		writebuf[0] = (uint8)0x54;
		writebuf[1] = (uint8)0xEE;
		writebuf[2] = (uint8)0x01;
		writebuf[3] = (uint8)(0xA2 + (i * 2));

		if (WriteAllRS232Port(&pMS580314BA->RS232Port, writebuf, 4) != EXIT_SUCCESS)
		{
			printf("Error writing data to a MS580314BA. \n");
			return EXIT_FAILURE;
		}

		mSleep(10);

		writebuf[0] = (uint8)0x54;
		writebuf[1] = (uint8)0xEF;
		writebuf[2] = (uint8)0x02;

		if (WriteAllRS232Port(&pMS580314BA->RS232Port, writebuf, 3) != EXIT_SUCCESS)
		{
			printf("Error writing data to a MS580314BA. \n");
			return EXIT_FAILURE;
		}

		mSleep(10);

		if (ReadRS232Port(&pMS580314BA->RS232Port, readbuf, 50, &nbReadBytes) != EXIT_SUCCESS)
		{
			printf("Error reading data from a MS580314BA. \n");
			return EXIT_FAILURE;
		}

		//printf("%d  %d  %d  %d  %d  %d  %d  %d \n",buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7]);

		long V = 0;

		V = (readbuf[1] << 8) | readbuf[2];
		if (i == 0) { V = (readbuf[2] << 8) | readbuf[3]; }
		//printf(" %d \n",V);
		pMS580314BA->Ca[i+1] = V;
	}

	return EXIT_SUCCESS;
}

/*
Write data to a MS580314BA.

uint8 data : (IN) Data to write.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int WriteDataMS580314BA(MS580314BA* pMS580314BA, uint8 data)
{
	uint8 writebuf[4];

	writebuf[0] = (uint8)0x54; // Read or Write multiple bytes for devices without internal address or where address does not require resetting.
	writebuf[1] = (uint8)0xEE; // I2C address
	writebuf[2] = (uint8)0x01; // Number of bytes
	writebuf[3] = (uint8)data;

	if (WriteAllRS232Port(&pMS580314BA->RS232Port, writebuf, 4) != EXIT_SUCCESS)
	{ 
		printf("Error writing data to a MS580314BA. \n");
		return EXIT_FAILURE;
	}

	mSleep(10);

	return EXIT_SUCCESS;
}

/*
Read data from of a MS580314BA.

int ind : (IN) ?.
unsigned long* pValue : (INOUT) Valid pointer receiving the value.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int ReadDataMS580314BA(MS580314BA* pMS580314BA, int ind, unsigned long* pValue)
{
	uint8 writebuf[3];
	uint8 readbuf[50];
	int nbReadBytes = 0;
	
	writebuf[0] = (uint8)0x54; // Read or Write multiple bytes for devices without internal address or where address does not require resetting.
	writebuf[1] = (uint8)0xEF; // I2C address
	writebuf[2] = (uint8)0x03; // Number of bytes

	if (WriteAllRS232Port(&pMS580314BA->RS232Port, writebuf, 3) != EXIT_SUCCESS)
	{ 
		printf("Error writing data to a MS580314BA. \n");
		return EXIT_FAILURE;
	}

	mSleep(10);

	if (ReadRS232Port(&pMS580314BA->RS232Port, readbuf, 50, &nbReadBytes) != EXIT_SUCCESS)
	{
		printf("Error reading data from a MS580314BA. \n");
		return EXIT_FAILURE;
	}

	//printf("%d  %d  %d  %d  %d  %d  %d  %d \n",buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7]);
	if (ind == 0) { *pValue = readbuf[3] * (unsigned long)65536 + readbuf[4] * (unsigned long)256 + readbuf[5]; }
	if (ind == 1) { *pValue = readbuf[2] * (unsigned long)65536 + readbuf[3] * (unsigned long)256 + readbuf[4]; }
	//printf(" %d  \n",ret);

	return EXIT_SUCCESS;
}

inline int GetPressureMS580314BA(MS580314BA* pMS580314BA, double* pPressure)
{
	unsigned long D1 = 0, D2 = 0;
	//double tim = 0, ini = 0, fin = 0;
	//double OFF = 0, dt = 0, TEM = 0, P = 0, SENS = 0, T2 = 0, OFF2 = 0, SENS2 = 0, z = 0, zini = 0, TEMPE = 0, Pres = 0, zant = 0;
	double OFF = 0, dt = 0, TEM = 0, P = 0, SENS = 0, T2 = 0, OFF2 = 0, SENS2 = 0, TEMPE = 0, Pres = 0;

	//double depth = Pressure2Height(*pPressure, pMS580314BA->PressureRef, pMS580314BA->WaterDensity); // ...

	// Missing error checking...

	WriteDataMS580314BA(pMS580314BA, 0x42); // Convert D1 (OSR=512)
	WriteDataMS580314BA(pMS580314BA, 0x00);
	ReadDataMS580314BA(pMS580314BA, 1, &D1);

	WriteDataMS580314BA(pMS580314BA, 0x52); // Convert D2 (OSR=512)
	WriteDataMS580314BA(pMS580314BA, 0x00);
	ReadDataMS580314BA(pMS580314BA, 1, &D2);

	dt = D2-pMS580314BA->Ca[5]*256;
	TEM = 2000+dt*pMS580314BA->Ca[6]/8388608;

	OFF = pMS580314BA->Ca[2]*65536;
	OFF = OFF+(pMS580314BA->Ca[4]*dt)/128;

	SENS = pMS580314BA->Ca[1]*32768;
	SENS = SENS+(pMS580314BA->Ca[3]*dt)/256;

	if (TEM <= 2000)
	{
		T2 = 3*(dt*dt);
		T2 = T2/8589934592.0;
		OFF2 = 3*(TEM-2000)*(TEM-2000)/2;
		SENS2 = 5*(TEM-2000)*(TEM-2000)/8;
	}
	else
	{
		T2 = 7*(dt*dt);
		T2 = T2/137438953472.0;
		OFF2 = 1*(TEM-2000)*(TEM-2000)/16;
		SENS2 = 0;
	}

	////Sleep(100);

	if (TEM < -1500)
	{
		OFF2 = OFF2+7*((TEM+1500)*(TEM+1500));
		SENS2 = SENS2+4*((TEM+1500)*(TEM+1500));
	}

	TEM = TEM-T2;
	OFF = OFF-OFF2;
	SENS = SENS-SENS2;

	P = (D1*SENS/2097152-OFF)/32768;

	Pres = P/10.0;
	TEMPE = TEM/100.0;

	//if (tim<1) { zini = (Pres*100)/9810; }
	//if (pMS580314BA->PressureIni == 0) { pMS580314BA->PressureIni = Pres; }

	if ((D1 == 0)||(D2 == 0))
	{
		//depth = depth;
		*pPressure = *pPressure; // ?
	}
	else
	{
		//depth = (Pres*100)/9810-(pMS580314BA->PressureRef*100)/9810;
		//*pPressure = (Pres-pMS580314BA->PressureIni)*(100.0/9810.0)*pMS580314BA->WaterDensity*STANDARD_GRAVITY/1e5;
		*pPressure = Pres/1000.0;
	}

	// ...

	//printf("*pPressure=%f,\n",*pPressure);

	//*pPressure = pMS580314BA->PressureRef-depth*(pMS580314BA->WaterDensity*STANDARD_GRAVITY)/1e5;

	pMS580314BA->LastPressure = *pPressure;

	return EXIT_SUCCESS;
}

// MS580314BA must be initialized to 0 before (e.g. MS580314BA ms580314ba; memset(&ms580314ba, 0, sizeof(MS580314BA));)!
inline int ConnectMS580314BA(MS580314BA* pMS580314BA, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];

	memset(pMS580314BA->szCfgFilePath, 0, sizeof(pMS580314BA->szCfgFilePath));
	sprintf(pMS580314BA->szCfgFilePath, "%.255s", szCfgFilePath);

	// If szCfgFilePath starts with "hardcoded://", parameters are assumed to be already set in the structure, 
	// otherwise it should be loaded from a configuration file.
	if (strncmp(szCfgFilePath, "hardcoded://", strlen("hardcoded://")) != 0)
	{
		memset(line, 0, sizeof(line));

		// Default values.
		memset(pMS580314BA->szDevPath, 0, sizeof(pMS580314BA->szDevPath));
		sprintf(pMS580314BA->szDevPath, "COM1");
		pMS580314BA->BaudRate = 9600;
		pMS580314BA->timeout = 1000;
		pMS580314BA->threadperiod = 100;
		pMS580314BA->bSaveRawData = 1;
		pMS580314BA->PressureRef = 1;
		pMS580314BA->WaterDensity = 1000;

		// Load data from a file.
		file = fopen(szCfgFilePath, "r");
		if (file != NULL)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pMS580314BA->szDevPath) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMS580314BA->BaudRate) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMS580314BA->timeout) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMS580314BA->threadperiod) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMS580314BA->bSaveRawData) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pMS580314BA->PressureRef) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pMS580314BA->WaterDensity) != 1) printf("Invalid configuration file.\n");
			if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
		}
		else
		{
			printf("Configuration file not found.\n");
		}
	}

	if (pMS580314BA->threadperiod < 0)
	{
		printf("Invalid parameter : threadperiod.\n");
		pMS580314BA->threadperiod = 100;
	}

	// Used to save raw data, should be handled specifically...
	//pMS580314BA->pfSaveFile = NULL;

	pMS580314BA->LastPressure = 0;

	if (OpenRS232Port(&pMS580314BA->RS232Port, pMS580314BA->szDevPath) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a MS580314BA.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsRS232Port(&pMS580314BA->RS232Port, pMS580314BA->BaudRate, NOPARITY, FALSE, 8, 
		ONESTOPBIT, (UINT)pMS580314BA->timeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a MS580314BA.\n");
		CloseRS232Port(&pMS580314BA->RS232Port);
		return EXIT_FAILURE;
	}

	if (CalibrateMS580314BA(pMS580314BA) != EXIT_SUCCESS)
	{
		printf("Unable to connect to MS580314BA : Calibration failure.\n");
		CloseRS232Port(&pMS580314BA->RS232Port);
		return EXIT_FAILURE;
	}

	if (InitMS580314BA(pMS580314BA) != EXIT_SUCCESS)
	{
		printf("Unable to connect to MS580314BA : Initialization failure.\n");
		CloseRS232Port(&pMS580314BA->RS232Port);
		return EXIT_FAILURE;
	}

	printf("MS580314BA connected.\n");

	return EXIT_SUCCESS;
}

inline int DisconnectMS580314BA(MS580314BA* pMS580314BA)
{
	if (CloseRS232Port(&pMS580314BA->RS232Port) != EXIT_SUCCESS)
	{
		printf("MS580314BA disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("MS580314BA disconnected.\n");

	return EXIT_SUCCESS;
}

#ifndef DISABLE_MS580314BATHREAD
THREAD_PROC_RETURN_VALUE MS580314BAThread(void* pParam);
#endif // !DISABLE_MS580314BATHREAD

#endif // !MS580314BA_H
