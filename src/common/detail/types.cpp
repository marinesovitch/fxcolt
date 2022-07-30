// author: Darek Slusarczyk alias marines marinesovitch 2012-2013, 2022
#include "ph.h"
#include "types.h"

namespace fx
{

SDateTime::SDateTime(const datetime_t datetime) : m_value(datetime)
{
}

// ---------------------------------------------------------------------------

SPrice::SPrice(const price_t value) : m_value(value)
{
}

SPrice::operator bool() const
{
	const bool result = isNull();
	return result;
}

bool SPrice::isNull() const
{
	const price_t NullPrice = 1e-6;
	const bool result = std::abs(m_value) < NullPrice;
	return result;
}

std::string SPrice::toString() const
{
	const std::string& result = isNull() ? "0" : std::to_string(m_value);
	return result;
}

// ---------------------------------------------------------------------------

SVolume::SVolume(const volume_t value)
	: m_value(value)
{
}

// ---------------------------------------------------------------------------

SAccountInfo::SAccountInfo(
	const std::string& broker,
	const account_login_t& accountLogin)
	: m_broker(broker)
	, m_accountLogin(accountLogin)
{
}

bool SAccountInfo::operator<(const SAccountInfo& rhs) const
{
	const bool result 
		= (m_accountLogin < rhs.m_accountLogin)
		|| ((m_accountLogin == rhs.m_accountLogin) && (m_broker < rhs.m_broker));
	return result;
}

// ---------------------------------------------------------------------------

STick::STick()
{
	m_symbolName[0] = 0;
}

STick::STick(
	const char* symbol,
	const SDateTime& time,
	const SPrice& bid,
	const SPrice& ask,
	const SPrice& last)
	: m_time(time)
	, m_ask(ask)
	, m_bid(bid)
	, m_last(last)
{
	strcpy(m_symbolName, symbol);
}

} // namespace fx
