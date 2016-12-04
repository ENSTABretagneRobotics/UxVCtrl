// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef AIS_H
#define AIS_H

// From http://jamsignal.blogspot.fr/2009/05/free-ais-message-decoder.html

//#include <stdlib.h>
//
//// Enable additional features in math.h.
//#	ifndef _USE_MATH_DEFINES
//#		define _USE_MATH_DEFINES
//#	endif // _USE_MATH_DEFINES
//
//#ifndef UNREFERENCED_PARAMETER
//#define UNREFERENCED_PARAMETER(P) (void)(P)
//#endif // UNREFERENCED_PARAMETER
//
//#include <math.h>
//
//#include <iostream>
//#include <iomanip>

#ifndef __GNUC__
#include <bitset>
#else
// min and max need to be undefined for this header to work...
#if !defined(DISABLE_MINMAX_UNDEFINITION)
#ifdef max
#undef max
#endif // max
#ifdef min
#undef min
#endif // min
#endif // !defined(DISABLE_MINMAX_UNDEFINITION)
#include <bitset>
#if !defined(DISABLE_MINMAX_REDEFINITION)
#if !defined(NOMINMAX) || defined(FORCE_MINMAX_DEFINITION)
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif // max
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif // min
#endif // !defined(NOMINMAX) || defined(FORCE_MINMAX_DEFINITION)
#endif // !defined(DISABLE_MINMAX_REDEFINITION)
#endif // __GNUC__

//using namespace std;

inline int bin_to_int_AIS(std::string temp_s)
{
	int i=0;
	for (int x=0; x<(int)temp_s.length(); x++) {
		i=i + ((temp_s[x]-48) * (1<<(temp_s.length()-x-1)));
		//cout<<"b2i "<<temp_s[x] <<" "<<(1<<(temp_s.length()-x-1))<<" "<< ((temp_s[x]-48) * (1<<(temp_s.length()-x-1)))<<endl;
	}
	return i;
}

