#include "ph.h"
#include "command.h"
#include "consts.h"
#include "order.h"
#include "cpp/converter.h"

namespace fx
{

namespace
{

const std::string CmdNameGetSymbols = "get_symbols";
const std::string CmdNameShowTicks= "show_ticks";
const std::string CmdNameHideTicks= "hide_ticks";

const std::string CmdNameListSymbols= "list_symbols";
const std::string CmdNameGet = "get";
const std::string CmdNameOpen = "open";
const std::string CmdNameClose = "close";
const std::string CmdNameCloseAll = "close_all";
const std::string CmdNameModify = "modify";
const std::string CmdNameSetStopLoss = "set_stop_loss";
const std::string CmdNameSetTakeProfit = "set_take_profit";

static cpp::enum_map<KCommand::EOperation> s_operation_conv_map;
static const cpp::enum_conv<KCommand::EOperation> s_operation_conv(
	s_operation_conv_map
		(KCommand::GetSymbols, CmdNameGetSymbols)
		(KCommand::ShowTicks, CmdNameShowTicks)
		(KCommand::HideTicks, CmdNameHideTicks)

		(KCommand::ListSymbols, CmdNameListSymbols)
		(KCommand::Get, CmdNameGet)
		(KCommand::Open, CmdNameOpen)
		(KCommand::Close, CmdNameClose)
		(KCommand::CloseAll, CmdNameCloseAll)
		(KCommand::Modify, CmdNameModify)
		(KCommand::SetStopLoss, CmdNameSetStopLoss)
		(KCommand::SetTakeProfit, CmdNameSetTakeProfit)
		);

static const std::map<std::string, KCommand::EOperation> s_alias2operation =
{
	{"gs", KCommand::GetSymbols},
	{"st", KCommand::ShowTicks},
	{"ht", KCommand::HideTicks},

	{"ls", KCommand::ListSymbols},
	{"g", KCommand::Get},
	{"o", KCommand::Open},
	{"c", KCommand::Close},
	{"ca", KCommand::CloseAll},
	{"m", KCommand::Modify},
	{"sl", KCommand::SetStopLoss},
	{"tp", KCommand::SetTakeProfit}
};

// ---------------------------------------------------------------------------

void addPrice2args(const SPrice& price, cpp::strings_t* args)
{
	const std::string& priceStr = price.toString();
	args->push_back(priceStr);
}

template<typename TValue>
void addValue2args(const TValue& value, cpp::strings_t* args)
{
	const std::string& valueStr = std::to_string(value);
	args->push_back(valueStr);
}

template<typename TItem>
void addItem2args(const TItem& item, cpp::strings_t* args)
{
	addValue2args(item.m_value, args);
}

cpp::strings_t newOrder2args(const SNewOrder& newOrder)
{
	cpp::strings_t result;
	const int argsCount = 7;
	result.reserve(argsCount);

	const std::string& symbolName = newOrder.m_symbolName;
	result.push_back(symbolName);

	const SOrder::EType type = newOrder.m_type;
	const std::string& typeStr = SOrder::type2str(type);
	result.push_back(typeStr);

	const SVolume& lots = newOrder.m_lots;
	addItem2args(lots, &result);

	const SPrice& openPrice = newOrder.m_openPrice;
	addPrice2args(openPrice, &result);

	const SPrice& stopLoss = newOrder.m_stopLoss;
	addPrice2args(stopLoss, &result);

	const SPrice& takeProfit = newOrder.m_takeProfit;
	addPrice2args(takeProfit, &result);

	const SDateTime& expirationTime = newOrder.m_expirationTime;
	addItem2args(expirationTime, &result);

	assert(result.size() == argsCount);

	return result;
}

cpp::strings_t modifyOrder2args(const SModifyOrder& modifyOrder)
{
	cpp::strings_t result;
	const int argsCount = 4;
	result.reserve(argsCount);

	const SPrice& openPrice = modifyOrder.m_openPrice;
	addPrice2args(openPrice, &result);

	const SPrice& stopLoss = modifyOrder.m_stopLoss;
	addPrice2args(stopLoss, &result);

	const SPrice& takeProfit = modifyOrder.m_takeProfit;
	addPrice2args(takeProfit, &result);

	const SDateTime& expirationTime = modifyOrder.m_expirationTime;
	addItem2args(expirationTime, &result);

	assert(result.size() == argsCount);

	return result;
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

KCommand::KCommand(const EOperation operation)
	: m_operation(operation)
{
}

KCommand::KCommand(
	const EOperation operation,
	const tickets_t& tickets)
	: m_operation(operation)
	, m_tickets(tickets)
{
}

KCommand::KCommand(
	const EOperation operation,
	const cpp::strings_t& args)
	: m_operation(operation)
	, m_args(args)
{
}

KCommand::KCommand(
	const EOperation operation,
	const std::string& arg,
	const tickets_t& tickets)
	: m_operation(operation)
	, m_args(1, arg)
	, m_tickets(tickets)
{
}

KCommand::KCommand(
	const EOperation operation,
	const cpp::strings_t& args,
	const tickets_t& tickets)
	: m_operation(operation)
	, m_args(args)
	, m_tickets(tickets)
{
}

KCommand::~KCommand()
{
}

// ---------------------------------------------------------------------------

std::string KCommand::toString() const
{
	std::ostringstream os;

	const std::string& cmdName = name();
	os << cmdName;

	for (const auto& arg : m_args)
	{
		os << ' ' << arg;
	}

	for (const auto& ticket : m_tickets)
	{
		os << ' ' << ticket;
	}

	const std::string& result = os.str();
	return result;
}

const std::string& KCommand::name() const
{
	const std::string& result = s_operation_conv.to_str(m_operation);
	return result;
}

const cpp::strings_t& KCommand::args() const
{
	return m_args;
}

const tickets_t& KCommand::tickets() const
{
	return m_tickets;
}

bool KCommand::isLocal() const
{
	return false;
}

void KCommand::accept( ICommandVisitor* visitor )
{
	visitor->visitDefault(this);
}

// ---------------------------------------------------------------------------

KCommand::EOperation KCommand::operationFromStr(const std::string& operationStr)
{
	KCommand::EOperation result;
	if (!s_operation_conv.from_str(operationStr, &result))
	{
		auto it = s_alias2operation.find(operationStr);
		if (it != s_alias2operation.end())
		{
			result = it->second;
		}
		else
		{
			result = KCommand::Unknown;
		}
	}
	return result;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

KCmdListSymbols::KCmdListSymbols()
	: KCommand(ListSymbols)
{
}

KCmdGet::KCmdGet(const tickets_t& tickets)
	: KCommand(Get, tickets)
{
}

KCmdOpen::KCmdOpen(const SNewOrder& newOrder)
	: KCommand(Open, newOrder2args(newOrder))
{
}

KCmdClose::KCmdClose(const tickets_t& tickets)
	: KCommand(Close, tickets)
{
}

KCmdCloseAll::KCmdCloseAll()
	: KCommand(CloseAll)
{
}

KCmdModify::KCmdModify(const SModifyOrder& modifyOrder, const tickets_t& tickets)
	: KCommand(Modify, modifyOrder2args(modifyOrder), tickets)
{
}

KCmdSetStopLoss::KCmdSetStopLoss(const SPrice& stopLoss, const tickets_t& tickets)
	: KCommand(SetStopLoss, stopLoss.toString(), tickets)
{
}

KCmdSetTakeProfit::KCmdSetTakeProfit(const SPrice& takeProfit, const tickets_t& tickets)
	: KCommand(SetTakeProfit, takeProfit.toString(), tickets)
{
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

KLocalCommand::KLocalCommand(const EOperation operation)
	: KCommand(operation)
{
}

bool KLocalCommand::isLocal() const
{
	return true;
}

// ---------------------------------------------------------------------------

KCmdGetSymbols::KCmdGetSymbols()
	: KLocalCommand(GetSymbols)
{
}

void KCmdGetSymbols::accept( ICommandVisitor* visitor )
{
	visitor->visitGetSymbols(this);
}

// ---------------------------------------------------------------------------

KCmdShowTicks::KCmdShowTicks()
	: KLocalCommand(ShowTicks)
{
}

void KCmdShowTicks::accept( ICommandVisitor* visitor )
{
	visitor->visitShowTicks(this);
}

// ---------------------------------------------------------------------------

KCmdHideTicks::KCmdHideTicks()
	: KLocalCommand(HideTicks)
{
}

void KCmdHideTicks::accept( ICommandVisitor* visitor )
{
	visitor->visitHideTicks(this);
}

// ---------------------------------------------------------------------------

ICommandVisitor::~ICommandVisitor()
{
}

// ---------------------------------------------------------------------------

void getTraderCommandsHelp(std::ostream& os)
{
	for (const auto& [alias, cmd] : s_alias2operation)
	{
		const auto& cmdStr = s_operation_conv.to_str(cmd);
		os << cmdStr << " (" << alias << ")\n";
	}
}

} // namespace fx
