// author: Darek Slusarczyk alias marines marinesovitch 2012-2013, 2022
#include "ph.h"
#include "traderImpl.h"
#include "trader.h"
#include "tradingStrategy.h"
#include "connection.h"
#include "common/command.h"
#include "common/symbolInfo.h"
#include "common/order.h"
#include "common/types.h"
#include "cpp/types.h"

namespace fx
{

namespace
{

typedef std::map<ticket_t, HOrder> orders_t;
typedef std::map<std::string, HTradingStrategy> symbol2strategy_t;

// ---------------------------------------------------------------------------

class KTrader : public ITrader, ICommandVisitor
{
	public:
		KTrader( const account_key_t& key );
		virtual ~KTrader();

	public:
		// ITrader
		virtual void setConnection( HConnection connection );
		virtual void executeCommand( HCommand command );

		virtual void showTicks( const bool show );

		virtual const std::string& getStrategy( const std::string& symbol ) const;
		virtual void setStrategy( const std::string& symbol, HTradingStrategy strategy );
		virtual void removeStrategy( const std::string& symbol );
		virtual void executeStrategyCommand( const std::string& symbol, std::istringstream& cmdLine );

	public:
		// ITraderSink
		virtual void onRegisterSymbol(const std::string& symbol);
		virtual void onUnregisterSymbol(const std::string& symbol);

		virtual void onTick(const STick& tick);
		virtual void onSymbol(const SSymbolInfo& symbolInfo);
		virtual void onOrder(const SOrder& order);
		virtual void onCmdResult(const std::string& output);

	public:
		// ICommandVisitor
		virtual void visitGetSymbols( KCmdGetSymbols* cmd );
		virtual void visitShowTicks( KCmdShowTicks* cmd );
		virtual void visitHideTicks( KCmdHideTicks* cmd );
		virtual void visitDefault( KCommand* cmd );

	private:
		const account_key_t m_accountKey;
		HConnection m_connection;
		cpp::stringset_t m_symbols;
		orders_t m_orders;
		bool m_showTicks = false;
		symbol2strategy_t m_symbol2strategy;

};

// ---------------------------------------------------------------------------

KTrader::KTrader( const account_key_t& key )
	: m_accountKey( key )
{
}

KTrader::~KTrader()
{
}

// ---------------------------------------------------------------------------
// ITrader

void KTrader::setConnection( HConnection connection )
{
	m_connection = connection;
}

void KTrader::executeCommand( HCommand command )
{
	if (command->isLocal())
	{
		command->accept(this);
	}
	else
	{
		m_connection->sendCommand( command );
	}
}

void KTrader::showTicks( const bool show )
{
	m_showTicks = show;
}

const std::string& KTrader::getStrategy( const std::string& symbol ) const
{
	auto it = m_symbol2strategy.find(symbol);
	if (it != m_symbol2strategy.end())
	{
		HTradingStrategy strategy = it->second;
		const std::string& result = strategy->getName();
		return result;
	}
	else
	{
		static const std::string EmptyStrategy;
		return EmptyStrategy;
	}
}

void KTrader::setStrategy( const std::string& symbol, HTradingStrategy strategy )
{
	assert( getStrategy( symbol ).empty() );
	m_symbol2strategy.insert( std::make_pair( symbol, strategy ) );
}

void KTrader::removeStrategy( const std::string& symbol )
{
	m_symbol2strategy.erase( symbol );
}

void KTrader::executeStrategyCommand( const std::string& symbol, std::istringstream& cmdLine )
{
	auto it = m_symbol2strategy.find(symbol);
	assert(it != m_symbol2strategy.end());
	HTradingStrategy strategy = it->second;
	strategy->executeCommand(cmdLine);
}


// ---------------------------------------------------------------------------
// ITraderSink

void KTrader::onRegisterSymbol(const std::string& symbol)
{
	std::cout << "onRegisterSymbol " << symbol << std::endl;
	m_symbols.insert( symbol );
}

void KTrader::onUnregisterSymbol(const std::string& symbol)
{
	std::cout << "onUnregisterSymbol " << symbol << std::endl;
	m_symbols.erase( symbol );
}

void KTrader::onTick(const STick& tick)
{
	if (m_showTicks)
	{
		std::cout << "onTick " << tick.m_symbolName << ' ' 
			<< tick.m_ask.m_value << ' ' << tick.m_bid.m_value
			<< ' ' << tick.m_last.m_value << ' ' << tick.m_time.m_value << std::endl;
	}
}

void KTrader::onSymbol(const SSymbolInfo& symbolInfo)
{
	std::cout << "KTrader::onSymbol " << symbolInfo.m_name << std::endl;
}

void KTrader::onOrder(const SOrder& order)
{
	const std::string& orderStr = SOrder::serialize(order);
	std::cout << "KTrader::onOrder " << orderStr << std::endl;
}

void KTrader::onCmdResult(const std::string& output)
{
	std::cout << "KTrader::onCmdResult " << output << std::endl;
}


// ---------------------------------------------------------------------------
// ICommandVisitor

void KTrader::visitGetSymbols( KCmdGetSymbols* /*cmd*/ )
{
	for (const auto& symbol : m_symbols) {
		std::cout << symbol << std::endl;
	}
}

void KTrader::visitShowTicks( KCmdShowTicks* /*cmd*/ )
{
	m_showTicks = true;
}

void KTrader::visitHideTicks( KCmdHideTicks* /*cmd*/ )
{
	m_showTicks = false;
}

void KTrader::visitDefault( KCommand* /*cmd*/ )
{
	assert(!"unknown command!");
}

} // anonymous namespace

// ---------------------------------------------------------------------------

ITrader* createTrader( const account_key_t& key )
{
	ITrader* trader = new KTrader( key );
	return trader;
}

} // namespace fx
