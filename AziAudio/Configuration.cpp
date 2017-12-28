/****************************************************************************
*                                                                           *
* Azimer's HLE Audio Plugin for Project64 Compatible N64 Emulators          *
* http://www.apollo64.com/                                                  *
* Copyright (C) 2000-2017 Azimer. All rights reserved.                      *
*                                                                           *
* License:                                                                  *
* GNU/GPLv2 http://www.gnu.org/licenses/gpl-2.0.html                        *
*                                                                           *
****************************************************************************/
#include "Configuration.h"
#include "common.h"

#include <stdio.h>
#include "resource.h"
#include "SoundDriverInterface.h"
#include "SoundDriverFactory.h"


#ifdef _WIN32
#include <windows.h>
extern HINSTANCE hInstance; // DLL's HINSTANCE
#endif

extern SoundDriverInterface *snd;

// ************* Member Variables *************

bool Configuration::configAIEmulation;
bool Configuration::configSyncAudio;
bool Configuration::configForceSync;
unsigned long Configuration::configVolume;
char Configuration::configAudioLogFolder[MAX_FOLDER_LENGTH];
/* TODO: Enable device selection...  is this even possible across multiple API implementations? */
#if 0 /* Disable Device Configuration */
LPGUID Configuration::configDevice;
#endif
SoundDriverType Configuration::configDriver;

// Host SampleRate / BitRate - Seems to help Frank #188
unsigned long Configuration::configFrequency;
unsigned long Configuration::configBitRate;

// Used for NewAudio only
unsigned long Configuration::configBufferLevel; 
unsigned long Configuration::configBufferFPS;
unsigned long Configuration::configBackendFPS;

// Prevent various sleep states from taking place
bool Configuration::configDisallowSleepXA2;
bool Configuration::configDisallowSleepDS8;

// Todo: Setting to identify which audio backend is being used

// ************* File-scope private Variables *************

// Todo: Remove -- these need to be reconsidered
//static int SelectedDSound;
// DirectSound selection
#ifdef _WIN32
static GUID EnumDeviceGUID[20];
static char EnumDeviceName[20][100];
static int EnumDeviceCount;
static SoundDriverType EnumDriverType[10];
static int EnumDriverCount;
#endif

const char *ConfigFile = "Config/AziCfg.bin";

// Dialog Procedures
#if defined(_WIN32)
BOOL CALLBACK DSEnumProc(LPGUID lpGUID, LPCTSTR lpszDesc, LPCTSTR lpszDrvName, LPVOID lpContext);
INT_PTR CALLBACK ConfigProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AdvancedProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK SettingsProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

