#include "ph.h"
#include "executorImpl.h"
#include "executor.h"
#include "accountManager.h"
#include "tradeManager.h"
#include "trader.h"
#include "tradingStrategyFactory.h"
#include "common/command.h"
#include "common/commandQueue.h"
#include "common/commandParserBase.h"
#include "common/traderCommandParser.h"
#include "common/namedPipe.h"
#include "common/utils.h"
#include "cpp/strUtils.h"

namespace fx
{

namespace
{

typedef std::map<std::string, ITradingStrategyFactory*> strategy2factory_t;

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

struct SListAccountsCommand;
struct SSelectCommand;
struct SHelpCommand;

struct IExecutorCommandVisitor
{
	virtual void visitListAccountsCommand( const SListAccountsCommand& cmd ) = 0;
	virtual void visitSelectCommand( const SSelectCommand& cmd ) = 0;
	virtual void visitHelpCommand( const SHelpCommand& cmd ) = 0;
};

// ---------------------------------------------------------------------------

struct SExecutorCommand
{
	virtual ~SExecutorCommand()
	{
	}

	virtual void accept( IExecutorCommandVisitor* visitor ) = 0;
};

struct SListAccountsCommand : public SExecutorCommand
{
	virtual void accept( IExecutorCommandVisitor* visitor )
	{
		visitor->visitListAccountsCommand( *this );
	}

};

struct SSelectCommand : public SExecutorCommand
{
	SSelectCommand( const account_key_t& key )
		: m_key( key )
	{
	}

	virtual void accept( IExecutorCommandVisitor* visitor )
	{
		visitor->visitSelectCommand( *this );
	}

	account_key_t m_key;

};

struct SHelpCommand : public SExecutorCommand
{
	SHelpCommand()
	{
	}

	virtual void accept( IExecutorCommandVisitor* visitor )
	{
		visitor->visitHelpCommand( *this );
	}

};

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

class KExecutorCommandParser : public KCommandParserBase
{
	public:
		KExecutorCommandParser(std::istringstream& cmdLine);

		SExecutorCommand* run(const std::string& cmdName);

	public:
		static bool isExecutorCommand( const std::string& cmd );
		static void getExecutorCommandsHelp(std::ostream& os);

	private:
		typedef void (KExecutorCommandParser::*TCommandParseRoutine)();
		TCommandParseRoutine getCmdParseRoutine( const std::string& rawCmdName ) const;

		void parseListAccountsCommand();
		void parseSelectCommand();
		void parseHelpCommand();

	private:
		static const std::map< std::string, TCommandParseRoutine > s_cmd2parser;
		static const std::map<std::string, std::string> s_alias2cmd;

