#pragma once

#include <iostream>

class Debug
{
public:
	static void Log(const char* input_text);
	static void Log(std::string input_text);
};