void Configuration::LoadSettings()
{
	size_t file_size = 0;
	unsigned char azicfg[256];
	FILE *file;
	file = fopen(ConfigFile, "rb");
	memset(azicfg, 0, sizeof(azicfg));
	if (file == NULL)
	{
		SaveSettings(); // Saves the config file with defaults
		return;
	}
	else
	{
		for (file_size = 0; file_size < sizeof(azicfg); file_size++) {
			const int character = fgetc(file);
			if (character < 0 || character > 255)
				break; /* hit EOF or a disk read error */
			azicfg[file_size] = (unsigned char)(character);
		}
		if (fclose(file) != 0)
			fputs("Failed to close config file stream.\n", stderr);
	}

	Configuration::configSyncAudio = (azicfg[0] != 0x00) ? true : false;
	Configuration::configForceSync = (azicfg[1] != 0x00) ? true : false;
	Configuration::configAIEmulation = (azicfg[2] != 0x00) ? true : false;
	Configuration::configVolume = (azicfg[3] > 100) ? 100 : azicfg[3];
	if (file_size > 4)
	{
		Configuration::configDriver = (SoundDriverType)(azicfg[4] << 8 | azicfg[5]);
		if (configDriver < 0x1000 || configDriver > 0x1FFF)
			configDriver = SND_DRIVER_NOSOUND;
		if (azicfg[6] > 0) 	configBufferLevel = azicfg[6];
		if (azicfg[7] > 0) 	configBufferFPS = azicfg[7];
		if (azicfg[8] > 0) 	configBackendFPS = azicfg[8];
		configDisallowSleepXA2 = (azicfg[9] != 0x00) ? true : false;
		configDisallowSleepDS8 = (azicfg[10] != 0x00) ? true : false;
	}
	if (SoundDriverFactory::DriverExists(configDriver) == false)
	{
		configDriver = SoundDriverFactory::DefaultDriver();
	}
}
void Configuration::SaveSettings()
{
	FILE *file;
	file = fopen(ConfigFile, "wb");
	if (file != NULL)
	{
		fprintf(file, "%c", configSyncAudio);
		fprintf(file, "%c", configForceSync);
		fprintf(file, "%c", configAIEmulation);
		fprintf(file, "%c", configVolume);
		fprintf(file, "%c%c", (configDriver >> 8) & 0xFF, configDriver & 0xFF);
		fprintf(file, "%c", configBufferLevel);
		fprintf(file, "%c", configBufferFPS);
		fprintf(file, "%c", configBackendFPS);
		fprintf(file, "%c", configDisallowSleepXA2);
		fprintf(file, "%c", configDisallowSleepDS8);
		fclose(file);
	}
}

/*
	Loads the default values expected for Configuration.  This will also load the settings from a file if the configuration file exists
*/
void Configuration::LoadDefaults()
{
	EnumDeviceCount = 0;
	EnumDriverCount = 0;
	safe_strcpy(Configuration::configAudioLogFolder, 499, "D:\\");
#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS) && !defined(_XBOX)
	strcpy_s(Configuration::configAudioLogFolder, 500, "D:\\");
#else
	strcpy(Configuration::configAudioLogFolder, "D:\\");
#endif
	// TODO: Query the system and get defaults (windows only?)
	Configuration::configSyncAudio = true;
	Configuration::configForceSync = false;
	Configuration::configAIEmulation = true;
	Configuration::configVolume = 0; /* 0:  max volume; 100:  min volume */
	configFrequency = 44100; // Not implemented -- needs testing
	configBitRate   = 16;    // Not implemented -- needs testing
	configBufferLevel = 3;  // NewAudio only - How many frames to buffer
	configBufferFPS = 45;   // NewAudio only - How much data to frame per second
	configBackendFPS = 90;  // NewAudio only - How much data to frame per second
	configDisallowSleepXA2 = false;
	configDisallowSleepDS8 = false;
#if 0 /* Disable Device Configuration */
	configDevice = NULL;
	if ((DirectSoundEnumerate(DSEnumProc, NULL)) != DS_OK)
	{
		EnumDeviceCount = 1;
		strcpy(EnumDeviceName[0], "Default");
		configDevice = NULL;
		printf("Unable to enumerate DirectSound devices\n");
	}
#endif
	EnumDriverCount = SoundDriverFactory::EnumDrivers(EnumDriverType, 10);
	configDriver = SoundDriverFactory::DefaultDriver();	
	LoadSettings();
}
#ifdef _WIN32
#pragma comment(lib, "comctl32.lib")
extern HINSTANCE hInstance;
void Configuration::ConfigDialog(HWND hParent)
{
	//DialogBox(hInstance, MAKEINTRESOURCE(IDD_CONFIG), hParent, ConfigProc);
	//return;
	PROPSHEETHEADER psh;
	PROPSHEETPAGE psp[2];
	
	memset(psp, 0, sizeof(psp));
	psp[0].dwSize = sizeof(PROPSHEETPAGE);
	psp[0].dwFlags = PSP_USETITLE;
	psp[0].hInstance = hInstance;
	psp[0].pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE_GENERAL);
	//psp[0].pszIcon = MAKEINTRESOURCE(IDI_FONT);
	psp[0].pfnDlgProc = SettingsProc;// FontDialogProc;
	psp[0].pszTitle = "Settings";// MAKEINTRESOURCE(IDS_FONT)
	psp[0].lParam = 0;
	psp[0].pfnCallback = NULL;

	psp[1].dwSize = sizeof(PROPSHEETPAGE);
	psp[1].dwFlags = PSP_USETITLE;
	psp[1].hInstance = hInstance;
	psp[1].pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE_ADVANCED);
	//psp[0].pszIcon = MAKEINTRESOURCE(IDI_FONT);
	psp[1].pfnDlgProc = AdvancedProc;// FontDialogProc;
	psp[1].pszTitle = "Advanced";// MAKEINTRESOURCE(IDS_FONT)
	psp[1].lParam = 0;
	psp[1].pfnCallback = NULL;

	memset(&psh, 0, sizeof(PROPSHEETHEADER));
	psh.dwSize = sizeof(PROPSHEETHEADER);
	psh.dwFlags = PSH_PROPSHEETPAGE;
	psh.hwndParent = hParent;
	psh.hInstance = hInstance;
	psh.pszCaption = "Audio Options";
	psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
	psh.nStartPage = 0;
	psh.ppsp = (LPCPROPSHEETPAGE)&psp;
	psh.pfnCallback = NULL;

	PropertySheet(&psh);

	SaveSettings();
}

