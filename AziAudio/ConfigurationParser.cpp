/****************************************************************************
*                                                                           *
* Azimer's HLE Audio Plugin for Project64 Compatible N64 Emulators          *
* http://www.apollo64.com/                                                  *
* Copyright (C) 2000-2021 Azimer. All rights reserved.                      *
*                                                                           *
* License:                                                                  *
* GNU/GPLv2 http://www.gnu.org/licenses/gpl-2.0.html                        *
*                                                                           *
****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <rapidjson/rapidjson.h>
#include "ConfigurationParser.h"

/*
*  JSON Example:
*     
* 
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
*/

bool ConfigurationParser::fetchLine(char** input, int maxLine, char* out)
{
	char* lineBuffer = out;
	char* readPtr = *input;
	bool maxLineHit = false;
	bool eofHit = false;
	int charCnt = 0;
	maxLineHit = false;
	int lastChar = -1;

	// Remove whitespace before the line
	while (*readPtr == ' ') readPtr++;

	// Get the first non-commented line
	while (*readPtr == ';')
	{
		while (*readPtr != '\r' && *readPtr != '\n' && *readPtr != '\0') { readPtr++; }
		while (*readPtr == '\r' || *readPtr == '\n' && !(eofHit = (*readPtr == '\0'))) { readPtr++; }
		while (*readPtr == ' ') readPtr++;
	}

	while (maxLineHit == false)
	{
		// Check for a comment character and if found skip everything until CRLF or terminator
		if (*readPtr == ';')
		{
			while (*readPtr != '\r' && *readPtr != '\n' && *readPtr != '\0') { readPtr++; }
		}
		if (*readPtr == '\0')
		{
			maxLineHit = eofHit = true;
		}
		if (*readPtr == '\r' || *readPtr == '\n' || charCnt == maxLine)
		{
			maxLineHit = true;
		}
		if (maxLineHit == false)
		{
			if (*readPtr != ' ') // Removes whitespace
				lastChar = charCnt;
			lineBuffer[charCnt++] = *readPtr++;
		}
	}
	// Clean CRLF characters at the end of the line
	while (*readPtr == '\r' || *readPtr == '\n' && !(eofHit = (*readPtr == '\0'))) { readPtr++; }

	lineBuffer[lastChar + 1] = '\0';
	*input = readPtr;

	return eofHit;
}
void ConfigurationParser::ParseValue(char* input, char* lvalue, char* rvalue, int lvalueMaxCnt, int rvalueMaxCnt)
{
	char* srcPtr = input;
	int rvalueCnt = 0;
	int lvalueCnt = 0;
	int eqCnt = 0;
	int eqLoc = 0;
	int i = 0;
	int rValueChar = -1;
	int lValueChar = -1;
	int inputLength = strlen(input);

	while (input[i] != '\0')
	{
		if (input[i] == '=')
		{
			eqCnt++; eqLoc = i;
		}
		i++;
	}
	if (eqCnt != 1)
	{
		rvalue[0] = lvalue[0] = '\0';
		return;
	}
	while (lvalueCnt < eqLoc)
	{
		if (lvalueCnt > lvalueMaxCnt)
		{
			rvalue[0] = lvalue[0] = '\0';
			return;
		}
		lvalue[lvalueCnt] = input[lvalueCnt];
		if (lvalue[lvalueCnt] != ' ') lValueChar = lvalueCnt;
		lvalueCnt++;
	}
	lvalue[lValueChar + 1] = '\0';

	eqLoc++; // Pass up the equal sign
	while (input[eqLoc] == ' ') eqLoc++; // Skip any spaces
	while ((rvalueCnt + eqLoc) < inputLength)
	{
		if (rvalueCnt > rvalueMaxCnt)
		{
			rvalue[0] = lvalue[0] = '\0';
			return;
		}
		rvalue[rvalueCnt] = input[rvalueCnt + eqLoc];
		if (rvalue[rvalueCnt] != ' ') rValueChar = rvalueCnt;
		rvalueCnt++;
	}
	rvalue[rValueChar + 1] = '\0';
}
void ConfigurationParser::Parse(char* input)
{
	char* readPtr = input;
	char lineBuffer[255]; // Max line length is 255
	char lvalue[50];
	char rvalue[50];
	char header[50];
	bool eoffound = false;
	while (!eoffound)
	{
		eoffound = fetchLine(&readPtr, 254, lineBuffer);
		if (lineBuffer[0] != '\0')
		{
			int maxHeaderLen = strlen(lineBuffer) - 1;
			if (lineBuffer[0] == '[' && lineBuffer[maxHeaderLen] == ']')
			{
				if (maxHeaderLen > 51) maxHeaderLen = 51;
				// Process Header	-- Not really necessary since we aren't going to use them for this...
				for (int i = 1; i < maxHeaderLen; i++)
					header[i - 1] = lineBuffer[i];
				header[maxHeaderLen-2] = '\0';
			}
			else
			{
				// Try to process it as a value
				ParseValue(lineBuffer, lvalue, rvalue, 50, 50);
			}
		}
	}
}
