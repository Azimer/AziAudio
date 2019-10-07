/****************************************************************************
*                                                                           *
* Azimer's HLE Audio Plugin for Project64 Compatible N64 Emulators          *
* http://www.apollo64.com/                                                  *
* Copyright (C) 2000-2019 Azimer. All rights reserved.                      *
*                                                                           *
* License:                                                                  *
* GNU/GPLv2 http://www.gnu.org/licenses/gpl-2.0.html                        *
*                                                                           *
****************************************************************************/
#pragma once

#include "common.h"
#include "Settings.h"
#ifdef _WIN32
#include <Windows.h>
#endif

/* strcpy() */
#include <string.h>

class Configuration
{
protected:
	static const int MAX_FOLDER_LENGTH = 500;
	static const int MAX_DEVICE_LENGTH = 100;
#ifdef _WIN32
	static INT_PTR CALLBACK ConfigProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
	static INT_PTR CALLBACK AdvancedProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK SettingsProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif
	static void LoadSettings();
	static void SaveSettings();
	static unsigned long configVolume;
	static char configAudioLogFolder[MAX_FOLDER_LENGTH];
	//static LPGUID configDevice;
	static SoundDriverType configDriver;
	static Settings currentSettings;

	// Setters
	static void setAIEmulation(bool value) { currentSettings.configAIEmulation = value; }
	static void setSyncAudio(bool value)   { currentSettings.configSyncAudio = value; }
	static void setForceSync(bool value)  { currentSettings.configForceSync = value; }
	static void setVolume(unsigned long value) { configVolume = value; }
	static void setDriver(SoundDriverType value) { configDriver = value; }
	static void setFrequency(unsigned long value) { currentSettings.configFrequency = value; }
	static void setBitRate(unsigned long value) { currentSettings.configBitRate = value; }
	static void setBufferLevel(unsigned long value) { currentSettings.configBufferLevel = value; }
	static void setBufferFPS(unsigned long value) { currentSettings.configBufferFPS = value; }
	static void setBackendFPS(unsigned long value) { currentSettings.configBackendFPS = value; }
	static void setDisallowSleepXA2(bool value) { currentSettings.configDisallowSleepXA2 = value; };
	static void setDisallowSleepDS8(bool value) { currentSettings.configDisallowSleepDS8 = value; };
	static void setResTimer(bool value) { currentSettings.configResTimer = value; };

	static void ResetAdvancedPage(HWND hDlg);

public:
	static void LoadDefaults();
#ifdef _WIN32
	static void ConfigDialog(HWND hParent);
	static void AboutDialog(HWND hParent);
#endif
	// Accessors for the Configuration variables to prevent changes outside of Configuration.cpp
	static bool getAIEmulation() { return currentSettings.configAIEmulation; }
	static bool getSyncAudio()   { return currentSettings.configSyncAudio; }
	static bool getForceSync()  { return currentSettings.configForceSync; }
	static unsigned long getVolume() { return configVolume; }
	static char* getAudioLogFolder() {
		static char retVal[MAX_FOLDER_LENGTH];
		strcpy(retVal, configAudioLogFolder);
		return retVal;
	}
#if 0 /* Disable Device Configuration */
	static LPGUID getDevice() { return configDevice; }
#endif
	static SoundDriverType getDriver() { return configDriver; }
	static unsigned long getFrequency() { return currentSettings.configFrequency; }
	static unsigned long getBitRate() { return currentSettings.configBitRate; }
	static unsigned long getBufferLevel() { return currentSettings.configBufferLevel; }
	static unsigned long getBufferFPS() { return currentSettings.configBufferFPS; }
	static unsigned long getBackendFPS() { return currentSettings.configBackendFPS; }
	static bool getDisallowSleepXA2() { return currentSettings.configDisallowSleepXA2; };
	static bool getDisallowSleepDS8() { return currentSettings.configDisallowSleepDS8; };
	static bool getResTimer() { return currentSettings.configResTimer; };
};