void Configuration::AboutDialog(HWND hParent)
{
#define ABOUTMESSAGE \
	PLUGIN_VERSION\
	"\nby Azimer\n"\
	"\nHome: https://www.apollo64.com/\n"\
	"Source: https://github.com/Azimer/AziAudio/\n"\
	"\n"\
	"MusyX code credited to Bobby Smiles and Mupen64Plus\n"

	MessageBoxA(hParent, ABOUTMESSAGE, "About", MB_OK|MB_ICONINFORMATION);
}
#endif

#if 0 /* Disable Device Enumeration */
// TODO: I think this can safely be removed
#ifdef _WIN32
BOOL CALLBACK DSEnumProc(LPGUID lpGUID, LPCTSTR lpszDesc, LPCTSTR lpszDrvName, LPVOID lpContext)
{
	UNREFERENCED_PARAMETER(lpszDrvName);
	UNREFERENCED_PARAMETER(lpContext);
	if (lpGUID == NULL)
	{
		safe_strcpy(EnumDeviceName[EnumDeviceCount], 99, "Default");
		memset(&EnumDeviceGUID[EnumDeviceCount], 0, sizeof(GUID));
	}
	else
	{
		safe_strcpy(EnumDeviceName[EnumDeviceCount], 99, lpszDesc);
		memcpy(&EnumDeviceGUID[EnumDeviceCount], lpGUID, sizeof(GUID));
	}
	EnumDeviceCount++;
	return TRUE;
}
#endif
#endif

