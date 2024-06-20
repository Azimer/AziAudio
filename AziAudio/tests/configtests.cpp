#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>

#include "testmain.h"
#include "../Configuration.h"
#include "../common.h"


int ConfigTestDefaults(std::string& failureResult)
{
    int failCnt = 0;
    Configuration::LoadDefaults();
    failCnt += TestTrue(Configuration::getSyncAudio() == false, "LoadDefaults: SyncAudio should be false by default", failureResult);
    failCnt += TestTrue(Configuration::getForceSync() == false, "LoadDefaults: ForceSync should be false by default", failureResult);
    failCnt += TestTrue(Configuration::getAIEmulation() == true, "LoadDefaults: AIEmulation should be true by default", failureResult);
    failCnt += TestTrue(Configuration::getVolume() == 0, "LoadDefaults: Volume should be 0 by default", failureResult);
    failCnt += TestTrue(Configuration::getDriver() == SND_DRIVER_DS8, "LoadDefaults: Drive should be DS8 by default", failureResult);
    failCnt += TestTrue(Configuration::getBufferLevel() == 3, "LoadDefaults: BufferLevel should be 3 by default", failureResult);
    failCnt += TestTrue(Configuration::getBufferFPS() == 45, "LoadDefaults: BufferFPS should be 45 by default", failureResult);
    failCnt += TestTrue(Configuration::getBackendFPS() == 90, "LoadDefaults: BackendFPS should be 90 by default", failureResult);
    failCnt += TestTrue(Configuration::getDisallowSleepDS8() == 0, "LoadDefaults: DisallowSleepDS8 should be false by default", failureResult);
    failCnt += TestTrue(Configuration::getDisallowSleepXA2() == 0, "LoadDefaults: DisallowSleepXA2 should be false by default", failureResult);
    return failCnt;
}

static const char* ConfigTestSaveINI =\
"[DEFAULT_SETTINGS]\r\n"
"SyncAudio = 0\r\n"
"ForceSync = 0\r\n"
"AIEmulation = 1\r\n"
"Volume = 0\r\n"
"Driver = 4097\r\n"
"BufferLevel = 3\r\n"
"BufferFPS = 45\r\n"
"BackendFPS = 90\r\n"
"DisallowSleepXA2 = 0\r\n"
"DisallowSleepDS8 = 0\r\n";

static const char* ConfigTestSaveINIModded =\
"[DEFAULT_SETTINGS]\r\n"
"SyncAudio = 1\r\n"
"ForceSync = 1\r\n"
"AIEmulation = 0\r\n"
"Volume = 50\r\n"
"Driver = 4099\r\n"
"BufferLevel = 6\r\n"
"BufferFPS = 48\r\n"
"BackendFPS = 98\r\n"
"DisallowSleepXA2 = 0\r\n"
"DisallowSleepDS8 = 1\r\n";

static const char* ConfigTestLoadINI =\
"[DEFAULT_SETTINGS]\r\n"
"SyncAudio=1\r\n"
"ForceSync=0\r\n"
"AIEmulation=1\r\n"
"Volume=0\r\n"
"Driver=4099\r\n"
"BufferLevel=2\r\n"
"BufferFPS=30\r\n"
"BackendFPS=75\r\n"
"DisallowSleepXA2=1\r\n"
"DisallowSleepDS8=1\r\n";

static const char *ConfigTestLoadINIPart =\
"[DEFAULT_SETTINGS]\r\n"
"SyncAudio=1\r\n"
"ForceSync=0\r\n"
"AIEmulation=1\r\n"
"Volume=0\r\n"
"Driver=4099\r\n"
"BufferLevel=2\r\n"
"BufferFPS=30\r\n"
"BackendFPS=75\r\n"
"DisallowSleepXA2=1\r\n"
"DisallowSleepDS8=1\r\n"
"[DEADBEEF-ABBACDDC-C:00]\r\n"
"INTERNAL_NAME=tseTgamI\r\n"
"SyncAudio=0\r\n"
;

