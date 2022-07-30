// author: Darek Slusarczyk alias marines marinesovitch 2012-2013, 2022
#include "ph.h"
#include "tradeManagerImpl.h"
#include "tradeManager.h"
#include "tradeObserver.h"
#include "trader.h"
#include "traderImpl.h"
#include "communicator.h"
#include "common/consts.h"
#include "common/notes.h"
#include "common/order.h"
#include "common/symbolInfo.h"
#include "common/types.h"
#include "common/utils.h"
#include "cpp/types.h"

namespace fx
{

namespace
{


typedef std::map< account_key_t, HTrader > traders_t;

// ---------------------------------------------------------------------------

//class KTradeObserverStub : public ITradeObserver
//{
//	public: 
//		virtual void onRegisterSymbol(const SSymbolInfo& symbolInfo);
//		virtual void onUnregisterSymbol(const SSymbolInfo& symbolInfo);
//		virtual void onTick(const STick& tick);
//		virtual void onOrder(const SOrder& order);
//
//	private: 
//		KTradeObserverStub();
//
//	public: 
//		static KTradeObserverStub s_instance;
//
//};
//
//void KTradeObserverStub::onRegisterSymbol(const SSymbolInfo& symbolInfo)
//{
////	std::cout << "onRegisterSymbol" << " " << symbolInfo.m_label << std::endl;
//	std::cout << "onRegisterSymbol" << std::endl;
//}
//
//
//void KTradeObserverStub::onUnregisterSymbol(const SSymbolInfo& symbolInfo)
//{
//	std::cout << "onUnregisterSymbol" << std::endl;
//}
//
//
//void KTradeObserverStub::onTick(const STick& tick)
//{
////	std::cout << "onTick" << std::endl;
//}
//
//
//void KTradeObserverStub::onOrder(const SOrder& order)
//{
//	const std::string& orderStr = SOrder::serialize(order);
//	std::cout << "Order " << orderStr << std::endl;
//}
//
//KTradeObserverStub::KTradeObserverStub()
//{
//}
//
//KTradeObserverStub KTradeObserverStub::s_instance;

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

class KTradeManager : public ITradeManager, public ICommunicatorObserver
{
	public: 
		KTradeManager( ICommunicator* communicator );
		virtual ~KTradeManager();

	public: 
		// ITradeManager
		virtual void run();
		virtual HTrader getTrader(const account_key_t& key) const;

	public: 
		// ICommunicatorObserver
		virtual void onNewAccountDetected(const account_key_t& key);
		virtual void onSymbolNote(const account_key_t& key, const note::EKind noteKind, const std::string& symbol);

	private:
		ICommunicator& m_communicator;
		ITradeObserver* m_observer = nullptr;

		traders_t m_traders;

};

// ---------------------------------------------------------------------------

KTradeManager::KTradeManager( ICommunicator* communicator ) 
	: m_communicator( *communicator )
	//, m_observer(&KTradeObserverStub::s_instance)
{
	m_communicator.setObserver( this );
}

KTradeManager::~KTradeManager()
{
}

//void KTradeManager::setObserver(ITradeObserver* observer)
//{
//	m_observer = observer;
//}

// ---------------------------------------------------------------------------

void KTradeManager::run()
{
}

HTrader KTradeManager::getTrader(const account_key_t& key) const
{
	HTrader result;
	auto it = m_traders.find( key );
	if ( it != m_traders.end() )
	{
		result = it->second;
	}
	return result;
}

// ---------------------------------------------------------------------------

void KTradeManager::onNewAccountDetected(const account_key_t& key)
{
	assert( m_traders.count( key ) == 0 );
	HTrader trader( fx::createTrader( key ) );
	m_traders.insert( std::make_pair( key, trader ) );
	HConnection connection = m_communicator.connect( key, trader.get() );
	trader->setConnection( connection );
}

void KTradeManager::onSymbolNote(const account_key_t& key, const note::EKind noteKind, const std::string& symbol)
{
	HTrader trader = getTrader( key );
	assert( trader );

	switch ( noteKind )
	{
		case note::RegisterSymbol:
			trader->onRegisterSymbol(symbol);
			break;

		case note::UnregisterSymbol:
			trader->onUnregisterSymbol(symbol);
			break;

		default:
			assert( !"unknown note kind!" );
	}
}

} // anonymous namespace

// ---------------------------------------------------------------------------

ITradeManager* createTradeManager(ICommunicator* communicator)
{
	ITradeManager* tradeManager = new KTradeManager( communicator );
	return tradeManager;
}

} // namespace fx