#if defined(_WIN32) && !defined(_XBOX)
INT_PTR CALLBACK Configuration::ConfigProc(
	HWND hDlg,  // handle to dialog box
	UINT uMsg,     // message
	WPARAM wParam, // first message parameter
	LPARAM lParam  // second message parameter
	) {
	UNREFERENCED_PARAMETER(lParam);
	int x; 
	switch (uMsg) {
	case WM_INITDIALOG:
		SendMessage(GetDlgItem(hDlg, IDC_DEVICE), CB_RESETCONTENT, 0, 0);
		
#if 0 /* Disable Device Enumeration */
		for (x = 0; x < EnumDeviceCount; x++) 
		{
			SendMessage(GetDlgItem(hDlg, IDC_DEVICE), CB_ADDSTRING, 0, (long)EnumDeviceName[x]);
			if (configDevice != NULL)
				if (memcmp(&EnumDeviceGUID, configDevice, sizeof(GUID)) == 0)
					SendMessage(GetDlgItem(hDlg, IDC_DEVICE), CB_SETCURSEL, x, 0);
		}
		if (configDevice == NULL)
			SendMessage(GetDlgItem(hDlg, IDC_DEVICE), CB_SETCURSEL, 0, 0);
#endif
		SendMessage(GetDlgItem(hDlg, IDC_BACKEND), CB_RESETCONTENT, 0, 0);
		for (x = 0; x < EnumDriverCount; x++) 
		{
			SendMessage(GetDlgItem(hDlg, IDC_BACKEND), CB_ADDSTRING, 0, (long)SoundDriverFactory::GetDriverDescription(EnumDriverType[x]));
			if (EnumDriverType[x] == configDriver)
				SendMessage(GetDlgItem(hDlg, IDC_BACKEND), CB_SETCURSEL, x, 0);
		}

		SendMessage(GetDlgItem(hDlg, IDC_OLDSYNC), BM_SETCHECK, Configuration::configForceSync ? BST_CHECKED : BST_UNCHECKED, 0);
		SendMessage(GetDlgItem(hDlg, IDC_AUDIOSYNC), BM_SETCHECK, Configuration::configSyncAudio ? BST_CHECKED : BST_UNCHECKED, 0);
		SendMessage(GetDlgItem(hDlg, IDC_AI), BM_SETCHECK, Configuration::configAIEmulation ? BST_CHECKED : BST_UNCHECKED, 0);
		SendMessage(GetDlgItem(hDlg, IDC_VOLUME), TBM_SETPOS, TRUE, Configuration::configVolume);
		SendMessage(GetDlgItem(hDlg, IDC_VOLUME), TBM_SETTICFREQ, 20, 0);
		SendMessage(GetDlgItem(hDlg, IDC_VOLUME), TBM_SETRANGEMIN, FALSE, 0);
		SendMessage(GetDlgItem(hDlg, IDC_VOLUME), TBM_SETRANGEMAX, FALSE, 100);
		if (Configuration::configVolume == 100)
		{
			SendMessage(GetDlgItem(hDlg, IDC_MUTE), BM_SETCHECK, BST_CHECKED, 0);
		}
		else
		{
			SendMessage(GetDlgItem(hDlg, IDC_MUTE), BM_SETCHECK, BST_UNCHECKED, 0);
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			configForceSync = SendMessage(GetDlgItem(hDlg, IDC_OLDSYNC), BM_GETSTATE, 0, 0) == BST_CHECKED ? true : false;
			configSyncAudio = SendMessage(GetDlgItem(hDlg, IDC_AUDIOSYNC), BM_GETSTATE, 0, 0) == BST_CHECKED ? true : false;
			configAIEmulation = SendMessage(GetDlgItem(hDlg, IDC_AI), BM_GETSTATE, 0, 0) == BST_CHECKED ? true : false;
#if 0 /* Disable Device Enumeration - NYI */
			x = (int)SendMessage(GetDlgItem(hDlg, IDC_DEVICE), CB_GETCURSEL, 0, 0);  // TODO: need to save and switch devices
			if (x == 0)
				configDevice = NULL;
			else
				memcpy(&configDevice, &EnumDeviceGUID[x], sizeof(GUID));
#endif
			Configuration::configVolume = SendMessage(GetDlgItem(hDlg, IDC_VOLUME), TBM_GETPOS, 0, 0);
			snd->SetVolume(Configuration::configVolume);

			x = (int)SendMessage(GetDlgItem(hDlg, IDC_BACKEND), CB_GETCURSEL, 0, 0);
			if (EnumDriverType[x] != configDriver)
				configDriver = EnumDriverType[x];

			SaveSettings();

			EndDialog(hDlg, 0);
			break;
		case IDCANCEL:
			EndDialog(hDlg, 0);
			break;
		case IDC_MUTE:
			if (IsDlgButtonChecked(hDlg, IDC_MUTE))
			{
				SendMessage(GetDlgItem(hDlg, IDC_VOLUME), TBM_SETPOS, TRUE, 100);
				snd->SetVolume(100);
			}
			else {
				SendMessage(GetDlgItem(hDlg, IDC_VOLUME), TBM_SETPOS, TRUE, configVolume);
				snd->SetVolume(configVolume);
			}
			break;
		}
		break;
	case WM_KEYDOWN:
		break;
	case WM_VSCROLL:
		short int userReq = LOWORD(wParam);
		if (userReq == TB_ENDTRACK || userReq == TB_THUMBTRACK)
		{
			int dwPosition = SendMessage(GetDlgItem(hDlg, IDC_VOLUME), TBM_GETPOS, 0, 0);
			if (dwPosition == 100)
			{
				SendMessage(GetDlgItem(hDlg, IDC_MUTE), BM_SETCHECK, BST_CHECKED, 0);
			}
			else
			{
				SendMessage(GetDlgItem(hDlg, IDC_MUTE), BM_SETCHECK, BST_UNCHECKED, 0);
			}
			configVolume = dwPosition;
			snd->SetVolume(dwPosition);
		}
		break;
	}

	return FALSE;

}

