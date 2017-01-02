// ComPortTest.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <windows.h>
#include <stdio.h>
using namespace std;

#define FILE_NAME	L"..\\chase\\Params.ini"


void Del_ArrPorts(unsigned int* Arr)
{
	for (int i = 0; i < 100; ++i)
	{
		Arr[i] = 0;
	}

}
void get_serial_ports(unsigned int* Arr)
{
	for (int i = 0; i < 100; ++i)
	{
		std::ostringstream os; 
		os << "COM" << i;
		std::string name = os.str();
		std::string full_name = "\\\\.\\" + name;
		HANDLE h = CreateFileA(full_name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
		if (h != INVALID_HANDLE_VALUE)
		{
			Arr[i] = 1;
			CloseHandle(h);
		}
	}
}

unsigned int CheckRequiredPort(unsigned int* Full_Arr, unsigned int* Partial_Arr)
{
	for (int i = 0; i < 100; ++i)
	{
		if ((Full_Arr[i] - Partial_Arr[i]) == 1)
		{
			return (i);
		}
	}
	return (100);
}


int _tmain(int argc, _TCHAR* argv[])
{
	std::vector<std::string> ports;
	unsigned int Arr_Ports_AllPorts[100];
	unsigned int Arr_Ports_AllPorts_WithoutOne[100];
	Del_ArrPorts(Arr_Ports_AllPorts);
	Del_ArrPorts(Arr_Ports_AllPorts_WithoutOne);
	
	std::cout << "Please remove your required Com Port from your Pc and press any key to continue..." << std::endl;
	std::cin.get();
	get_serial_ports(Arr_Ports_AllPorts_WithoutOne);

	std::cout << "Now, please connect your required Com Port to your Pc and press any key to continue..." << std::endl;
	std::cin.get();
	get_serial_ports(Arr_Ports_AllPorts);



	unsigned int RequiredPort;
	RequiredPort = CheckRequiredPort(Arr_Ports_AllPorts, Arr_Ports_AllPorts_WithoutOne);
	if (RequiredPort != 100)
	{
		std::cout << "The required Port is: Com " << RequiredPort << std::endl;

		// Write the required Com Port that found
		unsigned int myIntValue = RequiredPort;
		wchar_t wRequiredPort[3];
		swprintf_s(wRequiredPort, L"%d", myIntValue);

		WritePrivateProfileString(TEXT("RobotInterface"),
			TEXT("ComPort"),
			wRequiredPort,
			TEXT("..\\chase\\Params.ini"));
	}
	else
	{
		std::cout << "Can not identify your required Com Port" << std::endl;
	}
	std::cin.get();



	return 0;
}