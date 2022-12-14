// author: Darek Slusarczyk alias marines marinesovitch 2012-2013, 2022
#ifndef INC_COMMON_SYMBOLINFO_H
#define INC_COMMON_SYMBOLINFO_H

#include "types.h"

namespace fx
{

struct SSymbolInfo
{
	public:
		char m_name[consts::MaxSymbolNameLen];

	public:
		SSymbolInfo();
		SSymbolInfo(const std::string& name);

};

} // namespace fx

#endif