INT_PTR CALLBACK Configuration::AdvancedProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);

	switch (uMsg)
	{
	case WM_INITDIALOG:
		SendMessage(GetDlgItem(hDlg, IDC_OLDSYNC), BM_SETCHECK, configForceSync ? BST_CHECKED : BST_UNCHECKED, 0);
		SendMessage(GetDlgItem(hDlg, IDC_AUDIOSYNC), BM_SETCHECK, configSyncAudio ? BST_CHECKED : BST_UNCHECKED, 0);
		SendMessage(GetDlgItem(hDlg, IDC_AI), BM_SETCHECK, configAIEmulation ? BST_CHECKED : BST_UNCHECKED, 0);
		SendMessage(GetDlgItem(hDlg, IDC_BUFFERS), TBM_SETTICFREQ, 1, 0);
		SendMessage(GetDlgItem(hDlg, IDC_BUFFERS), TBM_SETRANGEMIN, FALSE, 2);
		SendMessage(GetDlgItem(hDlg, IDC_BUFFERS), TBM_SETRANGEMAX, FALSE, 9);
		SendMessage(GetDlgItem(hDlg, IDC_BUFFERS), TBM_SETPOS, TRUE, configBufferLevel);
		SendMessage(GetDlgItem(hDlg, IDC_SLIDER_BACKFPS), TBM_SETTICFREQ, 1, 0);
		SendMessage(GetDlgItem(hDlg, IDC_SLIDER_BACKFPS), TBM_SETRANGEMIN, FALSE, 1);
		SendMessage(GetDlgItem(hDlg, IDC_SLIDER_BACKFPS), TBM_SETRANGEMAX, FALSE, 8);
		SendMessage(GetDlgItem(hDlg, IDC_SLIDER_BACKFPS), TBM_SETPOS, TRUE, (configBackendFPS / 15));
		SendMessage(GetDlgItem(hDlg, IDC_SLIDER_BUFFERFPS), TBM_SETTICFREQ, 1, 0);
		SendMessage(GetDlgItem(hDlg, IDC_SLIDER_BUFFERFPS), TBM_SETRANGEMIN, FALSE, 1);
		SendMessage(GetDlgItem(hDlg, IDC_SLIDER_BUFFERFPS), TBM_SETRANGEMAX, FALSE, 8);
		SendMessage(GetDlgItem(hDlg, IDC_SLIDER_BUFFERFPS), TBM_SETPOS, TRUE, (configBufferFPS / 15));
		SendMessage(GetDlgItem(hDlg, IDC_DISALLOWDS8), BM_SETCHECK, configDisallowSleepDS8 ? BST_CHECKED : BST_UNCHECKED, 0);
		SendMessage(GetDlgItem(hDlg, IDC_DISALLOWXA2), BM_SETCHECK, configDisallowSleepXA2 ? BST_CHECKED : BST_UNCHECKED, 0);

		char textPos[20];
		sprintf(textPos, "%i", configBufferLevel);
		SetDlgItemText(hDlg, IDC_BUFFERS_TEXT, (LPCSTR)textPos);
		sprintf(textPos, "%i ms", 1000/configBackendFPS);
		SetDlgItemText(hDlg, IDC_SLIDER_BACKFPS_TEXT, (LPCSTR)textPos);
		sprintf(textPos, "%i ms", 1000/configBufferFPS);
		SetDlgItemText(hDlg, IDC_SLIDER_BUFFERFPS_TEXT, (LPCSTR)textPos);

		break;
	case WM_NOTIFY:
		if (((NMHDR FAR *) lParam)->code == PSN_APPLY) {
			configForceSync = SendMessage(GetDlgItem(hDlg, IDC_OLDSYNC), BM_GETSTATE, 0, 0) == BST_CHECKED ? true : false;
			configSyncAudio = SendMessage(GetDlgItem(hDlg, IDC_AUDIOSYNC), BM_GETSTATE, 0, 0) == BST_CHECKED ? true : false;
			configAIEmulation = SendMessage(GetDlgItem(hDlg, IDC_AI), BM_GETSTATE, 0, 0) == BST_CHECKED ? true : false;
			configBufferLevel = SendMessage(GetDlgItem(hDlg, IDC_BUFFERS), TBM_GETPOS, 0, 0);
			configBackendFPS = SendMessage(GetDlgItem(hDlg, IDC_SLIDER_BACKFPS), TBM_GETPOS, 0, 0)*15;
			configBufferFPS = SendMessage(GetDlgItem(hDlg, IDC_SLIDER_BUFFERFPS), TBM_GETPOS, 0, 0)*15;
			configDisallowSleepDS8 = SendMessage(GetDlgItem(hDlg, IDC_DISALLOWDS8), BM_GETSTATE, 0, 0) == BST_CHECKED ? true : false;
			configDisallowSleepXA2 = SendMessage(GetDlgItem(hDlg, IDC_DISALLOWXA2), BM_GETSTATE, 0, 0) == BST_CHECKED ? true : false;
		}
		break;
	case WM_HSCROLL:
		if (lParam != NULL)
		{
			char textPos[20];
			int dwPosition;
			switch (GetDlgCtrlID((HWND)lParam))
			{
				case IDC_BUFFERS:					
					dwPosition = SendMessage(GetDlgItem(hDlg, IDC_BUFFERS), TBM_GETPOS, 0, 0);
					sprintf(textPos, "%i", dwPosition);
					SetDlgItemText(hDlg, IDC_BUFFERS_TEXT, (LPCSTR)textPos);
					break;
				case IDC_SLIDER_BACKFPS:
					dwPosition = SendMessage(GetDlgItem(hDlg, IDC_SLIDER_BACKFPS), TBM_GETPOS, 0, 0);
					sprintf(textPos, "%i ms", (DWORD)(1000/(dwPosition*15)));
					SetDlgItemText(hDlg, IDC_SLIDER_BACKFPS_TEXT, (LPCSTR)textPos);
					break;
				case IDC_SLIDER_BUFFERFPS:
					dwPosition = SendMessage(GetDlgItem(hDlg, IDC_SLIDER_BUFFERFPS), TBM_GETPOS, 0, 0);
					sprintf(textPos, "%i ms", (DWORD)(1000 / (dwPosition * 15)));
					SetDlgItemText(hDlg, IDC_SLIDER_BUFFERFPS_TEXT, (LPCSTR)textPos);
					break;
			}

		}
	default:
		return FALSE;
	}
	return TRUE;
}

