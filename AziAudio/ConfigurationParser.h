#pragma once

#include <stdio.h>

class ConfigurationParser
{
public:
	void Parse(char* input);
	int GetInt(char* header, char* setting);
	float GetFloat(char* header, char* setting);
	char* GetString(char* header, char* setting);

	void SetInt(char* header, char* setting, int value);
	void SetFloat(char* header, char* setting, float value);
	void SetString(char* header, char* setting, char* value);
private:
	bool fetchLine(char** input, int maxLine, char* out);
	void ParseValue(char* input, char* lvalue, char* rvalue, int lvalueMaxCnt, int rvalueMaxCnt);
};