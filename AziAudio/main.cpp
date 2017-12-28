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

#include "common.h"
#include "AudioSpec.h"

#include "SoundDriverInterface.h"
#include "SoundDriverFactory.h"

#include "audiohle.h"
//#include "rsp/rsp.h"

#include <string.h> // memcpy(), strcpy()
#include <stdio.h> // needed for configuration

#ifdef USE_PRINTF
	#include <io.h>
	#include <fcntl.h>
	#include <ios>
	using namespace std;
#endif

SoundDriverInterface *snd = NULL;

bool bLockAddrRegister = false;
u32 LockAddrRegisterValue = 0;
bool bBackendChanged = false;

#ifdef USE_PRINTF
  void RedirectIOToConsole();
#endif

HINSTANCE hInstance;

#ifdef __GNUC__
extern "C"
#endif

#ifdef _WIN32
BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,  // handle to DLL module
  DWORD fdwReason,     // reason for calling function
  LPVOID lpvReserved   // reserved
  ) {
	UNREFERENCED_PARAMETER(lpvReserved);
	UNREFERENCED_PARAMETER(fdwReason);
	hInstance = hinstDLL;
	return TRUE;
}
#endif

EXPORT void CALL DllAbout(HWND hParent) {
#if defined(_WIN32) || defined(_XBOX)
	Configuration::AboutDialog(hParent);
#else
	puts(PLUGIN_VERSION);
#endif
}

EXPORT void CALL DllConfig(HWND hParent)
{
#if defined(_WIN32) && !defined(_XBOX)
	SoundDriverType currentDriver = Configuration::getDriver();
	Configuration::ConfigDialog(hParent);
	if (currentDriver != Configuration::getDriver())
	{
		bBackendChanged = true;
	}
#else
	fputs("To do:  Implement saving configuration settings.\n", stderr);
#endif
}

EXPORT void CALL DllTest(HWND hParent) {
#if defined(_WIN32)
	MessageBoxA(hParent, "Nothing to test yet... ", "Test Box", MB_OK);
#else
	puts("DllTest");
#endif
}

// Initialization / Deinitalization Functions

// Note: We call CloseDLL just in case the audio plugin was already initialized...
AUDIO_INFO AudioInfo;
u32 Dacrate = 0;

EXPORT Boolean CALL InitiateAudio(AUDIO_INFO Audio_Info) {
	if (snd != NULL)
	{
		snd->AI_Shutdown();
		delete snd;
	}

#ifdef USE_PRINTF
	RedirectIOToConsole();
	DEBUG_OUTPUT("Logging to console enabled...\n");
#endif
	Dacrate = 0;
	//CloseDLL ();

	memcpy(&AudioInfo, &Audio_Info, sizeof(AUDIO_INFO));
	DRAM = Audio_Info.RDRAM;
	DMEM = Audio_Info.DMEM;
	IMEM = Audio_Info.IMEM;

	Configuration::LoadDefaults();
	snd = SoundDriverFactory::CreateSoundDriver(Configuration::getDriver());

	if (snd == NULL)
		return FALSE;

	snd->AI_Startup();
	bLockAddrRegister = false;
	if (*(u16*)(AudioInfo.HEADER + 0x3E) == 0x5342)
		bLockAddrRegister = true;
	LockAddrRegisterValue = 0;
	return TRUE;
}

EXPORT void CALL CloseDLL(void) {
	DEBUG_OUTPUT("Call: CloseDLL()\n");
	if (snd != NULL)
	{
		snd->AI_Shutdown();
		delete snd;
		snd = NULL;
	}
}

EXPORT void CALL GetDllInfo(PLUGIN_INFO * PluginInfo) {
	PluginInfo->MemoryBswaped = TRUE;
	PluginInfo->NormalMemory  = FALSE;
	safe_strcpy(PluginInfo->Name, 100, PLUGIN_VERSION);
	PluginInfo->Type = PLUGIN_TYPE_AUDIO;
	PluginInfo->Version = 0x0101; // Set this to retain backwards compatibility
}

EXPORT void CALL ProcessAList(void) {
	if (snd == NULL)
		return;
	HLEStart ();
}

EXPORT void CALL RomOpen(void) 
{
	DEBUG_OUTPUT("Call: RomOpen()\n");
	if (snd == NULL)
		return;
	//snd->AI_ResetAudio();
}

EXPORT void CALL RomClosed(void) 
{
	DEBUG_OUTPUT("Call: RomClosed()\n");
	Dacrate = 0; // Forces a revisit to initialize audio
	if (snd == NULL)
		return;
	if (bBackendChanged == true)
	{
		snd->AI_Shutdown();
		delete snd;
		snd = SoundDriverFactory::CreateSoundDriver(Configuration::getDriver());
		snd->AI_Startup();
		bBackendChanged = false;
	}
	else
	{
		snd->AI_ResetAudio();
	}
}

