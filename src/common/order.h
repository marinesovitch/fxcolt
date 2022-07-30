// author: Darek Slusarczyk alias marines marinesovitch 2012-2013, 2022
#ifndef INC_COMMON_ORDER_H
#define INC_COMMON_ORDER_H

#include "types.h"

namespace fx
{

struct SOrder
{
	public:
		char m_symbolName[consts::MaxSymbolNameLen];

		ticket_t m_ticket;

		enum EStatus
		{
			Pending,
			Open,
			Closed,
			Unknown
		};

		EStatus m_status = Unknown;

		enum EType
		{
			// CAUTION! don't change the order! it has to map values into MetaTrader type
			// https://docs.mql4.com/constants/tradingconstants/orderproperties
			Buy = 0,
			Sell,
			BuyLimit,
			SellLimit,
			BuyStop,
			SellStop,
			None
		};

		EType m_type = None;
		SVolume m_lots = 0;
		SPrice m_openPrice;
		SPrice m_closePrice;

		SPrice m_stopLoss;
		SPrice m_takeProfit;

		SDateTime m_openTime;
		SDateTime m_expirationTime;
		SDateTime m_closeTime;

		SVolume m_commission;
		SVolume m_swap;
		SVolume m_profit;

	public:
		SOrder();
		SOrder(
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
			const price_t profit);

	public:
		static EStatus str2status(const std::string& statusStr);

		static EType str2type(const std::string& typeStr);
		static const std::string& type2str(const EType type);

		static std::string serialize(const SOrder& order);
		static SOrder deserialize(const std::string& strOrder);

};

// ---------------------------------------------------------------------------

struct SNewOrder
{
	public:
		std::string m_symbolName;

		SOrder::EType m_type = SOrder::None;
		SVolume m_lots = 0;
		SPrice m_openPrice;

		SPrice m_stopLoss;
		SPrice m_takeProfit;

		SDateTime m_expirationTime;

	public:
		SNewOrder();
		SNewOrder(
			const std::string& symbolName,
			const SOrder::EType type,
			const SVolume& lots,
			const SPrice& openPrice,
			const SPrice& stopLoss,
			const SPrice& takeProfit,
			const SDateTime& expirationTime);

};

// ---------------------------------------------------------------------------

struct SModifyOrder
{
	public:
		SPrice m_openPrice;

		SPrice m_stopLoss;
		SPrice m_takeProfit;

		SDateTime m_expirationTime;

	public:
		SModifyOrder();
		SModifyOrder(
			const SPrice& stopLoss,
			const SPrice& takeProfit,
			const SDateTime& expirationTime);
		SModifyOrder(
			const SPrice& openPrice,
			const SPrice& stopLoss,
			const SPrice& takeProfit,
			const SDateTime& expirationTime);

};

} // namespace fx

#endif
