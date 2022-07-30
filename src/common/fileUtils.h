// author: Darek Slusarczyk alias marines marinesovitch 2012-2013, 2022
#ifndef INC_COMMON_FILEUTILS_H
#define INC_COMMON_FILEUTILS_H

#include "cpp/types.h"

namespace fx
{

namespace utils
{

bool pathExists(const std::string& path);

bool enumNamedPipes(const std::string& filter, cpp::strings_t* pipes);

} // namespace utils

} // namespace fx

#endif
