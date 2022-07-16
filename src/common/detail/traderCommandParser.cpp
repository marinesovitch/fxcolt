#include "ph.h"
#include "traderCommandParser.h"
#include "commandParserBase.h"
#include "command.h"
#include "order.h"

namespace fx
{

namespace
{

class KTraderCommandParser : public KCommandParserBase
{
	public:
		KTraderCommandParser(const std::string& commandStr);
		KTraderCommandParser(std::istringstream& cmdLine);

	public:
		HCommand run(KCommand::EOperation operation = KCommand::Unknown);

	private:
		KCommand::EOperation parseOperation();
		HCommand parseCommand(const KCommand::EOperation operation);

		HCommand parseCommandListSymbols();
		HCommand parseCommandGet();
		HCommand parseCommandOpen();
		HCommand parseCommandClose();
		HCommand parseCommandCloseAll();
		HCommand parseCommandModify();
		HCommand parseCommandSetStopLoss();
		HCommand parseCommandSetTakeProfit();

		HCommand parseCommandGetSymbols();
		HCommand parseCommandShowTicks();
		HCommand parseCommandHideTicks();
};

// ---------------------------------------------------------------------------

KTraderCommandParser::KTraderCommandParser(const std::string& commandStr)
	: KCommandParserBase(commandStr)
{
}

KTraderCommandParser::KTraderCommandParser(std::istringstream& cmdLine)
	: KCommandParserBase(cmdLine)
{
}

HCommand KTraderCommandParser::run(KCommand::EOperation operation)
{
	if (operation == KCommand::Unknown)
	{
		operation = parseOperation();
	}
	HCommand result = parseCommand(operation);
	return result;
}

// ---------------------------------------------------------------------------

KCommand::EOperation KTraderCommandParser::parseOperation()
{
	std::string cmdName;
	m_cmdLine >> cmdName;

	const KCommand::EOperation operation = KCommand::operationFromStr(cmdName);
	if (operation == KCommand::Unknown)
	{
		throw std::invalid_argument("unknown command " + cmdName);
	}
	return operation;
}

HCommand KTraderCommandParser::parseCommand(const KCommand::EOperation operation)
{
	typedef HCommand (KTraderCommandParser::*parse_command_t)();
	static const std::map<KCommand::EOperation, parse_command_t> s_cmdParsers = {
		{KCommand::ListSymbols, &KTraderCommandParser::parseCommandListSymbols},
		{KCommand::Get, &KTraderCommandParser::parseCommandGet},
		{KCommand::Open, &KTraderCommandParser::parseCommandOpen},
		{KCommand::Close, &KTraderCommandParser::parseCommandClose},
		{KCommand::CloseAll, &KTraderCommandParser::parseCommandCloseAll},
		{KCommand::Modify, &KTraderCommandParser::parseCommandModify},
		{KCommand::SetStopLoss, &KTraderCommandParser::parseCommandSetStopLoss},
		{KCommand::SetTakeProfit, &KTraderCommandParser::parseCommandSetTakeProfit},

		{KCommand::GetSymbols, &KTraderCommandParser::parseCommandGetSymbols},
		{KCommand::ShowTicks, &KTraderCommandParser::parseCommandShowTicks},
		{KCommand::HideTicks, &KTraderCommandParser::parseCommandHideTicks},
	};

	auto cit = s_cmdParsers.find(operation);
	assert(cit != s_cmdParsers.cend());
	parse_command_t parser = cit->second;
	HCommand result = (this->*parser)();
	return result;
}

HCommand KTraderCommandParser::parseCommandListSymbols()
{
	HCommand result(new KCmdListSymbols());
	return result;
}

HCommand KTraderCommandParser::parseCommandGet()
{
	parseTickets(false);
	HCommand result(new KCmdGet(m_tickets));
	return result;
}

HCommand KTraderCommandParser::parseCommandOpen()
{
	const SNewOrder& newOrder = parseNewOrder();
	HCommand result(new KCmdOpen(newOrder));
	return result;
}

HCommand KTraderCommandParser::parseCommandClose()
{
	parseTickets();
	HCommand result(new KCmdClose(m_tickets));
	return result;
}

HCommand KTraderCommandParser::parseCommandCloseAll()
{
	HCommand result(new KCmdCloseAll());
	return result;
}

HCommand KTraderCommandParser::parseCommandModify()
{
	const SModifyOrder& modifyOrder = parseModifyOrder();
	parseTickets();
	HCommand result(new KCmdModify(modifyOrder, m_tickets));
	return result;
}

HCommand KTraderCommandParser::parseCommandSetStopLoss()
{
	const SPrice& stopLoss = parsePrice();
	parseTickets();
	HCommand result(new KCmdSetStopLoss(stopLoss, m_tickets));
	return result;
}

HCommand KTraderCommandParser::parseCommandSetTakeProfit()
{
	const SPrice& takeProfit = parsePrice();
	parseTickets();
	HCommand result(new KCmdSetTakeProfit(takeProfit, m_tickets));
	return result;
}

HCommand KTraderCommandParser::parseCommandGetSymbols()
{
	HCommand result(new KCmdGetSymbols());
	return result;
}

HCommand KTraderCommandParser::parseCommandShowTicks()
{
	HCommand result(new KCmdShowTicks());
	return result;
}

HCommand KTraderCommandParser::parseCommandHideTicks()
{
	HCommand result(new KCmdHideTicks());
	return result;
}

} // anonymous namespace

// ---------------------------------------------------------------------------

HCommand parseTraderCommand(const std::string& commandStr, std::string* error)
{
	HCommand result;
	try
	{
		KTraderCommandParser parser(commandStr);
		result = parser.run();
	}
	catch (std::exception& e)
	{
		*error = e.what();
	}
	return result;
}

HCommand parseTraderCommand(const KCommand::EOperation operation, std::istringstream& cmdLine)
{
	KTraderCommandParser parser(cmdLine);
	HCommand result = parser.run(operation);
	return result;
}

} // namespace fx
