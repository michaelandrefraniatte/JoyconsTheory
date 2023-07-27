using namespace std;
#include "stdafx.h"
#include <windows.h>
#include <bthsdpdef.h>
#include <bthdef.h>
#include <BluetoothAPIs.h>
#include <strsafe.h>
#include <iostream>
using namespace std;
#pragma comment(lib, "Bthprops.lib")
BLUETOOTH_DEVICE_INFO btdir;
BLUETOOTH_DEVICE_INFO btdil;
bool joyconlfound = false;
bool joyconrfound = false;
HBLUETOOTH_DEVICE_FIND hFind = NULL;
#pragma warning(disable : 4995)
extern "C"
{
	__declspec(dllexport) int connect()
	{
		int radio;
		int nRadios = 0;
		HANDLE hRadios[256];
		HBLUETOOTH_RADIO_FIND hFindRadio;
		BLUETOOTH_FIND_RADIO_PARAMS radioParam;
		radioParam.dwSize = sizeof(BLUETOOTH_FIND_RADIO_PARAMS);
		BLUETOOTH_RADIO_INFO radioInfo;
		BLUETOOTH_DEVICE_SEARCH_PARAMS srch;
		radioInfo.dwSize = sizeof(radioInfo);
		BLUETOOTH_DEVICE_INFO btdi;
		btdir.dwSize = sizeof(btdir);
		btdil.dwSize = sizeof(btdil);
		btdi.dwSize = sizeof(btdi);
		srch.dwSize = sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS);
		hFindRadio = BluetoothFindFirstRadio(&radioParam, &hRadios[nRadios++]);
		while (BluetoothFindNextRadio(hFindRadio, &hRadios[nRadios++]))
		{
			hFindRadio = BluetoothFindFirstRadio(&radioParam, &hRadios[nRadios++]);
			BluetoothFindRadioClose(hFindRadio);
		}
		for (radio = 0; radio < nRadios; radio++)
		{
			BluetoothGetRadioInfo(hRadios[radio], &radioInfo);
			srch.fReturnAuthenticated = TRUE;
			srch.fReturnRemembered = TRUE;
			srch.fReturnConnected = TRUE;
			srch.fReturnUnknown = TRUE;
			srch.fIssueInquiry = TRUE;
			srch.cTimeoutMultiplier = 2;
			srch.hRadio = hRadios[radio];
			if (hFindRadio)
			{
				BluetoothGetRadioInfo(hRadios[1], &radioInfo);
				srch.hRadio = hRadios[1];
				int nPaired = 0;
				int numberOfDevices = 2;
				hFind = BluetoothFindFirstDevice(&srch, &btdi);
				while (nPaired < numberOfDevices)
				{
					do
					{
						if (!wcscmp(btdi.szName, L"Joy-Con (R)") | !wcscmp(btdi.szName, L"Joy-Con (L)"))
						{
							if (!wcscmp(btdi.szName, L"Joy-Con (R)"))
							{
								btdir = btdi;
								joyconrfound = true;
							}
							if (!wcscmp(btdi.szName, L"Joy-Con (L)"))
							{
								btdil = btdi;
								joyconlfound = true;
							}
							WCHAR pass[6];
							DWORD pcServices = 16;
							GUID guids[16];
							pass[0] = radioInfo.address.rgBytes[0];
							pass[1] = radioInfo.address.rgBytes[1];
							pass[2] = radioInfo.address.rgBytes[2];
							pass[3] = radioInfo.address.rgBytes[3];
							pass[4] = radioInfo.address.rgBytes[4];
							pass[5] = radioInfo.address.rgBytes[5];
							BluetoothAuthenticateDevice(NULL, hRadios[1], &btdi, pass, 6);
							BluetoothEnumerateInstalledServices(hRadios[1], &btdi, &pcServices, guids);
							BluetoothSetServiceState(hRadios[1], &btdi, &HumanInterfaceDeviceServiceClass_UUID, BLUETOOTH_SERVICE_ENABLE);
						}
						nPaired++;
						Sleep(1);
					} while (BluetoothFindNextDevice(hFind, &btdi));
					Sleep(1);
				}
				if (!joyconrfound & joyconlfound)
					return 1;
				if (joyconrfound & joyconlfound)
					return 2;
				if (joyconrfound & !joyconlfound)
					return 3;
			}
		}
		return 0;
	}
	__declspec(dllexport) bool disconnect()
	{
		BluetoothRemoveDevice(&btdil.Address);
		BluetoothRemoveDevice(&btdir.Address);
		return true;
	}
}