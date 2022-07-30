// author: Darek Slusarczyk alias marines marinesovitch 2012-2013, 2022
#include "ph.h"
#include "accountManagerImpl.h"
#include "accountManager.h"
#include "common/types.h"
#include "common/utils.h"
#include <boost/bimap.hpp>

namespace fx
{

namespace
{

typedef boost::bimap<account_key_t, SAccountInfo> accounts_t;
typedef accounts_t::value_type account_record_t;

// ---------------------------------------------------------------------------

class KAccountManager : public IAccountManager
{
	public:
		KAccountManager();
		virtual ~KAccountManager();

	public:
		virtual account_key_t add(const SAccountInfo& accountInfo);
		virtual bool exists(const account_key_t& key) const;
		virtual const SAccountInfo& get(const account_key_t& key) const;
		virtual account_key_t getKey(const SAccountInfo& accountInfo) const;

		virtual bool getKeys(account_keys_t* keys) const;

	private:
		account_key_t m_firstFreeHandle;
		accounts_t m_accounts;

};

// ---------------------------------------------------------------------------

KAccountManager::KAccountManager()
	: m_firstFreeHandle( 0 )
{
}

KAccountManager::~KAccountManager()
{
}

account_key_t KAccountManager::add(const SAccountInfo& accountInfo)
{
	assert( getKey(accountInfo).isNull() );
	assert( utils::isNormalizedBrokerName( accountInfo.m_broker ) );
	const account_key_t result = m_firstFreeHandle++;
	m_accounts.insert( account_record_t( result, accountInfo ));
	return result;
}

bool KAccountManager::exists(const account_key_t& key) const
{
	auto& key2info = m_accounts.left;
	auto it = key2info.find(key);
	const bool result = (it != key2info.end());
	return result;
}

const SAccountInfo& KAccountManager::get(const account_key_t& key) const
{
	auto& key2info = m_accounts.left;
	auto it = key2info.find(key);
	assert(it != key2info.end());
	const SAccountInfo& result = it->second;
	return result;
}

account_key_t KAccountManager::getKey(const SAccountInfo& accountInfo) const
{
	assert( utils::isNormalizedBrokerName( accountInfo.m_broker ) );
	account_key_t result;
	auto& into2key = m_accounts.right;
	auto it = into2key.find(accountInfo);
	if (it != into2key.end())
	{
		result = it->second;
	}
	return result;
}

bool KAccountManager::getKeys(account_keys_t* keys) const
{
	for ( auto key2info : m_accounts )
	{
		const account_key_t& key = key2info.left;
		keys->push_back( key );
	}

	const bool result = !keys->empty();
	return result;
}

} // anonymous namespace

// ---------------------------------------------------------------------------

IAccountManager* createAccountManager()
{
	IAccountManager* accountManager = new KAccountManager();
	return accountManager;

}

} // namespace fx