		SExecutorCommand* m_result;

};

// ---------------------------------------------------------------------------

KExecutorCommandParser::KExecutorCommandParser(std::istringstream& cmdLine)
	: KCommandParserBase( cmdLine )
{
}

SExecutorCommand* KExecutorCommandParser::run(const std::string& cmdName)
{
	TCommandParseRoutine cmdParseRoutine = getCmdParseRoutine( cmdName );
	if ( cmdParseRoutine != nullptr )
	{
		(this->*cmdParseRoutine)();
	}
	else
	{
		parseError( "unknown command" );
	}

	return m_result;
}

bool KExecutorCommandParser::isExecutorCommand( const std::string& cmd )
{
	const bool result
		= ( s_cmd2parser.count( cmd ) != 0 )
		|| ( s_alias2cmd.count( cmd ) != 0 );
	return result;
}

void KExecutorCommandParser::getExecutorCommandsHelp(std::ostream& os)
{
	for (const auto& [alias, cmd] : s_alias2cmd)
	{
		os << cmd << " (" << alias << ")\n";
	}
}

KExecutorCommandParser::TCommandParseRoutine KExecutorCommandParser::getCmdParseRoutine( const std::string& rawCmdName ) const
{
	KExecutorCommandParser::TCommandParseRoutine result = nullptr;

	std::string cmdName;
	auto it = s_alias2cmd.find( rawCmdName );
	if ( it != s_alias2cmd.end() )
	{
		cmdName = it->second;
	}
	else
	{
		cmdName = rawCmdName;
	}

	auto it2 = s_cmd2parser.find( cmdName );
	if ( it2 != s_cmd2parser.end() )
	{
		result = it2->second;
	}

	return result;
}

void KExecutorCommandParser::parseListAccountsCommand()
{
	m_result = new SListAccountsCommand();
}

void KExecutorCommandParser::parseSelectCommand()
{
	const std::string& accountKeyStr = getNextToken();
	const int rawAccountKey = std::stoi( accountKeyStr );
	const account_key_t key( rawAccountKey );
	m_result = new SSelectCommand( key );
}

void KExecutorCommandParser::parseHelpCommand()
{
	m_result = new SHelpCommand();
}

// ---------------------------------------------------------------------------

const std::string CmdNameListAccounts = "accounts";
const std::string CmdNameSelect = "select";
const std::string CmdNameHelp = "help";

const std::map< std::string, KExecutorCommandParser::TCommandParseRoutine > KExecutorCommandParser::s_cmd2parser =
{
	{CmdNameListAccounts, &KExecutorCommandParser::parseListAccountsCommand},
	{CmdNameSelect, &KExecutorCommandParser::parseSelectCommand},
	{CmdNameHelp, &KExecutorCommandParser::parseHelpCommand},
};

const std::map<std::string, std::string> KExecutorCommandParser::s_alias2cmd =
{
	{"la", CmdNameListAccounts},
	{"sel", CmdNameSelect},
	{"h", CmdNameHelp},
};

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

struct SGenericTraderCommand;
struct SGenericStrategyCommand;
struct SGenericExecutorCommand;

struct IGenericCommandVisitor
{
	virtual void visitTraderCommand( SGenericTraderCommand* cmd ) = 0;
	virtual void visitStrategyCommand( SGenericStrategyCommand* cmd ) = 0;
	virtual void visitExecutorCommand( SGenericExecutorCommand* cmd ) = 0;
};

// ---------------------------------------------------------------------------

struct SGenericCommand
{
	SGenericCommand( const account_key_t& key )
		: m_key( key )
	{
	}

	virtual ~SGenericCommand()
	{
	}

	virtual void accept( IGenericCommandVisitor* visitor ) = 0;

	const account_key_t m_key;
};

struct SGenericTraderCommand : public SGenericCommand
{
	SGenericTraderCommand(
		const account_key_t& key,
		HCommand cmd )
		: SGenericCommand( key )
		, m_command( cmd )
	{
	}

	virtual void accept( IGenericCommandVisitor* visitor )
	{
		visitor->visitTraderCommand( this );
	}

	HCommand m_command;

};

struct SGenericStrategyCommand : public SGenericCommand
{
	SGenericStrategyCommand(
		const account_key_t& key,
		const std::string& strategy,
		const std::string& symbol,
		std::istringstream& cmdLine )
		: SGenericCommand( key )
		, m_strategy( strategy )
		, m_symbol( symbol )
		, m_cmdLine( std::move( cmdLine ) )
	{
	}

	virtual void accept( IGenericCommandVisitor* visitor )
	{
		visitor->visitStrategyCommand( this );
	}

	std::string m_strategy;
	std::string m_symbol;
	std::istringstream m_cmdLine;

};

struct SGenericExecutorCommand : public SGenericCommand
{
	SGenericExecutorCommand(
		const account_key_t& key,
		SExecutorCommand* cmd )
		: SGenericCommand( key )
		, m_command( cmd )
	{
	}

	virtual void accept( IGenericCommandVisitor* visitor )
	{
		visitor->visitExecutorCommand( this );
	}

