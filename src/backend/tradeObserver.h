#ifndef INC_BACKEND_TRADEOBSERVER_H
#define INC_BACKEND_TRADEOBSERVER_H

namespace fx
{

struct SSymbolInfo;
struct STick;
struct SOrder;

struct ITradeObserver
{

	public: 
		virtual void onRegisterSymbol(const SSymbolInfo& symbolInfo) = 0;
		virtual void onUnregisterSymbol(const SSymbolInfo& symbolInfo) = 0;
		virtual void onTick(const STick& tick) = 0;
		virtual void onOrder(const SOrder& order) = 0;

};

} // namespace fx

#endif
