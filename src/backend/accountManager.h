#ifndef INC_BACKEND_ACCOUNTMANAGER_H
#define INC_BACKEND_ACCOUNTMANAGER_H

#include "common/baseTypes.h"

namespace fx
{ 

struct SAccountInfo;

struct IAccountManager
{
	public:
		virtual ~IAccountManager();

	public:
		virtual account_key_t add(const SAccountInfo& accountInfo) = 0;
		virtual bool exists(const account_key_t& key) const = 0;
		virtual const SAccountInfo& get(const account_key_t& key) const = 0;
		virtual account_key_t getKey(const SAccountInfo& accountInfo) const = 0;

		virtual bool getKeys(account_keys_t* keys) const = 0;

};

} // namespace fx

#endif
