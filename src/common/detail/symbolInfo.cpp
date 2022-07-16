#include "ph.h"
#include "symbolInfo.h"

namespace fx
{

SSymbolInfo::SSymbolInfo()
{
	m_name[0] = 0;
}

SSymbolInfo::SSymbolInfo(const std::string& name)
{
	strcpy(m_name, name.c_str());
}

} // namespace fx
