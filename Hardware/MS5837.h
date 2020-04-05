// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef MS5837_H
#define MS5837_H

#include "OSMisc.h"
#include "I2CBus.h"

#ifndef DISABLE_MS5837THREAD
#include "OSThread.h"
#endif // !DISABLE_MS5837THREAD

#define TIMEOUT_MESSAGE_MS5837 4.0 // In s.
// Should be at least 2 * number of bytes to be sure to contain entirely the biggest desired message (or group of messages) + 1.
#define MAX_NB_BYTES_MS5837 256

#define DEFAULT_ADDR_MS5837 0x76 // 0b1110110

// From https://github.com/TEConnectivity/MS5837_Generic_C_Driver

enum ms5837_resolution_osr {
	ms5837_resolution_osr_256 = 0,
	ms5837_resolution_osr_512,
	ms5837_resolution_osr_1024,
	ms5837_resolution_osr_2048,
	ms5837_resolution_osr_4096,
	ms5837_resolution_osr_8192
};

// MS5837 device commands
#define MS5837_RESET_COMMAND										0x1E
#define MS5837_START_PRESSURE_ADC_CONVERSION						0x40
#define MS5837_START_TEMPERATURE_ADC_CONVERSION						0x50
#define MS5837_READ_ADC												0x00

#define MS5837_CONVERSION_OSR_MASK									0x0F

#define MS5837_CONVERSION_TIME_OSR_256								1000
#define MS5837_CONVERSION_TIME_OSR_512								2000
#define MS5837_CONVERSION_TIME_OSR_1024								3000
#define MS5837_CONVERSION_TIME_OSR_2048								5000
#define MS5837_CONVERSION_TIME_OSR_4096								9000
#define MS5837_CONVERSION_TIME_OSR_8192								17000

// MS5837 commands
#define MS5837_PROM_ADDRESS_READ_ADDRESS_0							0xA0
#define MS5837_PROM_ADDRESS_READ_ADDRESS_1							0xA2
#define MS5837_PROM_ADDRESS_READ_ADDRESS_2							0xA4
#define MS5837_PROM_ADDRESS_READ_ADDRESS_3							0xA6
#define MS5837_PROM_ADDRESS_READ_ADDRESS_4							0xA8
#define MS5837_PROM_ADDRESS_READ_ADDRESS_5							0xAA
#define MS5837_PROM_ADDRESS_READ_ADDRESS_6							0xAC
#define MS5837_PROM_ADDRESS_READ_ADDRESS_7							0xAE

// Coefficients indexes for temperature and pressure computation
#define MS5837_CRC_INDEX											0
#define MS5837_PRESSURE_SENSITIVITY_INDEX							1 
#define MS5837_PRESSURE_OFFSET_INDEX								2
#define MS5837_TEMP_COEFF_OF_PRESSURE_SENSITIVITY_INDEX				3
#define MS5837_TEMP_COEFF_OF_PRESSURE_OFFSET_INDEX					4
#define MS5837_REFERENCE_TEMPERATURE_INDEX							5
#define MS5837_TEMP_COEFF_OF_TEMPERATURE_INDEX						6
#define MS5837_COEFFICIENT_NUMBERS									7

struct MS5837DATA
{
	double pressure; // In mbar.
	double temperature; // In Celsius degrees.
	double depth; 
	double altitude; 
	double Pressure; // In bar.
};
typedef struct MS5837DATA MS5837DATA;

struct MS5837
{
	I2CBUS I2CBus;
	uint16_t eeprom_coeff[MS5837_COEFFICIENT_NUMBERS+1];
	uint32_t conversion_time[6];
	BOOL ms5837_coeff_read;
	FILE* pfSaveFile; // Used to save raw data, should be handled specifically...
	MS5837DATA LastMS5837Data;
	char szCfgFilePath[256];
	// Parameters.
	char szDevPath[256];
	unsigned int usbissoperatingmode;
	int BaudRate;
	int nbretries;
	int timeout;
	int threadperiod;
	BOOL bSaveRawData;
	BOOL bMedianFilter;
	unsigned int addr;
	double a;
	double b;
	int ms5837_resolution_osr;
	double PressureRef;
	double WaterDensity;
};
typedef struct MS5837 MS5837;