static const char *ConfigTestLoadINIRom =\
"[DEFAULT_SETTINGS]\r\n"
"SyncAudio=1\r\n"
"ForceSync=0\r\n"
"AIEmulation=1\r\n"
"Volume=0\r\n"
"Driver=4099\r\n"
"BufferLevel=2\r\n"
"BufferFPS=30\r\n"
"BackendFPS=75\r\n"
"DisallowSleepXA2=1\r\n"
"DisallowSleepDS8=1\r\n"
"[DEADBEEF-ABBACDDC-C:00]\r\n"
"INTERNAL_NAME=tseTgamI\r\n"
"SyncAudio=0\r\n"
"ForceSync=0\r\n"
"AIEmulation=1\r\n"
"Volume=50\r\n"
"Driver=4099\r\n"
"BufferLevel=3\r\n"
"BufferFPS=33\r\n"
"BackendFPS=66\r\n"
"DisallowSleepXA2=0\r\n"
"DisallowSleepDS8=0\r\n";

class TestConfiguration : Configuration
{
    public:
    static void SetTestValues()
    {
        setSyncAudio(true);
        setForceSync(true);
        setAIEmulation(false);
        setVolume(50);
        setDriver(SND_DRIVER_XA2);
        setBufferLevel(6);
        setBufferFPS(48);
        setBackendFPS(98);
        setDisallowSleepDS8(true);
        setDisallowSleepXA2(false);
    }
};

static t_romheader testHeader;

void ResetConfiguration()
{
    std::remove(CONFIGFILENAME);
    Configuration::RomRunning = false;
    std::memset(&testHeader, 0, sizeof(t_romheader));
    testHeader.crc1 = 0xDEADBEEF;
    testHeader.crc2 = 0xABBACDDC;
    std::memcpy(testHeader.name, "TestImage", 9);
	Configuration::Header = &testHeader;
}

bool validateINIFile(const char* expectedContents, std::string& failureResult)
{
    std::ifstream input;
    input.open(CONFIGFILENAME, std::ios::binary | std::ios::ate);
    if (!input.is_open())
    {
        failureResult += CONFIGFILENAME " is not found or unable to be opened.";
        return 1;
    }    
    std::streamsize size = input.tellg();
    input.seekg(0, std::ios::beg);
    std::vector<char> buffer(size);

    if (!input.read(buffer.data(), size))
    {
        failureResult += CONFIGFILENAME " is not able to be read.";
        input.close();
        return 1;
    }
    input.close();
    if (memcmp(buffer.data(), expectedContents, std::min<int>(strlen(expectedContents), size)) != 0)
    {
        failureResult += "INI contents do not match what is expected.";
        return 1;
    }
    return 0;
}

int ConfigTestSave(std::string& failureResult)
{
    ResetConfiguration();
    // Validate saving defaults works
    Configuration::LoadDefaults();
    Configuration::SaveSettings();
    if (validateINIFile(ConfigTestSaveINI, failureResult))
    { 
        std::cout << ConfigTestSaveINI << std::endl; 
        failureResult = "ERROR: ConfigTestSave: " + failureResult;
        return 1; 
    };

    return 0;
}

int ConfigTestSaveModified(std::string& failureResult)
{
    ResetConfiguration();
    // Validate saving modified defaults works
    // Make changes and save
    Configuration::LoadDefaults();
    TestConfiguration::SetTestValues();
    Configuration::SaveSettings();
    if (validateINIFile(ConfigTestSaveINIModded, failureResult))
    { 
        std::cout << ConfigTestSaveINI << std::endl; 
        failureResult = "ERROR: ConfigTestSave: " + failureResult;
        return 1; 
    };

    return 0;
}


