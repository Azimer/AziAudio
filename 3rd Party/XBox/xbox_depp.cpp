#include "xbox_depp.h"

// Functions in Windows that don't exist on the Xbox

BOOL PathFileExists(const char *pszPath)
{   
    return GetFileAttributes(pszPath) != INVALID_FILE_ATTRIBUTES;   
}

int MessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
{
	OutputDebugString(lpText);
	OutputDebugString("\n");
	return FALSE;
}

int MessageBoxA(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
{
	return MessageBox(hWnd, lpText, lpCaption, uType);
}

BOOL TerminateThread(HANDLE hThread, DWORD dwExitCode)
{
	ExitThread(dwExitCode);
	return TRUE;
}

BOOL IsWindow(HWND hWnd)
{
	return FALSE;
}

BOOL ShowWindow(HWND hWnd, int CmdShow)
{
	return FALSE;
}

BOOL SetWindowText(HWND hWnd, LPCTSTR lpString)
{
	OutputDebugString(lpString);
	OutputDebugString("\n");
	return FALSE;
}

LONG SetWindowLong(HWND hWnd, int nIndex, LONG dwNewLong)
{
	return 0;
}

BOOL GetClientRect(HWND hWnd, LPRECT lpRect)
{
	return FALSE;
}

int ShowCursor(BOOL bShow)
{
	return 0;
}

int GetDlgCtrlID(HWND hWnd)
{
	return 0;
}

HWND GetDlgItem(HWND hDlg, int nIDDlgItem)
{
	return NULL;
}

DWORD GetModuleFileName(HMODULE hModule, LPSTR lpFilename, DWORD nSize)
{
	return 0;
}

// We could have this return a string if we ever want to use it
BOOL StrTrim(LPSTR psz, LPCSTR pszTrimChars)
{
	// needs work
	/*char szBuf[256];
	memset(szBuf, 0, sizeof(szBuf));
	
	char szString[256];
	sprintf(szString, psz);
	
	char szTrim[256];
	sprintf(szTrim, pszTrimChars);
	
	for (int i=0; i<strlen(szString); i++)
	{
		BOOL bSkip = FALSE;
		for (int j=0; j<strlen(szTrim); j++)
		{
			if (strchr(szTrim[j], szString[i]))
			{
				bSkip = TRUE;
				break;
			}	
		}
		
		if (!bSkip)
			szBuf[strlen(szBuf)] = szString[i];
	}
	szBuf[strlen(szBuf)] = '\0';
	
	if (strlen(szBuf))
	{
		sprintf(psz, szBuf);
		return TRUE;
	}
	else*/
		return FALSE;
}

BOOL WaitMessage(void)
{
	return FALSE;
}
