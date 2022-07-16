#include "ph.h"
#include "order.h"
#include "cpp/converter.h"

namespace fx
{

namespace
{

static cpp::enum_map<SOrder::EStatus> s_status_conv_map;
static cpp::enum_conv<SOrder::EStatus> s_status_conv(
	s_status_conv_map
		(SOrder::Unknown, "Unknown")
		(SOrder::Pending, "Pending")
		(SOrder::Open, "Open")
		(SOrder::Closed, "Closed"));

static cpp::enum_map<SOrder::EType> s_type_conv_map;
static cpp::enum_conv<SOrder::EType> s_type_conv(
	s_type_conv_map
		(SOrder::Buy, "Buy")
		(SOrder::Sell, "Sell")
		(SOrder::BuyLimit, "BuyLimit")
		(SOrder::SellLimit, "SellLimit")
		(SOrder::BuyStop, "BuyStop")
		(SOrder::SellStop, "SellStop")
		(SOrder::None, "None"));

static std::map<std::string, SOrder::EType> s_alias2type =
{
	{"b", SOrder::Buy},
	{"s", SOrder::Sell},
	{"bl", SOrder::BuyLimit},
	{"sl", SOrder::SellLimit},
	{"bs", SOrder::BuyStop},
	{"ss", SOrder::SellStop}
};


SOrder::EStatus prepareStatus(
	const SOrder::EType type,
	const datetime_t closeTime)
{
	SOrder::EStatus result = SOrder::Unknown;
	if (closeTime != 0)
	{
		result = SOrder::Closed;
	}
	else
	{
		switch (type)
		{
			case SOrder::Buy:
			case SOrder::Sell:
				result = SOrder::Open;
				break;

			case SOrder::BuyLimit:
			case SOrder::SellLimit:
			case SOrder::BuyStop:
			case SOrder::SellStop:
				result = SOrder::Pending;
				break;

			default:
				assert(!"unexpected order type!");
		}
	}
	return result;
}

} // anonymous namespace

SOrder::SOrder()
{
}

SOrder::SOrder(
	const std::string& symbol,
	const ticket_t ticket,
	const EType type,
	const volume_t lots,
	const price_t openPrice,
	const price_t closePrice,
	const price_t stopLoss,
	const price_t takeProfit,
	const datetime_t openTime,
	const datetime_t expirationTime,
	const datetime_t closeTime,
	const price_t commission,
	const price_t swap,
	const price_t profit)
	: m_ticket(ticket)
	, m_status(prepareStatus(type, closeTime))
	, m_type(type)
	, m_lots(lots)
	, m_openPrice(openPrice)
	, m_closePrice(closePrice)
	, m_stopLoss(stopLoss)
	, m_takeProfit(takeProfit)
	, m_openTime(openTime)
	, m_expirationTime(expirationTime)
	, m_closeTime(closeTime)
	, m_commission(commission)
	, m_swap(swap)
	, m_profit(profit)
{
	strcpy(m_symbolName, symbol.c_str());
}

// ---------------------------------------------------------------------------

SOrder::EStatus SOrder::str2status(const std::string& statusStr)
{
	const SOrder::EStatus result = s_status_conv.from_str(statusStr);
	return result;
}

SOrder::EType SOrder::str2type(const std::string& typeStr)
{
	SOrder::EType result;
	if (!s_type_conv.from_str(typeStr, &result))
	{
		auto it = s_alias2type.find(typeStr);
		if (it != s_alias2type.end())
		{
			result = it->second;
		}
		else
		{
			result = SOrder::None;
		}

	}
	return result;
}

const std::string& SOrder::type2str(const SOrder::EType type)
{
	return s_type_conv.to_str(type);
}

std::string SOrder::serialize(const SOrder& order)
{
	const char* symbol = order.m_symbolName;
	const ticket_t ticket = order.m_ticket;
	const SOrder::EStatus status = order.m_status;
	const SOrder::EType type = order.m_type;
	const volume_t lots = order.m_lots.m_value;
	const price_t openPrice = order.m_openPrice.m_value;
	const price_t closePrice = order.m_closePrice.m_value;
	const price_t stopLoss = order.m_stopLoss.m_value;
	const price_t takeProfit = order.m_takeProfit.m_value;
	const datetime_t openTime = order.m_openTime.m_value;
	const datetime_t expirationTime = order.m_expirationTime.m_value;
	const datetime_t closeTime = order.m_closeTime.m_value;
	const price_t commission = order.m_commission.m_value;
	const price_t swap = order.m_swap.m_value;
	const price_t profit = order.m_profit.m_value;


	std::ostringstream os;
	os << symbol << ' '
		<< ticket << ' '
		<< s_status_conv.to_str(status) << ' '
		<< s_type_conv.to_str(type) << ' '
		<< lots << ' '
		<< openPrice << ' '
		<< closePrice << ' '
		<< stopLoss << ' '
		<< takeProfit << ' '
		<< openTime << ' '
		<< expirationTime << ' '
		<< closeTime << ' '
		<< commission << ' '
		<< swap << ' '
		<< profit;
	const std::string& result = os.str();
	return result;
}

SOrder SOrder::deserialize(const std::string& strOrder)
{
	std::istringstream is(strOrder);
	SOrder order;
	std::string rawStatus;
	std::string rawType;
	is >> order.m_symbolName
		>> order.m_ticket
		>> rawStatus
		>> rawType
		>> order.m_lots.m_value
		>> order.m_openPrice.m_value
		>> order.m_closePrice.m_value
		>> order.m_stopLoss.m_value
		>> order.m_takeProfit.m_value
		>> order.m_openTime.m_value
		>> order.m_expirationTime.m_value
		>> order.m_closeTime.m_value
		>> order.m_commission.m_value
		>> order.m_swap.m_value
		>> order.m_profit.m_value;

	order.m_status = s_status_conv.from_str(rawStatus);
	order.m_type = s_type_conv.from_str(rawType);

	return order;
}

// ---------------------------------------------------------------------------

SNewOrder::SNewOrder()
{
}

SNewOrder::SNewOrder(
	const std::string& symbolName,
	const SOrder::EType type,
	const SVolume& lots,
	const SPrice& openPrice,
	const SPrice& stopLoss,
	const SPrice& takeProfit,
	const SDateTime& expirationTime)
	: m_symbolName(symbolName)
	, m_type(type)
	, m_lots(lots)
	, m_openPrice(openPrice)
	, m_stopLoss(stopLoss)
	, m_takeProfit(takeProfit)
	, m_expirationTime(expirationTime)
{
}

// ---------------------------------------------------------------------------

SModifyOrder::SModifyOrder()
{
}

SModifyOrder::SModifyOrder(
	const SPrice& stopLoss,
	const SPrice& takeProfit,
	const SDateTime& expirationTime)
	: m_stopLoss(stopLoss)
	, m_takeProfit(takeProfit)
	, m_expirationTime(expirationTime)
{
}

SModifyOrder::SModifyOrder(
	const SPrice& openPrice,
	const SPrice& stopLoss,
	const SPrice& takeProfit,
	const SDateTime& expirationTime)
	: m_openPrice(openPrice)
	, m_stopLoss(stopLoss)
	, m_takeProfit(takeProfit)
	, m_expirationTime(expirationTime)
{
}

} // namespace fx