/**
 * \brief CRC check
 *
 * \param[in] uint16_t *: List of EEPROM coefficients
 * \param[in] uint8_t : crc to compare with
 *
 * \return BOOL : TRUE if CRC is OK, FALSE if KO
 */
inline BOOL ms5837_crc_check (uint16_t *n_prom, uint8_t crc)
{
	uint8_t cnt, n_bit;
	uint16_t n_rem, crc_read;
	
	n_rem = 0x00;
	crc_read = n_prom[0];
	n_prom[MS5837_COEFFICIENT_NUMBERS] = 0;
	n_prom[0] = (0x0FFF & (n_prom[0]));    // Clear the CRC byte

	for( cnt = 0 ; cnt < (MS5837_COEFFICIENT_NUMBERS+1)*2 ; cnt++ ) {

		// Get next byte
		if (cnt%2 == 1)
			n_rem ^=  n_prom[cnt>>1] & 0x00FF ;
		else
			n_rem ^=  n_prom[cnt>>1]>>8 ;

		for( n_bit = 8; n_bit > 0 ; n_bit-- ) {

			if( n_rem & 0x8000 )
				n_rem = (n_rem << 1) ^ 0x3000;
			else
				n_rem <<= 1;
		}
	}
	n_rem >>= 12;
	n_prom[0] = crc_read;
	
	return  ( n_rem == crc );
}
	
/*
Write the MS5837 8-bits command with the value passed

uint8_t : Command value to be written.
 */
