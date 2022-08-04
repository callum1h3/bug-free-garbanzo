#include "Debug.h"

void Debug::Log(const char* input_text)
{
	std::cout << input_text << "\n";
}

void Debug::Log(std::string input_text)
{
	std::cout << input_text.c_str() << "\n";
}