INT_PTR CALLBACK Configuration::SettingsProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int x;
	switch (uMsg)
	{
	case WM_INITDIALOG:
		SendMessage(GetDlgItem(hDlg, IDC_DEVICE), CB_RESETCONTENT, 0, 0);
		SendMessage(GetDlgItem(hDlg, IDC_BACKEND), CB_RESETCONTENT, 0, 0);
		SendMessage(GetDlgItem(hDlg, IDC_DEVICE), CB_ADDSTRING, 0, (long)"Default");
		SendMessage(GetDlgItem(hDlg, IDC_DEVICE), CB_SETCURSEL, 0, 0);
		for (x = 0; x < EnumDriverCount; x++)
		{
			SendMessage(GetDlgItem(hDlg, IDC_BACKEND), CB_ADDSTRING, 0, (long)SoundDriverFactory::GetDriverDescription(EnumDriverType[x]));
			if (EnumDriverType[x] == configDriver)
				SendMessage(GetDlgItem(hDlg, IDC_BACKEND), CB_SETCURSEL, x, 0);
		}
		SendMessage(GetDlgItem(hDlg, IDC_VOLUME), TBM_SETPOS, TRUE, Configuration::configVolume);
		SendMessage(GetDlgItem(hDlg, IDC_VOLUME), TBM_SETTICFREQ, 20, 0);
		SendMessage(GetDlgItem(hDlg, IDC_VOLUME), TBM_SETRANGEMIN, FALSE, 0);
		SendMessage(GetDlgItem(hDlg, IDC_VOLUME), TBM_SETRANGEMAX, FALSE, 100);
		if (Configuration::configVolume == 100)
		{
			SendMessage(GetDlgItem(hDlg, IDC_MUTE), BM_SETCHECK, BST_CHECKED, 0);
		}
		else
		{
			SendMessage(GetDlgItem(hDlg, IDC_MUTE), BM_SETCHECK, BST_UNCHECKED, 0);
		}
		break;
	case WM_NOTIFY:
		if (((NMHDR FAR *) lParam)->code == PSN_APPLY) {
			Configuration::configVolume = SendMessage(GetDlgItem(hDlg, IDC_VOLUME), TBM_GETPOS, 0, 0);
			snd->SetVolume(Configuration::configVolume);

			x = (int)SendMessage(GetDlgItem(hDlg, IDC_BACKEND), CB_GETCURSEL, 0, 0);
			if (EnumDriverType[x] != configDriver)
				configDriver = EnumDriverType[x];
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
			case IDC_MUTE:
			{
				if (IsDlgButtonChecked(hDlg, IDC_MUTE))
				{
					SendMessage(GetDlgItem(hDlg, IDC_VOLUME), TBM_SETPOS, TRUE, 100);
					snd->SetVolume(100);
				}
				else 
				{
					SendMessage(GetDlgItem(hDlg, IDC_VOLUME), TBM_SETPOS, TRUE, configVolume);
					snd->SetVolume(configVolume);
				}
			}
		}
		break;
	case WM_VSCROLL:
	{
		short int userReq = LOWORD(wParam);
		if (userReq == TB_ENDTRACK || userReq == TB_THUMBTRACK)
		{
			int dwPosition = SendMessage(GetDlgItem(hDlg, IDC_VOLUME), TBM_GETPOS, 0, 0);
			if (dwPosition == 100)
			{
				SendMessage(GetDlgItem(hDlg, IDC_MUTE), BM_SETCHECK, BST_CHECKED, 0);
			}
			else
			{
				SendMessage(GetDlgItem(hDlg, IDC_MUTE), BM_SETCHECK, BST_UNCHECKED, 0);
			}
			configVolume = dwPosition;
			snd->SetVolume(dwPosition);
		}
		break;
	}
	default:
		return FALSE;
	}
	return TRUE;
}



#endif
