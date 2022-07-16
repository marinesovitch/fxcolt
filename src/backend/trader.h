#ifndef INC_BACKEND_TRADER_H
#define INC_BACKEND_TRADER_H

#include "traderSink.h"
#include "common/smartTypes.h"

namespace fx
{

struct ITrader : public ITraderSink
{
	public:
		virtual ~ITrader();

	public:
		virtual void setConnection( HConnection connection ) = 0;
		virtual void executeCommand( HCommand command ) = 0;

		virtual void showTicks(const bool show) = 0;

		virtual const std::string& getStrategy( const std::string& symbol ) const = 0;
		virtual void setStrategy( const std::string& symbol, HTradingStrategy strategy ) = 0;
		virtual void removeStrategy( const std::string& symbol ) = 0;
		virtual void executeStrategyCommand( const std::string& symbol, std::istringstream& cmdLine ) = 0;

};

} // namespace fx

#endif
