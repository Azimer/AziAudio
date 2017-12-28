#ifndef _XBOX_DEPP_H__COMMON_
#define _XBOX_DEPP_H__COMMON_

#if _MSC_VER > 1000
#pragma once
#endif //_MSC_VER > 1000

#pragma warning(disable:4018)	// signed/unsigned mismatch
#pragma warning(disable:4101)	// unreferenced local variable
#pragma warning(disable:4244)	// conversion, possible loss of data
#pragma warning(disable:4731)	// frame pointer register modified by inline assembly code

/*
 * name-mangling needed to statically link the zilmar-spec plugin within
 * Surreal64, which requires unique function names per each "plugin"
 */
#if 1
#define AiDacrateChanged        _AUDIO_AZIAUD_##AiDacrateChanged
#define AiLenChanged            _AUDIO_AZIAUD_##AiLenChanged
#define AiReadLength            _AUDIO_AZIAUD_##AiReadLength
#define AiUpdate                _AUDIO_AZIAUD_##AiUpdate
#define CloseDLL                _AUDIO_AZIAUD_##CloseDLL
#define DllAbout                _AUDIO_AZIAUD_##DllAbout
#define DllConfig               _AUDIO_AZIAUD_##DllConfig
#define DllTest                 _AUDIO_AZIAUD_##DllTest
#define GetDllInfo              _AUDIO_AZIAUD_##GetDllInfo
#define InitiateAudio           _AUDIO_AZIAUD_##InitiateAudio
#define ProcessAList            _AUDIO_AZIAUD_##ProcessAList
#define RomClosed               _AUDIO_AZIAUD_##RomClosed
#define RomOpened               _AUDIO_AZIAUD_##RomOpened

#define PluginLoaded            _AUDIO_AZIAUD_##PluginLoaded
#define AiCallBack              _AUDIO_AZIAUD_##AiCallBack
#endif

/*
 * more name-mangling for Windows stubs needed to prevent collisions with 
 * stubs in the emulator or in another statically linked plugin
 */
#if 1
#define PathFileExists		_WIN32_AZIAUD_##PathFileExists
#define MessageBox			_WIN32_AZIAUD_##MessageBox
#define MessageBoxA			_WIN32_AZIAUD_##MessageBoxA
#define TerminateThread		_WIN32_AZIAUD_##TerminateThread
#define IsWindow			_WIN32_AZIAUD_##IsWindow
#define ShowWindow			_WIN32_AZIAUD_##ShowWindow
#define SetWindowText		_WIN32_AZIAUD_##SetWindowText
#define SetWindowLong		_WIN32_AZIAUD_##SetWindowLong
#define GetClientRect		_WIN32_AZIAUD_##GetClientRect
#define ShowCursor			_WIN32_AZIAUD_##ShowCursor
#define GetDlgCtrlID		_WIN32_AZIAUD_##GetDlgCtrlID
#define GetDlgItem			_WIN32_AZIAUD_##GetDlgItem
#define GetModuleFileName	_WIN32_AZIAUD_##GetModuleFileName
#define StrTrim				_WIN32_AZIAUD_##StrTrim
#define WaitMessage			_WIN32_AZIAUD_##WaitMessage
#define DllMain				_WIN32_AZIAUD_##DllMain
#endif

#ifndef _XBOX_ICC
#include <xtl.h>
#else
#include "my_types.h"
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#define XAUDIO_LIBRARIES_UNAVAILABLE

#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)

// Message Box arg's, unused on XBOX
#define MB_ABORTRETRYIGNORE 0x00000002L
#define MB_CANCELTRYCONTINUE 0x00000006L
#define MB_HELP 0x00004000L
#define MB_OK 0x00000000L
#define MB_OKCANCEL 0x00000001L
#define MB_RETRYCANCEL 0x00000005L
#define MB_YESNO 0x00000004L
#define MB_YESNOCANCEL 0x00000003L
#define MB_ICONASTERISK 0x00000040L
#define MB_ICONERROR 0x00000010L
#define MB_ICONEXCLAMATION 0x00000030L
#define MB_ICONHAND 0x00000010L
#define MB_ICONINFORMATION 0x00000040L
#define MB_ICONQUESTION 0x00000020L
#define MB_ICONSTOP 0x00000010L
#define MB_ICONWARNING 0x00000030L

// ShowWindow arg's. unused on XBOX
#define SW_HIDE			 0
#define SW_SHOW			 5

BOOL PathFileExists(const char *pszPath);
int MessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType);
int MessageBoxA(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType);
BOOL TerminateThread(HANDLE hThread, DWORD dwExitCode);
BOOL IsWindow(HWND hWnd);
BOOL ShowWindow(HWND hWnd, int CmdShow);
BOOL SetWindowText(HWND hWnd, LPCTSTR lpString);
LONG SetWindowLong(HWND hWnd, int nIndex, LONG dwNewLong);
BOOL GetClientRect(HWND hWnd, LPRECT lpRect);
int ShowCursor(BOOL bShow);
int GetDlgCtrlID(HWND hWnd);
HWND GetDlgItem(HWND hDlg, int nIDDlgItem);
DWORD GetModuleFileName(HMODULE hModule, LPSTR lpFilename, DWORD nSize);
BOOL StrTrim(LPSTR psz, LPCSTR pszTrimChars);
BOOL WaitMessage(void);

#if defined(__cplusplus)
}
#endif

#endif //_XBOX_DEPP_H__COMMON_
