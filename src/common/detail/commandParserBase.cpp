// author: Darek Slusarczyk alias marines marinesovitch 2012-2013, 2022
#include "ph.h"
#include "commandParserBase.h"
#include "command.h"
#include "order.h"

namespace fx
{

namespace
{

template<typename T, typename Converter>
T parseNumValue(const std::string& valueStr, const char* label)
{
	try {
		std::size_t idx = 0;
		const T result = Converter()(valueStr, &idx); // it may throw an exception
		if (idx == valueStr.length()) {
			return result;
		}
		throw std::invalid_argument("contains incorrect characters");
	} catch (std::exception& e) {
		std::ostringstream os;
		os << "incorrect " << label << ": '" << valueStr << "': " << e.what();
		throw std::invalid_argument(os.str());
	}
}

// ---------------------

struct stoi
{
	int operator()(const std::string& valueStr, size_t* idx) const
	{
		return std::stoi(valueStr, idx);
	}
};

int parseInt(const std::string& valueStr, const char* label)
{
	return parseNumValue<int, stoi>(valueStr, label);
}

// ---------------------

struct stod
{
	double operator()(const std::string& valueStr, size_t* idx) const
	{
		return std::stod(valueStr, idx);
	}
};

double parseDouble(const std::string& valueStr, const char* label)
{
	return parseNumValue<double, stod>(valueStr, label);
}

} // anonymous namespace

KCommandParserBase::KCommandParserBase(const std::string& cmdLine)
	: m_cmdLine(cmdLine)
{
}

KCommandParserBase::KCommandParserBase(std::istringstream& cmdLine)
	: m_cmdLine(std::move(cmdLine))
{
}

KCommandParserBase::~KCommandParserBase()
{
}

// ---------------------------------------------------------------------------

SNewOrder KCommandParserBase::parseNewOrder()
{
	const std::string& symbolName = getNextToken();

	const SOrder::EType type = parseOrderType();
	const SVolume& lots = parseVolume();
	const SPrice& openPrice = parsePrice();

	const SPrice& stopLoss = parsePrice(false);
	const SPrice& takeProfit = parsePrice(false);

	const SDateTime& expirationTime = parseDateTime();

	const SNewOrder newOrder(
		symbolName,
		type,
		lots,
		openPrice,
		stopLoss,
		takeProfit,
		expirationTime);

	return newOrder;
}

SModifyOrder KCommandParserBase::parseModifyOrder()
{
	const SPrice& openPrice = parsePrice();
	const SPrice& stopLoss = parsePrice(false);
	const SPrice& takeProfit = parsePrice(false);

	const SDateTime& expirationTime = parseDateTime();

	const SModifyOrder modifyOrder(openPrice, stopLoss, takeProfit, expirationTime);
	return modifyOrder;
}

void KCommandParserBase::parseTickets(const bool obligatory)
{
	std::string ticketStr;
	while (getNextToken(&ticketStr))
	{
		const ticket_t ticket( parseInt(ticketStr, "ticket") );
		m_tickets.push_back(ticket);
	}

	if (obligatory && m_tickets.empty())
	{
		throw std::invalid_argument("not enough arguments, order tickets are obligatory");
	}
}

SOrder::EType KCommandParserBase::parseOrderType()
{
	const std::string& typeStr = getNextToken();
	const SOrder::EType result = SOrder::str2type(typeStr);
	if (result == SOrder::None)
	{
		throw std::invalid_argument("incorrect order type: " + typeStr);
	}
	return result;
}

SVolume KCommandParserBase::parseVolume()
{
	const std::string& volumeStr = getNextToken();
	const volume_t result = parseDouble(volumeStr, "volume");
	return result;
}

SPrice KCommandParserBase::parsePrice(const bool obligatory)
{
	const std::string& priceStr = getNextToken(obligatory, "0");
	const price_t result = parseDouble(priceStr, "price");
	return result;
}

SDateTime KCommandParserBase::parseDateTime(const bool obligatory)
{
	const std::string& dateTimeStr = getNextToken(obligatory, "0");
	const datetime_t result = parseInt(dateTimeStr, "datetime");
	return result;
}

std::string KCommandParserBase::getNextToken(const bool obligatory, const std::string& defaultValue)
{
	if (!(m_cmdLine >> m_token))
	{
		if (obligatory)
		{
			throw std::invalid_argument("too few arguments");
		}
		else
		{
			m_token = defaultValue;
		}
	}
	return m_token;
}

bool KCommandParserBase::getNextToken(std::string* token)
{
	const bool result = (m_cmdLine >> *token) ? true : false;
	return result;
}

void KCommandParserBase::parseError( const std::string& reason ) const
{
	std::ostringstream os;
	os << m_token << ' ' << reason;
	const std::string& error = os.str();
	throw std::invalid_argument( error );
}

} // namespace fx