EXPORT void CALL AiDacrateChanged(int SystemType) {
	u32 Frequency, video_clock;

	DEBUG_OUTPUT("Call: AiDacrateChanged()\n");
	if (snd == NULL)
		return;
	if (Dacrate == *AudioInfo.AI_DACRATE_REG)
		return;

	Dacrate = *AudioInfo.AI_DACRATE_REG & 0x00003FFF;
#ifdef _DEBUG
	if (Dacrate != *AudioInfo.AI_DACRATE_REG)
		MessageBoxA(
			NULL,
			"Unknown/reserved bits in AI_DACRATE_REG set.",
			"Warning",
			MB_ICONWARNING
		);
#endif
	switch (SystemType) {
		default         :  assert(FALSE);
		case SYSTEM_NTSC:  video_clock = 48681812; break;
		case SYSTEM_PAL :  video_clock = 49656530; break;
		case SYSTEM_MPAL:  video_clock = 48628316; break;
	}
	Frequency = video_clock / (Dacrate + 1);
#if 0
	if ((Frequency > 7000) && (Frequency < 9000))
		Frequency = 8000;
	else if ((Frequency > 10000) && (Frequency < 12000))
		Frequency = 11025;
	else if ((Frequency > 21000) && (Frequency < 23000))
		Frequency = 22050;
	else if ((Frequency > 31000) && (Frequency < 33000))
		Frequency = 32000;
	else if ((Frequency > 43000) && (Frequency < 45000))
		Frequency = 44100;
	else if ((Frequency > 47000) && (Frequency < 49000))
		Frequency = 48000;
#endif
	snd->AI_SetFrequency(Frequency);
}

EXPORT void CALL AiLenChanged(void) 
{
	if (snd == NULL)
		return;
	if (bLockAddrRegister == true)
	{
		if (LockAddrRegisterValue == 0)
			LockAddrRegisterValue = *AudioInfo.AI_DRAM_ADDR_REG;
		snd->AI_LenChanged(
			(AudioInfo.RDRAM + (LockAddrRegisterValue & 0x00FFFFF8)),
			*AudioInfo.AI_LEN_REG & 0x3FFF8);
	}
	else
	{
		snd->AI_LenChanged(
			(AudioInfo.RDRAM + (*AudioInfo.AI_DRAM_ADDR_REG & 0x00FFFFF8)),
			*AudioInfo.AI_LEN_REG & 0x3FFF8);
	}
}

EXPORT u32 CALL AiReadLength(void) {
	if (snd == NULL)
		return 0;
	*AudioInfo.AI_LEN_REG = snd->AI_ReadLength();
	return *AudioInfo.AI_LEN_REG;
}

EXPORT void CALL AiUpdate(Boolean Wait) {
	static int intCount = 0;

	if (snd == NULL)
	{
#if defined(_WIN32) || defined(_XBOX)
		Sleep(1);
#endif
		return;
	}
	snd->AI_Update(Wait);
	return;
}

int safe_strcpy(char* dst, size_t limit, const char* src)
{
#if defined(_MSC_VER) && !defined(_XBOX)
    return strcpy_s(dst, limit, src);
#else
    size_t bytes;
    int failure;

    if (dst == NULL || src == NULL)
        return (failure = 22); /* EINVAL, from MSVC <errno.h> */

    bytes = strlen(src) + 1; /* strlen("abc") + 1 == 4 bytes */
    failure = 34; /* ERANGE, from MSVC <errno.h> */
    if (bytes > limit)
        bytes = limit;
    else
        failure = 0;

    memcpy(dst, src, bytes);
    dst[limit - 1] = '\0'; /* in case of ERANGE, may not be null-terminated */
    return (failure);
#endif
}

#ifdef USE_PRINTF
static const WORD MAX_CONSOLE_LINES = 500;
void RedirectIOToConsole() {
#if !defined(_XBOX)
	int hConHandle;
	long lStdHandle;
	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	FILE *fp;
	// allocate a console for this app
	FreeConsole();
	if (!AllocConsole())
		return;
	// set the screen buffer to be big enough to let us scroll text
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
	coninfo.dwSize.Y = MAX_CONSOLE_LINES;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);
	// redirect unbuffered STDOUT to the console
	lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen(hConHandle, "w");
	*stdout = *fp;
	setvbuf(stdout, NULL, _IONBF, 0);
	// redirect unbuffered STDIN to the console
	lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen(hConHandle, "r");
	*stdin = *fp;
	setvbuf(stdin, NULL, _IONBF, 0);
	// redirect unbuffered STDERR to the console
	lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen(hConHandle, "w");
	*stderr = *fp;
	setvbuf(stderr, NULL, _IONBF, 0);
	// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog 
	// point to console as well
	ios::sync_with_stdio();
#endif
}

#endif
