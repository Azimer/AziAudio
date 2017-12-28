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
#include "SoundDriverFactory.h"
#include "NoSoundDriver.h"

int SoundDriverFactory::FactoryNextSlot = 0;
SoundDriverFactory::FactoryDriversStruct SoundDriverFactory::FactoryDrivers[MAX_FACTORY_DRIVERS];

SoundDriverInterface* SoundDriverFactory::CreateSoundDriver(SoundDriverType DriverID)
{
	SoundDriverInterface *result = NULL;
	for (int x = 0; x < FactoryNextSlot; x++)
	{
		if (FactoryDrivers[x].DriverType == DriverID)
		{
			result = FactoryDrivers[x].CreateFunction();
			break;
		}
	}
	if (result == NULL)
		result = new NoSoundDriver();

	return result;
}

// Priority denotes which driver to consider the default.
// Currently priority highest to lowest:  XA2L(11), XA2(10), DS8(6), DS8L(5), NoAudio(0)
// Priority setting can be changed per build...  for example... XBox should be DS8 or DS8L since it doesn't support XA2.
// However, since these two implementations shouldn't be included in the project, DS8 and DS8L will be default.
bool SoundDriverFactory::RegisterSoundDriver(SoundDriverType DriverType, SoundDriverCreationFunction CreateFunction, char *Description, int Priority)
{
	if (FactoryNextSlot < MAX_FACTORY_DRIVERS)
	{
		FactoryDrivers[FactoryNextSlot].DriverType = DriverType;
		FactoryDrivers[FactoryNextSlot].CreateFunction = CreateFunction;
		FactoryDrivers[FactoryNextSlot].Priority = Priority;
		safe_strcpy(FactoryDrivers[FactoryNextSlot].Description, 99, Description);
		FactoryNextSlot++;
		return true;
	}
	return false;
}

// Traverse the FactoryDrivers array and find the best default driver
SoundDriverType SoundDriverFactory::DefaultDriver()
{
	int highestPriority = -1;
	SoundDriverType retVal = SoundDriverType::SND_DRIVER_NOSOUND;
	for (int x = 0; x < FactoryNextSlot; x++)
	{
		if (FactoryDrivers[x].Priority > highestPriority)
		{
			retVal = FactoryDrivers[x].DriverType;
			highestPriority = FactoryDrivers[x].Priority;
		}
	}
	return retVal;
}

int SoundDriverFactory::EnumDrivers(SoundDriverType *drivers, int max_entries)
{
	int retVal = 0;
	for (int x = 0; x < FactoryNextSlot; x++)
	{
		if (x >= max_entries) break;
		drivers[x] = FactoryDrivers[x].DriverType;
		retVal++;
	}
	return retVal;
}

const char* SoundDriverFactory::GetDriverDescription(SoundDriverType driver)
{
	for (int x = 0; x < FactoryNextSlot; x++)
	{
		if (driver == FactoryDrivers[x].DriverType)
			return FactoryDrivers[x].Description;
	}
	return "Error";
}