int ConfigTestLoad(std::string& failureResult)
{
    ResetConfiguration();
    std::ofstream output;
    output.open(CONFIGFILENAME, std::ios::binary | std::ios::ate);
    output.write(ConfigTestLoadINI, strlen(ConfigTestLoadINI));
    output.close();

    Configuration::LoadDefaults();
    Configuration::LoadSettings();

    int failCnt = 0;
    failCnt += TestTrue(Configuration::getSyncAudio() == true, "ConfigTestLoad: SyncAudio should be true", failureResult);
    failCnt += TestTrue(Configuration::getForceSync() == false, "ConfigTestLoad: ForceSync should be false", failureResult);
    failCnt += TestTrue(Configuration::getAIEmulation() == true, "ConfigTestLoad: AIEmulation should be true", failureResult);
    failCnt += TestTrue(Configuration::getVolume() == 0, "ConfigTestLoad: Volume should be 0", failureResult);
    failCnt += TestTrue(Configuration::getDriver() == SND_DRIVER_XA2, "ConfigTestLoad: Driver should be XA2", failureResult);
    failCnt += TestTrue(Configuration::getBufferLevel() == 2, "ConfigTestLoad: BufferLevel should be 2", failureResult);
    failCnt += TestTrue(Configuration::getBufferFPS() == 30, "ConfigTestLoad: BufferFPS should be 30", failureResult);
    failCnt += TestTrue(Configuration::getBackendFPS() == 75, "ConfigTestLoad: BackendFPS should be 75", failureResult);
    failCnt += TestTrue(Configuration::getDisallowSleepDS8() == 1, "ConfigTestLoad: DisallowSleepDS8 should be true", failureResult);
    failCnt += TestTrue(Configuration::getDisallowSleepXA2() == 1, "ConfigTestLoad: DisallowSleepXA2 should be true", failureResult);
    return failCnt;
}

int ConfigTestLoadRomMissing(std::string& failureResult)
{
    ResetConfiguration();
    Configuration::RomRunning = true;
    std::ofstream output;
    output.open(CONFIGFILENAME, std::ios::binary | std::ios::ate);
    output.write(ConfigTestLoadINI, strlen(ConfigTestLoadINI));
    output.close();

    Configuration::LoadDefaults();
    Configuration::LoadSettings();

    int failCnt = 0;
    failCnt += TestTrue(Configuration::getSyncAudio() == true, "ConfigTestLoad: SyncAudio should be true", failureResult);
    failCnt += TestTrue(Configuration::getForceSync() == false, "ConfigTestLoad: ForceSync should be false", failureResult);
    failCnt += TestTrue(Configuration::getAIEmulation() == true, "ConfigTestLoad: AIEmulation should be true", failureResult);
    failCnt += TestTrue(Configuration::getVolume() == 0, "ConfigTestLoad: Volume should be 0", failureResult);
    failCnt += TestTrue(Configuration::getDriver() == SND_DRIVER_XA2, "ConfigTestLoad: Driver should be XA2", failureResult);
    failCnt += TestTrue(Configuration::getBufferLevel() == 2, "ConfigTestLoad: BufferLevel should be 2", failureResult);
    failCnt += TestTrue(Configuration::getBufferFPS() == 30, "ConfigTestLoad: BufferFPS should be 30", failureResult);
    failCnt += TestTrue(Configuration::getBackendFPS() == 75, "ConfigTestLoad: BackendFPS should be 75", failureResult);
    failCnt += TestTrue(Configuration::getDisallowSleepDS8() == 1, "ConfigTestLoad: DisallowSleepDS8 should be true", failureResult);
    failCnt += TestTrue(Configuration::getDisallowSleepXA2() == 1, "ConfigTestLoad: DisallowSleepXA2 should be true", failureResult);
    return failCnt;
}

