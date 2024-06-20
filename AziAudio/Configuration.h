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

//#include "common.h"
#include "Settings.h"
#ifdef _WIN32
#include <windows.h>
#endif

/* strcpy() */
#include <string.h>

typedef struct
{
	u16 validation;       /* 0x00 */
	u8  compression;      /* 0x02 */
	u8  unknown1;         /* 0x03 */
	u32  clockrate;        /* 0x04 */
	u32  programcounter;   /* 0x08 */
	u32  release;          /* 0x0c */
	u32  crc1;             /* 0x10 */
	u32  crc2;             /* 0x14 */
	u64 unknown2;         /* 0x18 */

	u8  name[20];         /* 0x20 - 0x33 */

	u8  unknown3;         /* 0x34 */
	u8  unknown4;         /* 0x35 */
	u8  unknown5;         /* 0x36 */
	u8  unknown6;         /* 0x37 */
	u8  unknown7;         /* 0x38 */
	u8  unknown8;         /* 0x39 */
	u8  unknown9;         /* 0x3a */
	u8  manufacturerid;   /* 0x3b */
	u16 cartridgeid;      /* 0x3c */
	u8  countrycode;      /* 0x3e */
	u8  unknown10;        /* 0x3f */
} t_romheader;

#define CONFIGFILENAME			"Config\\AziAudio.ini"

#define KEY_INTNAME				"INTERNAL_NAME"
#define KEY_SYNCAUDIO			"SyncAudio"
#define KEY_FORCESYNC			"ForceSync"
#define KEY_AIEMULATION			"AIEmulation"
#define KEY_VOLUME				"Volume"
#define KEY_DRIVER				"Driver"
#define KEY_BUFFERLEVEL			"BufferLevel"
#define KEY_BUFFERFPS			"BufferFPS"
#define KEY_BACKENDFPS			"BackendFPS"
#define KEY_DISALLOWSLEEPXA2	"DisallowSleepXA2"
#define KEY_DISALLOWSLEEPDS8	"DisallowSleepDS8"

#define SECTION_GENERAL "DEFAULT_SETTINGS"

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
	static unsigned long configVolume;
	static char configAudioLogFolder[MAX_FOLDER_LENGTH];
	//static LPGUID configDevice;
	static SoundDriverType configDriver;
	static Settings currentSettings;

	// Setters
	static void setAIEmulation(bool value)          { currentSettings.configAIEmulation = value;      }
	static void setSyncAudio(bool value)            { currentSettings.configSyncAudio = value;        }
	static void setForceSync(bool value)            { currentSettings.configForceSync = value;        }
	static void setVolume(unsigned long value)      { configVolume = value;                           }
	static void setDriver(SoundDriverType value)    { configDriver = value;                           }
	static void setFrequency(unsigned long value)   { currentSettings.configFrequency = value;        }
	static void setBitRate(unsigned long value)     { currentSettings.configBitRate = value;          }
	static void setBufferLevel(unsigned long value) { currentSettings.configBufferLevel = value;      }
	static void setBufferFPS(unsigned long value)   { currentSettings.configBufferFPS = value;        }
	static void setBackendFPS(unsigned long value)  { currentSettings.configBackendFPS = value;       }
	static void setDisallowSleepXA2(bool value)     { currentSettings.configDisallowSleepXA2 = value; }
	static void setDisallowSleepDS8(bool value)     { currentSettings.configDisallowSleepDS8 = value; }
	static void setResTimer(bool value)             { currentSettings.configResTimer = value;         }

	static void ResetAdvancedPage(HWND hDlg);

public:
	static t_romheader* Header;
	static bool RomRunning;
	static void LoadDefaults();
	static void LoadSettings();
	static void SaveSettings();
	static bool config_load();
	static bool config_load_rom();
	static bool config_save();
	static bool config_save_rom();
#ifdef _WIN32
	static void ConfigDialog(HWND hParent);
	static void AboutDialog(HWND hParent);
#endif
	// Accessors for the Configuration variables to prevent changes outside of Configuration.cpp
	static bool getAIEmulation()                    { return currentSettings.configAIEmulation;       }
	static unsigned long getVolume()                { return configVolume;                            }
	static bool getForceSync()                      { return currentSettings.configForceSync;         }
	static bool getSyncAudio()                      { return currentSettings.configSyncAudio;         }
	static SoundDriverType getDriver()              { return configDriver;                            }
	static unsigned long getFrequency()             { return currentSettings.configFrequency;         }
	static unsigned long getBitRate()               { return currentSettings.configBitRate;           }
	static unsigned long getBufferLevel()           { return currentSettings.configBufferLevel;       }
	static unsigned long getBufferFPS()             { return currentSettings.configBufferFPS;         }
	static unsigned long getBackendFPS()            { return currentSettings.configBackendFPS;        }
	static bool getDisallowSleepXA2()               { return currentSettings.configDisallowSleepXA2;  }
	static bool getDisallowSleepDS8()               { return currentSettings.configDisallowSleepDS8;  }
	static bool getResTimer()                       { return currentSettings.configResTimer;          }

	static char* getAudioLogFolder() {
		static char retVal[MAX_FOLDER_LENGTH];
		strcpy(retVal, configAudioLogFolder);
		return retVal;
	}
#if 0 /* Disable Device Configuration */
	static LPGUID getDevice() { return configDevice; }
#endif
};