inline int decode_AIS(char* aisbuf, int aisbuflen, double* pAIS_Latitude, double* pAIS_Longitude, double* pAIS_SOG, double* pAIS_COG)
{	
	UNREFERENCED_PARAMETER(aisbuflen);

	// six bit ascii table. (gpsd.berlios.de/AIVDM)
	std::string six_bit_table[120];
	six_bit_table[48]="000000";
	six_bit_table[49]="000001";
	six_bit_table[50]="000010";
	six_bit_table[51]="000011";
	six_bit_table[52]="000100";
	six_bit_table[53]="000101";
	six_bit_table[54]="000110";
	six_bit_table[55]="000111";
	six_bit_table[56]="001000";
	six_bit_table[57]="001001";
	six_bit_table[58]="001010";
	six_bit_table[59]="001011";
	six_bit_table[60]="001100";
	six_bit_table[61]="001101";
	six_bit_table[62]="001110";
	six_bit_table[63]="001111";
	six_bit_table[64]="010000";
	six_bit_table[65]="010001";
	six_bit_table[66]="010010";
	six_bit_table[67]="010011";
	six_bit_table[68]="010100";
	six_bit_table[69]="010101";
	six_bit_table[70]="010110";
	six_bit_table[71]="010111";
	six_bit_table[72]="011000";
	six_bit_table[73]="011001";
	six_bit_table[74]="011010";
	six_bit_table[75]="011011";
	six_bit_table[76]="011100";
	six_bit_table[77]="011101";
	six_bit_table[78]="011110";
	six_bit_table[79]="011111";
	six_bit_table[80]="100000";
	six_bit_table[81]="100001";
	six_bit_table[82]="100010";
	six_bit_table[83]="100011";
	six_bit_table[84]="100100";
	six_bit_table[85]="100101";
	six_bit_table[86]="100110";
	six_bit_table[87]="100111";

	six_bit_table[96]="101000";
	six_bit_table[97]="101001";
	six_bit_table[98]="101010";
	six_bit_table[99]="101011";
	six_bit_table[100]="101100";
	six_bit_table[101]="101101";
	six_bit_table[102]="101110";
	six_bit_table[103]="101111";
	six_bit_table[104]="110000";
	six_bit_table[105]="110001";
	six_bit_table[106]="110010";
	six_bit_table[107]="110011";
	six_bit_table[108]="110100";
	six_bit_table[109]="110101";
	six_bit_table[110]="110110";
	six_bit_table[111]="110111";
	six_bit_table[112]="111000";
	six_bit_table[113]="111001";
	six_bit_table[114]="111010";
	six_bit_table[115]="111011";
	six_bit_table[116]="111100";
	six_bit_table[117]="111101";
	six_bit_table[118]="111110";
	six_bit_table[119]="111111";



	std::string six_bit=aisbuf;

	// convert 6 bit string to binary
	std::string ais_binary="";
	unsigned int x = 0;
	for (x=0; x<six_bit.length(); x++) {
		int z,y;
		z=six_bit[x];
		y=(x+1)*6-5;
		ais_binary.append(six_bit_table[z]);
		//std::cout<<z<<" "<<six_bit_table[z]<<" "<<y<<std::endl;
	}

	std::string temp_s=ais_binary.substr(0,6);
	//std::bitset<6> bb (temp_s);
	//int ais_message_type=bb.to_ulong();
	int ais_message_type=bin_to_int_AIS(temp_s);

	if ((ais_message_type < 1)||(ais_message_type > 3))
	{
		// Unhandled...
		return EXIT_FAILURE;
	}

	temp_s=ais_binary.substr(6,2);
	//std::bitset<2> cc (temp_s);
	//int ais_repeat_indicator=cc.to_ulong();
	int ais_repeat_indicator=bin_to_int_AIS(temp_s);

	temp_s=ais_binary.substr(8,30);
	//std::bitset<30> dd (temp_s);
	//int ais_mmsi=dd.to_ulong();
	int ais_mmsi=bin_to_int_AIS(temp_s);

	temp_s=ais_binary.substr(38,4);
	//std::bitset<4> ee(temp_s);
	//int ais_navigation_status=ee.to_ulong();
	int ais_navigation_status=bin_to_int_AIS(temp_s);

	temp_s=ais_binary.substr(42,8);
	//std::bitset<8> ff(temp_s);
	//int ais_rate_of_turn=ff.to_ulong();
	int ais_rate_of_turn=bin_to_int_AIS(temp_s);

	temp_s=ais_binary.substr(50,10);
	//std::bitset<10> gg(temp_s);
	//int ais_speed_over_ground=gg.to_ulong()/10;
	int ais_speed_over_ground=bin_to_int_AIS(temp_s)/10;

	std::string ais_position_accuracy=ais_binary.substr(60,1);

	char west=ais_binary[61];
	temp_s=ais_binary.substr(61,28);
	std::bitset<28> hh(temp_s);
	if (west=='1') {hh.flip();}
	double ais_longitude=hh.to_ulong();
	ais_longitude=ais_longitude/10000/60;
	if (west=='1') {ais_longitude *= -1;}


	char south=ais_binary[89];
	temp_s=ais_binary.substr(89,27);
	std::bitset<27> ii(temp_s);
	if (south=='1') {ii.flip();}
	double ais_latitude=ii.to_ulong();
	ais_latitude=ais_latitude/10000/60;
	if (south=='1') {ais_latitude *= -1;}


	temp_s=ais_binary.substr(116,12);
	//std::bitset<12> jj(temp_s);
	//int ais_course_over_ground=jj.to_ulong()/10;
	int ais_course_over_ground=bin_to_int_AIS(temp_s)/10;


	temp_s=ais_binary.substr(128,9);
	//std::bitset<9> kk(temp_s);
	//int ais_true_heading=kk.to_ulong();
	int ais_true_heading=bin_to_int_AIS(temp_s);

	std::cout<<"\nmessage type = \t\t"<<ais_message_type;
	std::cout<<"\nrepeat indicator = \t"<<ais_repeat_indicator;
	std::cout<< std::setfill('0')<< "\nMMSI = \t\t\t" << std::setw(9) <<ais_mmsi;
	std::cout<<"\nnavagation status = \t"<<ais_navigation_status;
	std::cout<<"\nrate of turn = \t\t"<<ais_rate_of_turn;
	std::cout<<"\nspeed over ground = \t"<<ais_speed_over_ground;
	std::cout<<"\nposition accuracy = \t"<<ais_position_accuracy;
	std::cout<< std::fixed << "\nlongitude = \t\t"<<ais_longitude;
	std::cout<< std::fixed << "\nlatitude = \t\t"<<ais_latitude;
	std::cout<<"\ncourse over ground = \t"<<ais_course_over_ground;
	std::cout<<"\ntrue heading = \t\t"<<ais_true_heading;
	std::cout<<std::endl;

	*pAIS_Latitude = ais_latitude;
	*pAIS_Longitude = ais_longitude;
	*pAIS_SOG = ais_speed_over_ground/1.94; // Convert to speed in m/s.
	*pAIS_COG = ais_course_over_ground*M_PI/180.0; // Convert to angle in rad.

	return EXIT_SUCCESS;
}

#endif // AIS_H