	std::unique_ptr< SExecutorCommand > m_command;

};

// ---------------------------------------------------------------------------

struct ICommandParserCallback
{
	virtual account_key_t getDefaultAccount() const = 0;
	virtual bool isStrategyCommand( const std::string& cmd ) const = 0;
};

class KGeneralCommandParser : public KCommandParserBase
{
	public:
		KGeneralCommandParser(
			const ICommandParserCallback& callback,
			const IAccountManager& accountManager,
			const std::string& cmdLine);

		SGenericCommand* run();

	private:
		void parseTradingCommand();

		void parseAccountKey();
		void prepareAccountKey();
		void verifyAccountKey() const;

		void parseCommand();

		bool isExecutorCommand() const;
		bool isTraderCommand() const;
		bool isStrategyCommand() const;

		void parseExecutorCommand();
		void parseTraderCommand();
		void parseStrategyCommand();

	private:
		const ICommandParserCallback& m_callback;
		const IAccountManager& m_accountManager;
		account_key_t m_key;
		SGenericCommand* m_result = nullptr;

};

// ---------------------------------------------------------------------------

KGeneralCommandParser::KGeneralCommandParser(
	const ICommandParserCallback& callback,
	const IAccountManager& accountManager,
	const std::string& cmdLine)
	: KCommandParserBase( cmdLine )
	, m_callback( callback )
	, m_accountManager( accountManager )
{
}

SGenericCommand* KGeneralCommandParser::run()
{
	getNextToken();
	if (isExecutorCommand())
	{
		parseExecutorCommand();
	}
	else
	{
		parseTradingCommand();
	}
	return m_result;
}

void KGeneralCommandParser::parseTradingCommand()
{
	parseAccountKey();
	parseCommand();
}

void KGeneralCommandParser::parseAccountKey()
{
	prepareAccountKey();
	verifyAccountKey();
}

void KGeneralCommandParser::prepareAccountKey()
{
	if (cpp::su::isInteger( m_token ))
	{
		const int rawKey = std::stoi(m_token);
		m_key = account_key_t(rawKey);
		getNextToken();
	}
	else
	{
		m_key = m_callback.getDefaultAccount();
	}
}

void KGeneralCommandParser::verifyAccountKey() const
{
	if (!m_accountManager.exists(m_key))
	{
		if (m_key.isNull())
		{
			parseError( "default account not selected, please run 'select' command" );
		}
		else
		{
			const std::string reason = "incorrect account key " + std::to_string( m_key );
			parseError( reason );
		}
	}
}

void KGeneralCommandParser::parseCommand()
{
	if (isTraderCommand())
	{
		parseTraderCommand();
	}
	else if (isStrategyCommand())
	{
		parseStrategyCommand();
	}
	else
	{
		parseError( "unknown command" );
	}
}

bool KGeneralCommandParser::isExecutorCommand() const
{
	const bool result = KExecutorCommandParser::isExecutorCommand(m_token);
	return result;
}

bool KGeneralCommandParser::isTraderCommand() const
{
	const KCommand::EOperation operation = KCommand::operationFromStr(m_token);
	const bool result = (operation != KCommand::Unknown);
	return result;
}

bool KGeneralCommandParser::isStrategyCommand() const
{
	const bool result = m_callback.isStrategyCommand(m_token);
	return result;
}

void KGeneralCommandParser::parseExecutorCommand()
{
	KExecutorCommandParser executorCmdParser( m_cmdLine );
	const std::string& cmdName = m_token;
	SExecutorCommand* executorCmd = executorCmdParser.run(cmdName);
	m_result = new SGenericExecutorCommand( m_key, executorCmd );
}

void KGeneralCommandParser::parseTraderCommand()
{
	const KCommand::EOperation operation = KCommand::operationFromStr(m_token);
	assert(operation != KCommand::Unknown);
	HCommand cmd = fx::parseTraderCommand( operation, m_cmdLine );
	m_result = new SGenericTraderCommand( m_key, cmd );
}

void KGeneralCommandParser::parseStrategyCommand()
{
	const std::string strategy = m_token;
	const std::string& symbol = getNextToken();
	m_result = new SGenericStrategyCommand( m_key, strategy, symbol, m_cmdLine );
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

class KExecutor : public IExecutor, ICommandParserCallback, IGenericCommandVisitor, IExecutorCommandVisitor
{
	public:
		KExecutor(
			std::ostream* cout,
			std::ostream* cerr,
			IAccountManager* accountManager,
			ITradeManager* tradeManager );
		virtual ~KExecutor();

	public:
		// IExecutor
		virtual bool executeCommand(const account_key_t& key, HCommand command, std::string* error);
		virtual bool executeCommand(const std::string& cmdLine, std::string* error);

		virtual void registerStrategy( ITradingStrategyFactory* tradingStrategyFactory );

	public:
		// ICommandParserCallback
		virtual account_key_t getDefaultAccount() const;
		virtual bool isStrategyCommand( const std::string& cmd ) const;

	public:
		// IGenericCommandVisitor
		virtual void visitTraderCommand( SGenericTraderCommand* traderCmd );
		virtual void visitStrategyCommand( SGenericStrategyCommand* strategyCmd );
		virtual void visitExecutorCommand( SGenericExecutorCommand* executorCmd );

	public:
		// IExecutorCommandVisitor
		virtual void visitListAccountsCommand( const SListAccountsCommand& cmd );
		virtual void visitSelectCommand( const SSelectCommand& cmd );
		virtual void visitHelpCommand( const SHelpCommand& cmd );

	private:
		HTrader getTrader( const account_key_t& key );
		HTradingStrategy createStrategy( const std::string& strategyName );
		void ensureStrategy(
			HTrader trader,
			const std::string& symbol,
			const std::string& strategy );
		void printAccount( const account_key_t& key );

	private:
		std::ostream& m_cout;
		std::ostream& m_cerr;

		IAccountManager& m_accountManager;
		ITradeManager& m_tradeManager;

		strategy2factory_t m_strategyFactories;

		account_key_t m_selectedAccount;

};

// ---------------------------------------------------------------------------

KExecutor::KExecutor(
	std::ostream* cout,
	std::ostream* cerr,
	IAccountManager* accountManager,
	ITradeManager* tradeManager )
	: m_cout( *cout )
	, m_cerr( *cerr )
	, m_accountManager( *accountManager )
	, m_tradeManager( *tradeManager )
{
}

KExecutor::~KExecutor()
{
	for ( auto it : m_strategyFactories )
	{
		ITradingStrategyFactory* factory = it.second;
		delete factory;
	}
}

// ---------------------------------------------------------------------------
// IExecutor

bool KExecutor::executeCommand(const account_key_t& key, HCommand command, std::string* error)
{
	bool result = false;
	try
	{
		HTrader trader = getTrader( key );
		trader->executeCommand( command );
		result = true;
	}
	catch ( std::exception& e )
	{
		*error = e.what();
	}
	return result;
}

bool KExecutor::executeCommand(const std::string& cmdLine, std::string* error)
{
	bool result = false;
	try
	{
		KGeneralCommandParser cmdParser(*this, m_accountManager, cmdLine);
		std::unique_ptr< SGenericCommand > command( cmdParser.run() );
		command->accept( this );
		result = true;
	}
	catch ( std::exception& e )
	{
		*error = e.what();
	}
	return result;
}

void KExecutor::registerStrategy( ITradingStrategyFactory* tradingStrategyFactory )
{
	cpp::strings_t strategies;
	tradingStrategyFactory->getNames( &strategies );
	for ( auto name : strategies )
	{
		assert( m_strategyFactories.count( name ) == 0 );
		m_strategyFactories.insert( std::make_pair( name, tradingStrategyFactory ) );
	}
}

// ---------------------------------------------------------------------------
// ICommandParserCallback

account_key_t KExecutor::getDefaultAccount() const
{
	return m_selectedAccount;
}

bool KExecutor::isStrategyCommand( const std::string& cmd ) const
{
	const bool result = ( m_strategyFactories.count(cmd) != 0 );
	return result;
}

// ---------------------------------------------------------------------------
// IGenericCommandVisitor

void KExecutor::visitTraderCommand( SGenericTraderCommand* traderCmd )
{
	const account_key_t& key = traderCmd->m_key;
	HTrader trader = getTrader( key );
	HCommand command = traderCmd->m_command;
	trader->executeCommand( command );
}

void KExecutor::visitStrategyCommand( SGenericStrategyCommand* strategyCmd )
{
	const account_key_t& key = strategyCmd->m_key;
	HTrader trader = getTrader( key );

	const std::string& strategy = strategyCmd->m_strategy;
	const std::string& symbol = strategyCmd->m_symbol;
	ensureStrategy( trader, symbol, strategy );

	std::istringstream& cmdLine = strategyCmd->m_cmdLine;
	trader->executeStrategyCommand( symbol, cmdLine );
}

void KExecutor::visitExecutorCommand( SGenericExecutorCommand* executorCmd )
{
	SExecutorCommand* command = executorCmd->m_command.get();
	command->accept( this );
}

// ---------------------------------------------------------------------------
// IExecutorCommandVisitor

void KExecutor::visitListAccountsCommand( const SListAccountsCommand& cmd )
{
	account_keys_t accountKeys;
	if (m_accountManager.getKeys( &accountKeys ))
	{
		for ( auto key : accountKeys )
		{
			printAccount(key);
		}
	}
	else
	{
		m_cout << "no account connected" << std::endl;
	}
}

void KExecutor::visitSelectCommand( const SSelectCommand& cmd )
{
	const account_key_t& key = cmd.m_key;
	if ( m_accountManager.exists( key ) )
	{
		m_selectedAccount = key;
		printAccount(key);
	}
	else
	{
		const std::string reason = "incorrect account key " + std::to_string( key );
		throw std::invalid_argument( reason );
	}
}

void KExecutor::visitHelpCommand( const SHelpCommand& cmd )
{
	m_cout << "command (alias)\n";
	m_cout << "---------------\n";
	KExecutorCommandParser::getExecutorCommandsHelp(m_cout);
	getTraderCommandsHelp(m_cout);
	m_cout << consts::CmdExit << std::endl;
}

// ---------------------------------------------------------------------------

HTrader KExecutor::getTrader( const account_key_t& key )
{
	assert( m_accountManager.exists( key ) );
	HTrader trader = m_tradeManager.getTrader( key );
	assert( trader );
	return trader;
}

HTradingStrategy KExecutor::createStrategy( const std::string& strategyName )
{
	auto it = m_strategyFactories.find( strategyName );
	assert( it != m_strategyFactories.end() );
	ITradingStrategyFactory* factory = it->second;
	HTradingStrategy result = factory->create( strategyName );
	return result;
}

void KExecutor::ensureStrategy(
	HTrader trader,
	const std::string& symbol,
	const std::string& strategyName )
{
	const std::string& currentStrategyName = trader->getStrategy( symbol );
	if ( currentStrategyName != strategyName )
	{
		if ( !currentStrategyName.empty() )
		{
			trader->removeStrategy( symbol );
		}

		HTradingStrategy strategy = createStrategy( strategyName );
		trader->setStrategy( symbol, strategy );
	}
}

void KExecutor::printAccount( const account_key_t& key )
{
	const SAccountInfo& accountInfo = m_accountManager.get( key );
	m_cout << key << ' ' << accountInfo.m_broker << ' ' << accountInfo.m_accountLogin << std::endl;
}

} // anonymous namespace

// ---------------------------------------------------------------------------

IExecutor* createExecutor(
	std::ostream* cout,
	std::ostream* cerr,
	IAccountManager* accountManager,
	ITradeManager* tradeManager )
{
	IExecutor* executor = new KExecutor( cout, cerr, accountManager, tradeManager );
	return executor;
}

} // namespace fx
