// author: Darek Slusarczyk alias marines marinesovitch 2012-2013, 2022
#include "ph.h"
#include "adapter.h"
#include "commandManager.h"
#include "common/consts.h"
#include "common/fileUtils.h"
#include "common/mailSlot.h"
#include "common/namedPipe.h"
#include "common/notes.h"
#include "common/transmission.h"
#include "common/types.h"
#include "common/utils.h"
#include "cpp/streams.h"
#include "cpp/strUtils.h"

namespace fx
{

namespace
{

class KAdapter
{
	private:
		KAdapter(const SAccountInfo& accountInfo);

	public:
		~KAdapter();

		static void init(const std::string& broker, const account_login_t& accountLogin);
		static KAdapter& get();

	public:
		void registerSymbol(const std::string& symbol);
		void unregisterSymbol(const std::string& symbol);
		void sendNote(const std::string& note, const std::string& body);
		void sendTick(const STick& tick);
		void dumpSymbol(const fx::SSymbolInfo& symbolInfo);
		void dumpOrder(const fx::SOrder& order);

		ICommandManager& cmdManager();

	private:
		static KAdapter* s_instance;

		const SAccountInfo m_accountInfo;

		cpp::stringset_t m_symbols;

		KMailSlotTransmitter m_noteTransmitter;
		KTransmitter<STick> m_tickTransmitter;
		KTransmitter<SSymbolInfo> m_symbolTransmitter;
		KTransmitter<SOrder> m_orderTransmitter;

		std::unique_ptr<ICommandManager> m_cmdManager;

};

KAdapter* KAdapter::s_instance = nullptr;

// ---------------------------------------------------------------------------

KAdapter::KAdapter(const SAccountInfo& accountInfo)
	: m_accountInfo(accountInfo)
	, m_noteTransmitter(utils::preparePath(EMedium::MailSlot, consts::NoteSlotName))
	, m_tickTransmitter(m_accountInfo)
	, m_symbolTransmitter(m_accountInfo)
	, m_orderTransmitter(m_accountInfo)
	, m_cmdManager(fx::createCommandManager(m_accountInfo))
{
	std::thread cmdThread(&ICommandManager::run, m_cmdManager.get());
	cmdThread.detach();

	m_tickTransmitter.run(consts::TickPipeName);
	m_symbolTransmitter.run(consts::SymbolPipeName);
	m_orderTransmitter.run(consts::OrderPipeName);
}

KAdapter::~KAdapter()
{
}

void KAdapter::init(const std::string& broker, const account_login_t& accountLogin)
{
	static std::mutex initMutex;
	std::lock_guard< std::mutex > guard(initMutex);

	if (s_instance == nullptr)
	{
		const SAccountInfo accountInfo(broker, accountLogin);
		s_instance = new KAdapter(accountInfo);
	}

	cpp::cout << "KAdapter::init " << broker << ' ' << accountLogin << std::endl;
}

KAdapter& KAdapter::get()
{
	assert(s_instance != nullptr);
	return *s_instance;
}

// ---------------------------------------------------------------------------

void KAdapter::registerSymbol(const std::string& symbol)
{
	m_symbols.insert(symbol);
	sendNote(fx::note::RegisterSymbolLabel, symbol);
}

void KAdapter::unregisterSymbol(const std::string& symbol)
{
	sendNote(fx::note::UnregisterSymbolLabel, symbol);
	m_symbols.erase(symbol);
}

void KAdapter::sendNote(const std::string& note, const std::string& body)
{
	cpp::cout << "KAdapter::sendNote start " << note << std::endl;

	const cpp::strings_t noteTokens = { note, m_accountInfo.m_broker, std::to_string( m_accountInfo.m_accountLogin ), body };
	const std::string& packedNote = note::packNote( noteTokens );

	m_noteTransmitter.write(packedNote);

	cpp::cout << "KAdapter::sendNote " << packedNote << std::endl;
}

void KAdapter::sendTick(const STick& tick)
{
//	cpp::cout << "KAdapter::sendTick " << tick.m_symbolName << ' ' << tick.m_bid.m_value << std::endl;
	m_tickTransmitter.write(tick);
//	cpp::cout << "KAdapter::sendTick after " << tick.m_symbolName << ' ' << tick.m_bid.m_value << std::endl;
}

void KAdapter::dumpSymbol(const fx::SSymbolInfo& symbolInfo)
{
	cpp::cout << "dumpSymbol " << symbolInfo.m_name << std::endl;
	m_symbolTransmitter.write(symbolInfo);
	cpp::cout << "dumpSymbol after " << symbolInfo.m_name << std::endl;
}

void KAdapter::dumpOrder(const fx::SOrder& order)
{
	cpp::cout << "dumpOrder " << order.m_ticket << std::endl;
	m_orderTransmitter.write(order);
	cpp::cout << "dumpOrder after " << order.m_ticket << std::endl;
}

ICommandManager& KAdapter::cmdManager()
{
	return *m_cmdManager;
}

} // anonymous namespace

} // namespace fx