inline int ms5837_write_command(MS5837* pMS5837, uint8_t cmd)
{
	unsigned char sendbuf[1];
	int sendbuflen = 0;

	// Prepare data to send to device.
	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuf[0] = (unsigned char)cmd;
	sendbuflen = 1;

	if (WriteAllI2CBus(&pMS5837->I2CBus, (unsigned char*)sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if ((pMS5837->bSaveRawData)&&(pMS5837->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, pMS5837->pfSaveFile);
		fflush(pMS5837->pfSaveFile);
	}

	return EXIT_SUCCESS;
}

/**
 * \brief Reads the ms5837 EEPROM coefficient stored at address provided.
 *
 * \param[in] uint8_t : Address of coefficient in EEPROM
 * \param[out] uint16_t* : Value read in EEPROM
 */
inline int ms5837_read_eeprom_coeff(MS5837* pMS5837, uint8_t command, uint16_t *coeff)
{
	unsigned char recvbuf[2];
	int recvbuflen = 0;

	if (ms5837_write_command(pMS5837, command) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	// Prepare the buffer that should receive data from device.
	memset(recvbuf, 0, sizeof(recvbuf));
	recvbuflen = 2;

	if (ReadAllI2CBus(&pMS5837->I2CBus, (unsigned char*)recvbuf, recvbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if ((pMS5837->bSaveRawData)&&(pMS5837->pfSaveFile))
	{
		fwrite(recvbuf, recvbuflen, 1, pMS5837->pfSaveFile);
		fflush(pMS5837->pfSaveFile);
	}
		
	*coeff = (recvbuf[0] << 8) | recvbuf[1];
    
	if (*coeff == 0) 
	{
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;	
}

/*
Reads the ms5837 EEPROM coefficients to store them for computation.
 */
inline int ms5837_read_eeprom(MS5837* pMS5837)
{
	uint8_t i = 0;

	for (i = 0; i< MS5837_COEFFICIENT_NUMBERS; i++)
	{
		if (ms5837_read_eeprom_coeff(pMS5837, MS5837_PROM_ADDRESS_READ_ADDRESS_0 + i*2, pMS5837->eeprom_coeff+i) != EXIT_SUCCESS)
		{
			printf("Error reading data from a MS5837 : ms5837_read_eeprom_coeff failed. \n");
			return EXIT_FAILURE;
		}
	}

	if (!ms5837_crc_check(pMS5837->eeprom_coeff, (pMS5837->eeprom_coeff[MS5837_CRC_INDEX] & 0xF000)>>12))
	{
		printf("Error reading data from a MS5837 : CRC error. \n");
		return EXIT_FAILURE;
	}

	pMS5837->ms5837_coeff_read = true;

	return EXIT_SUCCESS;
}

/**
 * \brief Trigger conversion and read ADC value
 *
 * \param[in] uint8_t : Command used for conversion (will determine Temperature vs Pressure and osr)
 * \param[out] uint32_t* : ADC value.
 */
inline int ms5837_conversion_and_read_adc(MS5837* pMS5837, uint8_t cmd, uint32_t *adc)
{
	unsigned char recvbuf[3];
	int recvbuflen = 0;

	if (ms5837_write_command(pMS5837, cmd) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	// delay conversion depending on resolution
	mSleep(pMS5837->conversion_time[ (cmd & MS5837_CONVERSION_OSR_MASK)/2 ]/1000 );
	
	// Send the read command
	if (ms5837_write_command(pMS5837, MS5837_READ_ADC) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	// Prepare the buffer that should receive data from device.
	memset(recvbuf, 0, sizeof(recvbuf));
	recvbuflen = 3;

	if (ReadAllI2CBus(&pMS5837->I2CBus, (unsigned char*)recvbuf, recvbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if ((pMS5837->bSaveRawData)&&(pMS5837->pfSaveFile))
	{
		fwrite(recvbuf, recvbuflen, 1, pMS5837->pfSaveFile);
		fflush(pMS5837->pfSaveFile);
	}

	*adc = ((uint32_t)recvbuf[0] << 16) | ((uint32_t)recvbuf[1] << 8) | recvbuf[2];
	
	return EXIT_SUCCESS;
}

/**
 * \brief Reads the temperature and pressure ADC value and compute the compensated values.
 *
 * \param[out] float* : Celsius Degree temperature value
 * \param[out] float* : mbar pressure value
 */
inline int ms5837_read_temperature_and_pressure(MS5837* pMS5837, float *temperature, float *pressure)
{
	int status = EXIT_SUCCESS;
	uint32_t adc_temperature, adc_pressure;
	int32_t dT, TEMP;
	int64_t OFF, SENS, P, T2, OFF2, SENS2;
	uint8_t cmd;
	
	// If first time adc is requested, get EEPROM coefficients.
	if(!pMS5837->ms5837_coeff_read) status = ms5837_read_eeprom(pMS5837);
	if (status != EXIT_SUCCESS) {
		return EXIT_FAILURE;
	}
	
	// First read temperature
	cmd = (uint8_t)(pMS5837->ms5837_resolution_osr*2);
	cmd |= MS5837_START_TEMPERATURE_ADC_CONVERSION;
	status = ms5837_conversion_and_read_adc(pMS5837, cmd, &adc_temperature);
	if (status != EXIT_SUCCESS) {
		printf("Error reading data from a MS5837 : ms5837_conversion_and_read_adc failed. \n");
		return EXIT_FAILURE;
	}

	// Now read pressure
	cmd = (uint8_t)(pMS5837->ms5837_resolution_osr*2);
	cmd |= MS5837_START_PRESSURE_ADC_CONVERSION;
	status = ms5837_conversion_and_read_adc(pMS5837, cmd, &adc_pressure);
	if (status != EXIT_SUCCESS) {
		printf("Error reading data from a MS5837 : ms5837_conversion_and_read_adc failed. \n");
		return EXIT_FAILURE;
	}
    
	if (adc_temperature == 0 || adc_pressure == 0) {
		printf("Error reading data from a MS5837 : Unexpected data. \n");
		return EXIT_FAILURE;
	}

	// Difference between actual and reference temperature = D2 - Tref
	dT = (int32_t)adc_temperature - ((int32_t)pMS5837->eeprom_coeff[MS5837_REFERENCE_TEMPERATURE_INDEX] << 8);
	
	// Actual temperature = 2000 + dT * TEMPSENS
	TEMP = 2000 + ((int64_t)dT * (int64_t)pMS5837->eeprom_coeff[MS5837_TEMP_COEFF_OF_TEMPERATURE_INDEX] >> 23) ;
	
	// Second order temperature compensation
	if( TEMP < 2000 )
	{
		T2 = ( 3 * ( (int64_t)dT  * (int64_t)dT  ) ) >> 33;
		OFF2 = 3 * ((int64_t)TEMP - 2000) * ((int64_t)TEMP - 2000) / 2 ;
		SENS2 = 5 * ((int64_t)TEMP - 2000) * ((int64_t)TEMP - 2000) / 8 ;
		
		if( TEMP < -1500 )
		{
			OFF2 += 7 * ((int64_t)TEMP + 1500) * ((int64_t)TEMP + 1500) ;
			SENS2 += 4 * ((int64_t)TEMP + 1500) * ((int64_t)TEMP + 1500) ;
		}
	}
	else
	{
		T2 = ( 2 * ( (int64_t)dT  * (int64_t)dT  ) ) >> 37;
		OFF2 = ((int64_t)TEMP + 1500) * ((int64_t)TEMP + 1500) >> 4;
		SENS2 = 0 ;
	}
	
	// OFF = OFF_T1 + TCO * dT
	OFF = ( (int64_t)(pMS5837->eeprom_coeff[MS5837_PRESSURE_OFFSET_INDEX]) << 16 ) + ( ( (int64_t)(pMS5837->eeprom_coeff[MS5837_TEMP_COEFF_OF_PRESSURE_OFFSET_INDEX]) * dT ) >> 7 ) ;
	OFF -= OFF2 ;
	
	// Sensitivity at actual temperature = SENS_T1 + TCS * dT
	SENS = ( (int64_t)pMS5837->eeprom_coeff[MS5837_PRESSURE_SENSITIVITY_INDEX] << 15 ) + ( ((int64_t)pMS5837->eeprom_coeff[MS5837_TEMP_COEFF_OF_PRESSURE_SENSITIVITY_INDEX] * dT) >> 8 ) ;
	SENS -= SENS2 ;
	
	// Temperature compensated pressure = D1 * SENS - OFF
	P = ( ( (adc_pressure * SENS) >> 21 ) - OFF ) >> 13 ;
	
	*temperature = ( (float)TEMP - T2 ) / 100;
	*pressure = (float)P / 100;
	
	return EXIT_SUCCESS;
}

// We suppose that read operations return when a message has just been completely sent, and not randomly.
inline int GetLatestDataMS5837(MS5837* pMS5837, MS5837DATA* pMS5837Data)
{
	float temperature = 0, pressure = 0;

	if (ms5837_read_temperature_and_pressure(pMS5837, &temperature, &pressure) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	// Analyze data.

	//memset(pMS5837Data, 0, sizeof(MS5837DATA));

	pMS5837Data->temperature = (double)temperature;
	pMS5837Data->pressure = (double)pressure;

	// Conversions...
	pMS5837Data->Pressure = pMS5837Data->pressure/1000.0;

	pMS5837->LastMS5837Data = *pMS5837Data;

	return EXIT_SUCCESS;
}

// MS5837 must be initialized to 0 before (e.g. MS5837 ms5837; memset(&ms5837, 0, sizeof(MS5837));)!
inline int ConnectMS5837(MS5837* pMS5837, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];

	memset(pMS5837->szCfgFilePath, 0, sizeof(pMS5837->szCfgFilePath));
	sprintf(pMS5837->szCfgFilePath, "%.255s", szCfgFilePath);

	// If szCfgFilePath starts with "hardcoded://", parameters are assumed to be already set in the structure, 
	// otherwise it should be loaded from a configuration file.
	if (strncmp(szCfgFilePath, "hardcoded://", strlen("hardcoded://")) != 0)
	{
		memset(line, 0, sizeof(line));

		// Default values.
		memset(pMS5837->szDevPath, 0, sizeof(pMS5837->szDevPath));
		sprintf(pMS5837->szDevPath, "COM1");
		pMS5837->usbissoperatingmode = (unsigned int)0x61;
		pMS5837->BaudRate = 115200;
		pMS5837->nbretries = 2;
		pMS5837->timeout = 2000;
		pMS5837->threadperiod = 50;
		pMS5837->bSaveRawData = 1;
		pMS5837->bMedianFilter = 1;
		pMS5837->addr = DEFAULT_ADDR_MS5837;
		pMS5837->a = 1;
		pMS5837->b = 0;
		pMS5837->ms5837_resolution_osr = ms5837_resolution_osr_8192;
		pMS5837->PressureRef = 1;
		pMS5837->WaterDensity = 1000;

		// Load data from a file.
		file = fopen(szCfgFilePath, "r");
		if (file != NULL)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pMS5837->szDevPath) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%u", &pMS5837->usbissoperatingmode) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMS5837->BaudRate) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMS5837->nbretries) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMS5837->timeout) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMS5837->threadperiod) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMS5837->bSaveRawData) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMS5837->bMedianFilter) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%u", &pMS5837->addr) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pMS5837->a) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pMS5837->b) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMS5837->ms5837_resolution_osr) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pMS5837->PressureRef) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pMS5837->WaterDensity) != 1) printf("Invalid configuration file.\n");
			if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
		}
		else
		{
			printf("Configuration file not found.\n");
		}
	}

	if (pMS5837->threadperiod < 0)
	{
		printf("Invalid parameter : threadperiod.\n");
		pMS5837->threadperiod = 50;
	}

	// Used to save raw data, should be handled specifically...
	//pMS5837->pfSaveFile = NULL;

	memset(&pMS5837->LastMS5837Data, 0, sizeof(MS5837DATA));

	if (OpenI2CBus(&pMS5837->I2CBus, pMS5837->szDevPath) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a MS5837.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsI2CBus(&pMS5837->I2CBus, pMS5837->usbissoperatingmode, (UINT)pMS5837->BaudRate, (UINT)pMS5837->timeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a MS5837.\n");
		CloseI2CBus(&pMS5837->I2CBus);
		return EXIT_FAILURE;
	}

	if (SetSlaveI2CBus(&pMS5837->I2CBus, pMS5837->addr, 0, 0, pMS5837->nbretries, (UINT)pMS5837->timeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a MS5837.\n");
		CloseI2CBus(&pMS5837->I2CBus);
		return EXIT_FAILURE;
	}

	pMS5837->conversion_time[0] = MS5837_CONVERSION_TIME_OSR_256;
	pMS5837->conversion_time[1] = MS5837_CONVERSION_TIME_OSR_512;
	pMS5837->conversion_time[2] = MS5837_CONVERSION_TIME_OSR_1024;
	pMS5837->conversion_time[3] = MS5837_CONVERSION_TIME_OSR_2048;
	pMS5837->conversion_time[4] = MS5837_CONVERSION_TIME_OSR_4096;
	pMS5837->conversion_time[5] = MS5837_CONVERSION_TIME_OSR_8192;

	// Default value to ensure coefficients are read before converting temperature.
	pMS5837->ms5837_coeff_read = FALSE;

	if (ms5837_write_command(pMS5837, MS5837_RESET_COMMAND) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a MS5837.\n");
		CloseI2CBus(&pMS5837->I2CBus);
		return EXIT_FAILURE;
	}

	mSleep(10);

	printf("MS5837 connected.\n");

	return EXIT_SUCCESS;
}

inline int DisconnectMS5837(MS5837* pMS5837)
{
	if (CloseI2CBus(&pMS5837->I2CBus) != EXIT_SUCCESS)
	{
		printf("MS5837 disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("MS5837 disconnected.\n");

	return EXIT_SUCCESS;
}

#ifndef DISABLE_MS5837THREAD
THREAD_PROC_RETURN_VALUE MS5837Thread(void* pParam);
#endif // !DISABLE_MS5837THREAD

#endif // !MS5837_H