int ConfigTestLoadRomPresent(std::string& failureResult)
{
    ResetConfiguration();
    Configuration::RomRunning = true;
    std::ofstream output;
    output.open(CONFIGFILENAME, std::ios::binary | std::ios::ate);
    output.write(ConfigTestLoadINIRom, strlen(ConfigTestLoadINIRom));
    output.close();

    Configuration::LoadDefaults();
    Configuration::LoadSettings();

    int failCnt = 0;
    failCnt += TestTrue(Configuration::getSyncAudio() == false, "ConfigTestLoadRomPresent: SyncAudio should be false", failureResult);
    failCnt += TestTrue(Configuration::getForceSync() == false, "ConfigTestLoadRomPresent: ForceSync should be false", failureResult);
    failCnt += TestTrue(Configuration::getAIEmulation() == true, "ConfigTestLoadRomPresent: AIEmulation should be true", failureResult);
    failCnt += TestTrue(Configuration::getVolume() == 50, "ConfigTestLoadRomPresent: Volume should be 50", failureResult);
    failCnt += TestTrue(Configuration::getDriver() == SND_DRIVER_XA2, "ConfigTestLoadRomPresent: Driver should be XA2", failureResult);
    failCnt += TestTrue(Configuration::getBufferLevel() == 3, "ConfigTestLoadRomPresent: BufferLevel should be 3", failureResult);
    failCnt += TestTrue(Configuration::getBufferFPS() == 33, "ConfigTestLoadRomPresent: BufferFPS should be 33", failureResult);
    failCnt += TestTrue(Configuration::getBackendFPS() == 66, "ConfigTestLoadRomPresent: BackendFPS should be 66", failureResult);
    failCnt += TestTrue(Configuration::getDisallowSleepDS8() == 0, "ConfigTestLoadRomPresent: DisallowSleepDS8 should be true", failureResult);
    failCnt += TestTrue(Configuration::getDisallowSleepXA2() == 0, "ConfigTestLoadRomPresent: DisallowSleepXA2 should be true", failureResult);
    return failCnt;
}

int ConfigTestLoadRomPartial(std::string& failureResult)
{
    ResetConfiguration();
    Configuration::RomRunning = true;
    std::ofstream output;
    output.open(CONFIGFILENAME, std::ios::binary | std::ios::ate);
    output.write(ConfigTestLoadINIPart, strlen(ConfigTestLoadINIPart));
    output.close();

    Configuration::LoadDefaults();
    Configuration::LoadSettings();

    int failCnt = 0;
    failCnt += TestTrue(Configuration::getSyncAudio() == false, "ConfigTestLoadRomPartial: SyncAudio should be true", failureResult);
    failCnt += TestTrue(Configuration::getForceSync() == false, "ConfigTestLoadRomPartial: ForceSync should be false", failureResult);
    failCnt += TestTrue(Configuration::getAIEmulation() == true, "ConfigTestLoadRomPartial: AIEmulation should be true", failureResult);
    failCnt += TestTrue(Configuration::getVolume() == 0, "ConfigTestLoadRomPartial: Volume should be 0", failureResult);
    failCnt += TestTrue(Configuration::getDriver() == SND_DRIVER_XA2, "ConfigTestLoadRomPartial: Driver should be XA2", failureResult);
    failCnt += TestTrue(Configuration::getBufferLevel() == 2, "ConfigTestLoadRomPartial: BufferLevel should be 2", failureResult);
    failCnt += TestTrue(Configuration::getBufferFPS() == 30, "ConfigTestLoadRomPartial: BufferFPS should be 30", failureResult);
    failCnt += TestTrue(Configuration::getBackendFPS() == 75, "ConfigTestLoadRomPartial: BackendFPS should be 75", failureResult);
    failCnt += TestTrue(Configuration::getDisallowSleepDS8() == 1, "ConfigTestLoadRomPartial: DisallowSleepDS8 should be true", failureResult);
    failCnt += TestTrue(Configuration::getDisallowSleepXA2() == 1, "ConfigTestLoadRomPartial: DisallowSleepXA2 should be true", failureResult);
    return failCnt;
}

