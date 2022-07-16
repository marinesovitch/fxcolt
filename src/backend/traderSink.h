#ifndef INC_BACKEND_TRADERSINK_H
#define INC_BACKEND_TRADERSINK_H

#include "common/baseTypes.h"

namespace fx
{

struct STick;
struct SSymbolInfo;
struct SOrder;

struct ITraderSink
{
	virtual void onRegisterSymbol(const std::string& symbol) = 0;
	virtual void onUnregisterSymbol(const std::string& symbol) = 0;

	virtual void onTick(const STick& tick) = 0;
	virtual void onSymbol(const SSymbolInfo& symbolInfo) = 0;
	virtual void onOrder(const SOrder& order) = 0;
	virtual void onCmdResult(const std::string& output) = 0;
};

} // namespace fx

#endif
