// author: Darek Slusarczyk alias marines marinesovitch 2012-2013, 2022
#ifndef INC_CPP_STRUTILS_H
#define INC_CPP_STRUTILS_H

#include "types.h"

namespace cpp
{

namespace su
{

std::string w2str(const wchar_t* wstr);
std::string w2str(const std::wstring& wstr);
std::wstring str2w(const char* str);
std::wstring str2w(const std::string& str);
void str2w(const std::string& str, wchar_t* wstr);

bool isInteger(const std::string& str);

std::string trim(const std::string& str);
void trim(std::string* str);

bool split(const std::string& str, const char separator, cpp::strings_t* tokens);
bool split(const std::string& str, const std::string& separator, cpp::strings_t* tokens);

} // namespace su

} // namespace cpp

#endif
