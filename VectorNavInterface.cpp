// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "VectorNavInterface.h"

// Inspired from https://github.com/ArduPilot/ardupilot...

#define LOCAL_TYPE_VECTORNAVINTERFACE 0
#define REMOTE_TYPE_VECTORNAVINTERFACE 1

// Temp...
RS232PORT VectorNavInterfacePseudoRS232Port;

//#pragma pack(show)

// Check for potential paddings in bitfields and structs, check their size and the sum of the size of their fields!

// To prevent unexpected padding in struct...
#pragma pack(push,1) 

/*
 * Copyright (C) 2010 Swift Navigation Inc.
 * Contact: Fergus Noble <fergus@swift-nav.com>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * be be distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */
/* CRC16 implementation according to CCITT standards */
static const uint16_t crc16tab[256] = {
  0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
  0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
  0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
  0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
  0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
  0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
  0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
  0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
  0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
  0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
  0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
  0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
  0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
  0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
  0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
  0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
  0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
  0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
  0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
  0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
  0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
  0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
  0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
  0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
  0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
  0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
  0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
  0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
  0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
  0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
  0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
  0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

uint16_t crc16_ccitt(const uint8_t *buf, uint32_t len, uint16_t crc)
{
    for (uint32_t i = 0; i < len; i++) {
        crc = (crc << 8) ^ crc16tab[((crc >> 8) ^ *buf++) & 0x00FF];
    }
    return crc;
}

uint64_t start_us;

struct VN_packet1 {
    uint64_t timeStartup;
    uint64_t timeGPS;
    float uncompAccel[3];
    float uncompAngRate[3];
    float pressure;
    float mag[3];
    float accel[3];
    float gyro[3];
    uint16_t sensSat;
    uint16_t AHRSStatus;
    float ypr[3];
    float quaternion[4];
    float linAccBody[3];
    float yprU[3];
    uint16_t INSStatus;
    double positionLLA[3];
    float velNED[3];
    float posU;
    float velU;
};

struct VN_packet2 {
    uint64_t timeGPS;
    float temp;
    uint8_t numGPS1Sats;
    uint8_t GPS1Fix;
    double GPS1posLLA[3];
    float GPS1velNED[3];
    float GPS1DOP[7];
    uint8_t numGPS2Sats;
    uint8_t GPS2Fix;
    float GPS2DOP[7];
};

#define VN_PKT1_HEADER { 0xfa, 0x35, 0x03, 0x00, 0x2c, 0x0f, 0x47, 0x01, 0x13, 0x06 }
#define VN_PKT2_HEADER { 0xfa, 0x4e, 0x02, 0x00, 0x10, 0x00, 0xb8, 0x20, 0x18, 0x20 }

int VectorNav_send_packet1(RS232PORT* pVectorNavInterfacePseudoRS232Port)
{
	double y = 0, p = 0, r = 0;
	double qw = 0, qx = 0, qy = 0, qz = 0;
	double lathat = 0, longhat = 0, althat = 0, _sog = 0, _cog = 0;
    struct VN_packet1 pkt;

    struct timeval tv;
	if (gettimeofday(&tv, NULL) != EXIT_SUCCESS) { tv.tv_sec = 0; tv.tv_usec = 0; }

    if (start_us == 0) {
        start_us = tv.tv_usec * 1000;
    }

	memset(&pkt, 0, sizeof(struct VN_packet1));

	EnterCriticalSection(&StateVariablesCS);
	
    pkt.timeStartup = start_us;
    pkt.timeGPS = tv.tv_usec * 1000;
    pkt.uncompAccel[0] = (float)Center(accrxhat);
    pkt.uncompAccel[1] = (float)-Center(accryhat);
    pkt.uncompAccel[2] = (float)-Center(accrzhat);
    pkt.uncompAngRate[0] = (float)Center(omegaxhat);
    pkt.uncompAngRate[1] = (float)-Center(omegayhat);
    pkt.uncompAngRate[2] = (float)-Center(omegazhat);
	
	pkt.accel[0] = (float)Center(accrxhat);
    pkt.accel[1] = (float)-Center(accryhat);
    pkt.accel[2] = (float)-Center(accrzhat);
    pkt.gyro[0] = (float)Center(omegaxhat);
    pkt.gyro[1] = (float)-Center(omegayhat);
    pkt.gyro[2] = (float)-Center(omegazhat);
    
    pkt.sensSat = 0;
    pkt.AHRSStatus = 0;

	y = fmod_2PI(-angle_env-Center(psihat)+M_PI/2.0);
	p = fmod_2PI(-Center(thetahat));
	r = fmod_2PI(Center(phihat));
	euler2quaternion(r, p, y, &qw, &qx, &qy, &qz);

	pkt.ypr[0] = (float)fmod_360_pos_rad2deg(y);
	pkt.ypr[1] = (float)fmod_360_rad2deg(p);
	pkt.ypr[2] = (float)fmod_360_rad2deg(r);

	pkt.quaternion[0] = (float)qx;
	pkt.quaternion[1] = (float)qy;
	pkt.quaternion[2] = (float)qz;
	pkt.quaternion[3] = (float)qw;

	// In Gauss?
	pkt.mag[0] = (float)(0.3*(cos(p)*cos(y)));
	pkt.mag[1] = (float)(0.3*(sin(r)*sin(p)*cos(y)-cos(r)*sin(y)));
	pkt.mag[2] = (float)(0.3*(cos(r)*sin(p)*cos(y)+sin(r)*sin(y)));
	//pkt.mag[0] = 0;
	//pkt.mag[1] = 0;
	//pkt.mag[2] = 0;

    pkt.linAccBody[0] = (float)Center(accrxhat);
    pkt.linAccBody[1] = (float)-Center(accryhat);
    pkt.linAccBody[2] = (float)-Center(accrzhat);

	pkt.yprU[0] = pkt.yprU[1] = pkt.yprU[2] = 0; //attitude uncertainty

	pkt.INSStatus = 0;
	
	EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, Center(xhat), Center(yhat), Center(zhat), &lathat, &longhat, &althat);
	_sog = sog*1.94;
	_cog = fmod_360_pos_rad2deg(-angle_env-Center(cog_gps)+M_PI/2.0);

	pkt.pressure = (float)(Height2Pressure(althat, AirPressure, 1.292)*100); // (float)(AirPressure*100+sensor_err(0,1)); // ArduPilot needs varying values to avoid Bad Baro Health message...?

    pkt.positionLLA[0] = lathat;
    pkt.positionLLA[1] = longhat;
    pkt.positionLLA[2] = althat;
    pkt.velNED[0] = (float)(_sog*cos(_cog*M_PI/180.0));
    pkt.velNED[1] = (float)(_sog*sin(_cog*M_PI/180.0));
    //pkt.velNED[2] = (float)(-Center(vrxhat)*sin(p)+Center(-vryhat)*sin(r)*cos(p)+Center(-vrzhat)*cos(r)*cos(p));
    pkt.velNED[2] = (float)Center(vz_ned);
    pkt.posU = 0.5f;
    pkt.velU = 0.25f;

	LeaveCriticalSection(&StateVariablesCS);

    const uint8_t header[] = VN_PKT1_HEADER;

	if (WriteAllRS232Port(pVectorNavInterfacePseudoRS232Port, (uint8*)&header, sizeof(header)) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if (WriteAllRS232Port(pVectorNavInterfacePseudoRS232Port, (uint8*)&pkt, sizeof(pkt)) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

    uint16_t crc = crc16_ccitt(&header[1], sizeof(header)-1, 0);
    crc = crc16_ccitt((const uint8_t *)&pkt, sizeof(pkt), crc);
    uint16_t crc2 = 0;
    swab((char*)&crc, (char*)&crc2, 2);

	//printf("pkt.ypr[0] = %f, pkt.ypr[1] = %f, pkt.ypr[2] = %f\n", (double)pkt.ypr[0], (double)pkt.ypr[1], (double)pkt.ypr[2]);

	if (WriteAllRS232Port(pVectorNavInterfacePseudoRS232Port, (uint8*)&crc2, sizeof(crc2)) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int VectorNav_send_packet2(RS232PORT* pVectorNavInterfacePseudoRS232Port)
{
	double y = 0, p = 0, r = 0;
	double lathat = 0, longhat = 0, althat = 0, _sog = 0, _cog = 0, _hdop = 0;
    struct VN_packet2 pkt;

    struct timeval tv;
	if (gettimeofday(&tv, NULL) != EXIT_SUCCESS) { tv.tv_sec = 0; tv.tv_usec = 0; }
	
	memset(&pkt, 0, sizeof(struct VN_packet2));

	EnterCriticalSection(&StateVariablesCS);

	y = fmod_2PI(-angle_env-Center(psihat)+M_PI/2.0);
	p = fmod_2PI(-Center(thetahat));
	r = fmod_2PI(Center(phihat));

	EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, Center(xhat), Center(yhat), Center(zhat), &lathat, &longhat, &althat);
	if (bCheckGNSSOK())
	{
		switch (GetGNSSlevel())
		{
		case GNSS_ACC_LEVEL_GNSS_FIX_LOW:
			pkt.GPS2Fix = pkt.GPS1Fix = 2;
			pkt.numGPS2Sats = pkt.numGPS1Sats = (uint8_t)GPS_low_acc_nbsat;
			_hdop = GPS_low_acc_HDOP;
			break;
		case GNSS_ACC_LEVEL_GNSS_FIX_MED:
			pkt.GPS2Fix = pkt.GPS1Fix = 3;
			pkt.numGPS2Sats = pkt.numGPS1Sats = (uint8_t)GPS_med_acc_nbsat;
			_hdop = GPS_med_acc_HDOP;
			break;
		case GNSS_ACC_LEVEL_GNSS_FIX_HIGH:
			pkt.GPS2Fix = pkt.GPS1Fix = 3;
			pkt.numGPS2Sats = pkt.numGPS1Sats = (uint8_t)GPS_high_acc_nbsat;
			_hdop = GPS_high_acc_HDOP;
			break;
		case GNSS_ACC_LEVEL_RTK_UNREL:
			pkt.GPS2Fix = pkt.GPS1Fix = 3;
			pkt.numGPS2Sats = pkt.numGPS1Sats = (uint8_t)GPS_low_acc_nbsat;
			_hdop = GPS_low_acc_HDOP;
			break;
		case GNSS_ACC_LEVEL_RTK_FLOAT:
			pkt.GPS2Fix = pkt.GPS1Fix = 5;
			pkt.numGPS2Sats = pkt.numGPS1Sats = (uint8_t)GPS_med_acc_nbsat;
			_hdop = GPS_med_acc_HDOP;
			break;
		case GNSS_ACC_LEVEL_RTK_FIXED:
			pkt.GPS2Fix = pkt.GPS1Fix = 6;
			pkt.numGPS2Sats = pkt.numGPS1Sats = (uint8_t)GPS_med_acc_nbsat;
			_hdop = GPS_med_acc_HDOP;
			break;
		default:
			pkt.GPS2Fix = pkt.GPS1Fix = 0;
			pkt.numGPS2Sats = pkt.numGPS1Sats = 0;
			_hdop = 0;
			break;
		}
		_sog = sog*1.94;
		_cog = fmod_360_pos_rad2deg(-angle_env-Center(cog_gps)+M_PI/2.0);
	}
	else
	{
		pkt.GPS2Fix = pkt.GPS1Fix = 1;
		pkt.numGPS2Sats = pkt.numGPS1Sats = 0;
		_hdop = 0;
		_sog = 0;
		_cog = 0;
	}

    pkt.timeGPS = tv.tv_usec * 1000ULL;
    pkt.temp = 23.5f;
    pkt.GPS1posLLA[0] = lathat;
    pkt.GPS1posLLA[1] = longhat;
    pkt.GPS1posLLA[2] = althat;
    pkt.GPS1velNED[0] = (float)(_sog*cos(_cog*M_PI/180.0));
    pkt.GPS1velNED[1] = (float)(_sog*sin(_cog*M_PI/180.0));
    //pkt.GPS1velNED[2] = (float)(-Center(vrxhat)*sin(p)+Center(-vryhat)*sin(r)*cos(p)+Center(-vrzhat)*cos(r)*cos(p));
    pkt.GPS1velNED[2] = (float)Center(vz_ned);
	pkt.GPS1DOP[0] = pkt.GPS1DOP[1] = pkt.GPS1DOP[2] = pkt.GPS1DOP[3] = pkt.GPS1DOP[4] = pkt.GPS1DOP[5] = pkt.GPS1DOP[6] = (float)_hdop;
	pkt.GPS2DOP[0] = pkt.GPS2DOP[1] = pkt.GPS2DOP[2] = pkt.GPS2DOP[3] = pkt.GPS2DOP[4] = pkt.GPS2DOP[5] = pkt.GPS2DOP[6] = (float)_hdop;

	LeaveCriticalSection(&StateVariablesCS);

    const uint8_t header[] = VN_PKT2_HEADER;

	//printf("pkt.GPS1Fix = %d, pkt.numGPS1Sats = %d, pkt.GPS1DOP[4] = %f, lathat = %f, longhat = %f, althat = %f\n", 
	//	(int)pkt.GPS1Fix, (int)pkt.numGPS1Sats, (double)pkt.GPS1DOP[4], lathat, longhat, althat);

	if (WriteAllRS232Port(pVectorNavInterfacePseudoRS232Port, (uint8*)&header, sizeof(header)) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if (WriteAllRS232Port(pVectorNavInterfacePseudoRS232Port, (uint8*)&pkt, sizeof(pkt)) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

    uint16_t crc = crc16_ccitt(&header[1], sizeof(header)-1, 0);
    crc = crc16_ccitt((const uint8_t *)&pkt, sizeof(pkt), crc);

    uint16_t crc2 = 0;
    swab((char*)&crc, (char*)&crc2, 2);

	if (WriteAllRS232Port(pVectorNavInterfacePseudoRS232Port, (uint8*)&crc2, sizeof(crc2)) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int connectvectornavinterface(RS232PORT* pVectorNavInterfacePseudoRS232Port)
{
	if (OpenRS232Port(pVectorNavInterfacePseudoRS232Port, szVectorNavInterfacePath) != EXIT_SUCCESS) 
	{
		printf("Unable to connect to a VectorNavInterface.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsRS232Port(pVectorNavInterfacePseudoRS232Port, VectorNavInterfaceBaudRate, NOPARITY, FALSE, 8, 
		ONESTOPBIT, (UINT)VectorNavInterfaceTimeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a VectorNavInterface.\n");
		CloseRS232Port(pVectorNavInterfacePseudoRS232Port);
		return EXIT_FAILURE;
	}

	printf("VectorNavInterface connected.\n");

	return EXIT_SUCCESS;
}

int disconnectvectornavinterface(RS232PORT* pVectorNavInterfacePseudoRS232Port)
{
	if (CloseRS232Port(pVectorNavInterfacePseudoRS232Port) != EXIT_SUCCESS) 
	{
		printf("VectorNavInterface disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("VectorNavInterface disconnected.\n");

	return EXIT_SUCCESS;
}

//recvlatestdatavectornavinterface(RS232PORT* pVectorNavInterfacePseudoRS232Port)

//sendlatestdatavectornavinterface(RS232PORT* pVectorNavInterfacePseudoRS232Port)

int inithandlevectornavinterface(RS232PORT* pVectorNavInterfacePseudoRS232Port)
{
	UNREFERENCED_PARAMETER(pVectorNavInterfacePseudoRS232Port);

	return EXIT_SUCCESS;
}

int handlevectornavinterface(RS232PORT* pVectorNavInterfacePseudoRS232Port)
{
	if (VectorNav_send_packet1(pVectorNavInterfacePseudoRS232Port) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if (VectorNav_send_packet2(pVectorNavInterfacePseudoRS232Port) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	uSleep(1000*20);

	return EXIT_SUCCESS;
}

int handlevectornavinterfacecli(SOCKET sockcli, void* pParam)
{
	int timeout = 5;
	RS232PORT VectorNavInterfacePseudoRS232Port_tmp = VectorNavInterfacePseudoRS232Port;

	UNREFERENCED_PARAMETER(pParam);

	// Not thread-safe...

	if (VectorNavInterfacePseudoRS232Port_tmp.DevType == TCP_SERVER_TYPE_RS232PORT) VectorNavInterfacePseudoRS232Port_tmp.s = sockcli;
				
	inithandlevectornavinterface(&VectorNavInterfacePseudoRS232Port_tmp);

	for (;;)
	{
		fd_set sock_set;
		int iResult = SOCKET_ERROR;
		struct timeval tv;

		if (bExit) break;

		tv.tv_sec = (long)(timeout/1000);
		tv.tv_usec = (long)((timeout%1000)*1000);

		// Initialize a fd_set and add the socket to it.
		FD_ZERO(&sock_set); 
		FD_SET(sockcli, &sock_set);

		iResult = select((int)sockcli+1, NULL, &sock_set, NULL, &tv);

		// Remove the socket from the set.
		// No need to use FD_ISSET() here, as we only have one socket the return value of select() is 
		// sufficient to know what happened.
		FD_CLR(sockcli, &sock_set); 

		switch (iResult)
		{
		case SOCKET_ERROR:
			return EXIT_FAILURE;
		case 0:
			// The timeout on select() occured.
			break;
		default:
			if (handlevectornavinterface(&VectorNavInterfacePseudoRS232Port_tmp) != EXIT_SUCCESS)
			{
				return EXIT_FAILURE;
			}
			break;
		}

		if (bExit) break;
	}

	return EXIT_SUCCESS;
}

THREAD_PROC_RETURN_VALUE VectorNavInterfaceThread(void* pParam)
{
	//CHRONO chrono;
	//double dt = 0, t = 0, t0 = 0;
	//struct timeval tv;

	UNREFERENCED_PARAMETER(pParam);

	//EnterCriticalSection(&strtimeCS);
	//sprintf(tlogfilename, LOG_FOLDER"tlog_%.64s.tlog", strtimeex_fns());
	//LeaveCriticalSection(&strtimeCS);
	//tlogfile = fopen(tlogfilename, "w");
	//if (tlogfile == NULL)
	//{
	//	printf("Unable to create tlog file.\n");
	//	if (!bExit) bExit = TRUE; // Unexpected program exit...
	//	return 0;
	//}

	// Try to determine whether it is a server TCP port.
	if ((szVectorNavInterfacePath[0] == ':')&&(atoi(szVectorNavInterfacePath+1) > 0))
	{
		VectorNavInterfacePseudoRS232Port.DevType = TCP_SERVER_TYPE_RS232PORT;
		while (LaunchMultiCliTCPSrv(szVectorNavInterfacePath+1, handlevectornavinterfacecli, NULL) != EXIT_SUCCESS)
		{
			printf("Error launching the VectorNavInterface server.\n");
			mSleep(4000);
			if (bExit) break;
		}
	}
	else
	{
		BOOL bConnected = FALSE;

		//t = 0;

		//StartChrono(&chrono);

		for (;;)
		{
			//uSleep(1000*20);
			//t0 = t;
			//GetTimeElapsedChrono(&chrono, &t);
			//dt = t-t0;

			//printf("VectorNavInterfaceThread period : %f s.\n", dt);

			if (!bConnected)
			{
				if (connectvectornavinterface(&VectorNavInterfacePseudoRS232Port) == EXIT_SUCCESS) 
				{
					uSleep(1000*20);
					bConnected = TRUE; 

					inithandlevectornavinterface(&VectorNavInterfacePseudoRS232Port);
				}
				else 
				{
					bConnected = FALSE;
					mSleep(1000);
				}
			}
			else
			{
				//// Time...
				//if (gettimeofday(&tv, NULL) != EXIT_SUCCESS)
				//{
				//	tv.tv_sec = 0;
				//	tv.tv_usec = 0;
				//}

				if (handlevectornavinterface(&VectorNavInterfacePseudoRS232Port) != EXIT_SUCCESS)
				{
					printf("Connection to a VectorNavInterface lost.\n");
					bConnected = FALSE;
					disconnectvectornavinterface(&VectorNavInterfacePseudoRS232Port);
					uSleep(1000*20);
				}
			}

			if (bExit) break;
		}

		//StopChrono(&chrono, &t);

		if (bConnected) disconnectvectornavinterface(&VectorNavInterfacePseudoRS232Port);
	}

	//fclose(tlogfile);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}

// Restore default alignment settings.
#pragma pack(pop) 