// ---------------------------------------------------------------------------

ADAPTER_API void stdcall RegisterSymbol(const wchar_t* wbroker, const fx::account_login_t accountLogin, const wchar_t* wsymbol)
{
	cpp::cout << "RegisterSymbol" << std::endl;
	if ((wbroker != nullptr) && (wsymbol != nullptr))
	{
		const std::string& broker = cpp::su::w2str(wbroker);
		fx::KAdapter::init(broker, accountLogin);
		fx::KAdapter& adapter = fx::KAdapter::get();
		const std::string& symbol = cpp::su::w2str(wsymbol);
		adapter.registerSymbol(symbol);
	}
	else
	{
		cpp::cout << "RegisterSymbol symbol fail " << std::endl;
	}
}

ADAPTER_API void stdcall UnregisterSymbol(const wchar_t* wsymbol)
{
	fx::KAdapter& adapter = fx::KAdapter::get();
	const std::string& symbol = cpp::su::w2str(wsymbol);
	adapter.unregisterSymbol(symbol);
}

ADAPTER_API void stdcall DumpTick(
	const wchar_t* wsymbol,
	const fx::datetime_t time,
	const double bid,
	const double ask,
	const double last)
{
	fx::KAdapter& adapter = fx::KAdapter::get();
	const std::string& symbol = cpp::su::w2str(wsymbol);
	const fx::STick tick(symbol.c_str(), time, bid, ask, last);
	adapter.sendTick(tick);
}

ADAPTER_API int stdcall GetMaxCmdArgCount()
{
	return fx::consts::MaxCmdArgCount;
}

ADAPTER_API int stdcall GetMaxCmdStringLen()
{
	return fx::consts::MaxCmdArgLen;
}

ADAPTER_API int stdcall GetMaxCmdTicketCount()
{
	return fx::consts::MaxCmdTicketCount;
}

ADAPTER_API bool stdcall GetCommand(wchar_t* cmd, int* argCount, fx::MqlStr* args, int* ticketCount, int tickets[])
{
	fx::ICommandManager& cmdManager = fx::KAdapter::get().cmdManager();
	const bool result = cmdManager.getCommand(cmd, argCount, args, ticketCount, tickets);
	return result;
}

ADAPTER_API void stdcall OnCommandCompleted(const wchar_t* result)
{
	fx::ICommandManager& cmdManager = fx::KAdapter::get().cmdManager();
	cmdManager.onCommandCompleted(result);
}

ADAPTER_API void stdcall DumpSymbol(
	const wchar_t* wname)
{
	fx::KAdapter& adapter = fx::KAdapter::get();
	const std::string& name = cpp::su::w2str(wname);
	const fx::SSymbolInfo symbolInfo(name);
	adapter.dumpSymbol(symbolInfo);
}

ADAPTER_API void stdcall DumpOrder(
	const wchar_t* wsymbol,
	const fx::ticket_t ticket,
	const fx::SOrder::EType type,
	const fx::volume_t lots,
	const fx::price_t openPrice,
	const fx::price_t closePrice,
	const fx::price_t stopLoss,
	const fx::price_t takeProfit,
	const fx::datetime_t openTime,
	const fx::datetime_t expirationTime,
	const fx::datetime_t closeTime,
	const fx::price_t commission,
	const fx::price_t swap,
	const fx::price_t profit)
{
	fx::KAdapter& adapter = fx::KAdapter::get();
	const std::string& symbol = cpp::su::w2str(wsymbol);
	const fx::SOrder order(
		symbol,
		ticket,
		type,
		lots,
		openPrice,
		closePrice,
		stopLoss,
		takeProfit,
		openTime,
		expirationTime,
		closeTime,
		commission,
		swap,
		profit);
	adapter.dumpOrder(order);
}

ADAPTER_API void stdcall LogWrite(const wchar_t* wmsg)
{
	const std::string& msg = cpp::su::w2str(wmsg);
	cpp::cout << msg;
}

ADAPTER_API void stdcall LogWriteln(const wchar_t* wmsg)
{
	LogWrite(wmsg);
	cpp::cout << std::endl;
}
