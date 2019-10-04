#pragma once 
#include "common.h"

class Settings
{
public:
	unsigned long configFrequency;
	unsigned long configBufferLevel;
	unsigned long configBufferFPS;
	unsigned long configBackendFPS;
	bool configAIEmulation;
	bool configSyncAudio;
	bool configForceSync;
	bool configDisallowSleepXA2;
	bool configDisallowSleepDS8;
	unsigned long configBitRate;
	bool configResTimer;

	u16 CartID;
	char CartName[21];
};
