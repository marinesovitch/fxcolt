#ifndef INC_COMMON_TYPES_H
#define INC_COMMON_TYPES_H

#include "baseTypes.h"
#include "consts.h"

namespace fx
{

enum class EMedium
{
	MailSlot,
	NamedPipe
};

// ---------------------------------------------------------------------------

struct SDateTime
{
	SDateTime(const datetime_t datetime = 0);
	datetime_t m_value;
};

// ---------------------------------------------------------------------------

struct SPrice
{
	SPrice(const price_t value = 0);
	explicit operator bool() const;
	bool isNull() const;
	std::string toString() const;
	price_t m_value;
};

// ---------------------------------------------------------------------------

struct SVolume
{
	SVolume(const volume_t value = 0);
	volume_t m_value;
};

// ---------------------------------------------------------------------------

struct SAccountInfo
{
	public:
		SAccountInfo()=default;
		SAccountInfo(
			const std::string& broker,
			const account_login_t& accountLogin);

		bool operator<(const SAccountInfo& rhs) const;

	public:
		std::string m_broker;
		account_login_t m_accountLogin;

};

// ---------------------------------------------------------------------------

struct STick
{
	STick();
	STick(
		const char* symbol,
		const SDateTime& time, 
		const SPrice& bid, 
		const SPrice& ask, 
		const SPrice& last);

	char m_symbolName[consts::MaxSymbolNameLen];
	SDateTime m_time;
	SPrice m_ask;
	SPrice m_bid;
	SPrice m_last;

};

} // namespace fx

#endif
