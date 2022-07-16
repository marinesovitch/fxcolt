#include "ph.h"
#include "communicator.h"
#include "accountManager.h"
#include "connection.h"
#include "traderSink.h"
#include "common/command.h"
#include "common/commandQueue.h"
#include "common/consts.h"
#include "common/fileUtils.h"
#include "common/mailSlot.h"
#include "common/namedPipe.h"
#include "common/notes.h"
#include "common/order.h"
#include "common/symbolInfo.h"
#include "common/types.h"
#include "common/utils.h"
#include "cpp/strUtils.h"

namespace fx
{

namespace
{

struct SChannelInfo
{
	enum EKind
	{
		Tick = 0x1,
		Symbol = 0x2,
		Order = 0x4,
		Cmd = 0x8,
		All = Tick | Symbol | Order | Cmd
	};

	SAccountInfo m_accountInfo;

	EKind m_kind;

	int m_index;

	std::string m_pipePath;

	SChannelInfo(
		const SAccountInfo& accountInfo,
		const EKind kind,
		const int index,
		const std::string pipePath);

	bool operator<( const SChannelInfo& rhs ) const;

};

typedef std::vector< SChannelInfo > channels_t;

// ---------------------------------------------------------------------------

SChannelInfo::SChannelInfo(
	const SAccountInfo& accountInfo,
	const EKind kind,
	const int index,
	const std::string pipePath)
	: m_accountInfo(accountInfo)
	, m_kind(kind)
	, m_index(index)
	, m_pipePath(pipePath)
{
}

bool SChannelInfo::operator<( const SChannelInfo& rhs ) const
{
	const bool result = ( m_pipePath < rhs.m_pipePath );
	return result;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

class KChannelDetector
{
	public:
		KChannelDetector(channels_t* channels);

		bool run();

	private:
		void storeChannel(const std::string& pipePath);
		bool isChannel(const cpp::strings_t& elems) const;

		channels_t& m_channels;

		static std::map< std::string, SChannelInfo::EKind > s_label2kind;

};

std::map< std::string, SChannelInfo::EKind > KChannelDetector::s_label2kind;

// ---------------------------------------------------------------------------

KChannelDetector::KChannelDetector( channels_t* channels )
	: m_channels( *channels )
{
	if (s_label2kind.empty())
	{
		// assign in constructor to avoid random initialization order of globals
		s_label2kind =
		{
			{ consts::TickPipeName, SChannelInfo::Tick },
			{ consts::SymbolPipeName, SChannelInfo::Symbol },
			{ consts::OrderPipeName, SChannelInfo::Order },
			{ consts::CmdPipeName, SChannelInfo::Cmd }
		};
	}
}

bool KChannelDetector::run()
{
	cpp::strings_t pipes;
	if (utils::enumNamedPipes(consts::AppTag, &pipes))
	{
		for(auto pipe : pipes)
		{
			storeChannel(pipe);
		}
	}

	const bool result = !pipes.empty();
	return result;
}

void KChannelDetector::storeChannel(const std::string& pipePath)
{
	cpp::strings_t elems;
	cpp::su::split(pipePath, '\\', &elems);
	if (isChannel(elems))
	{
		// accountInfo
		int elemIndex = 5;
		const std::string& broker = elems[elemIndex++];

		const std::string& strLogin = elems[elemIndex++];
		const account_login_t login( std::stoll(strLogin) );

		const SAccountInfo accountInfo(broker, login);

		// channelInfo
		const std::string& strKind = elems[elemIndex++];
		auto it = s_label2kind.find(strKind);
		assert(it != s_label2kind.end());
		const SChannelInfo::EKind kind = it->second;

		const std::string& strIndex = elems[elemIndex++];
		const int index = std::stoi(strIndex);

		const SChannelInfo channelInfo(accountInfo, kind, index, pipePath);
		m_channels.push_back( channelInfo );
	}
}

bool KChannelDetector::isChannel(const cpp::strings_t& elems) const
{
	const std::size_t ExpectedNumOfTokens = 9;
	int index = 0;
	const bool result
		= (elems.size() == ExpectedNumOfTokens)
		&& (elems[index++] == "")
		&& (elems[index++] == "")
		&& (elems[index++] == ".")
		&& (elems[index++] == consts::PipePrefix)
		&& (elems[index++] == consts::AppTag)
		&& !(elems[index++].empty()) // account broker
		&& cpp::su::isInteger(elems[index++]) // account login
		&& (s_label2kind.count(elems[index++]) != 0)
		&& cpp::su::isInteger(elems[index]); // index of channel
	return result;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

class KChannelsManager
{
	public:
		KChannelsManager( IAccountManager* accountManager );

	public:
		bool detect( account_keys_t* newlyFoundAccounts );
		bool get(
			const account_key_t& key,
			const SChannelInfo::EKind kind,
			channels_t* channels ) const;

	private:
		void store( const channels_t& channels, account_keys_t* newlyFoundAccounts );

	private:
		IAccountManager& m_accountManager;

		typedef std::set< SChannelInfo > channelset_t;
		typedef std::map< account_key_t, channelset_t > account2channels_t;
		account2channels_t m_account2channels;

};

// ---------------------------------------------------------------------------

KChannelsManager::KChannelsManager( IAccountManager* accountManager )
	: m_accountManager( *accountManager )
{
}

bool KChannelsManager::detect( account_keys_t* newlyFoundAccounts )
{
	m_account2channels.clear();

	channels_t channels;
	KChannelDetector channelDetector( &channels );
	if (channelDetector.run())
	{
		store( channels, newlyFoundAccounts );
	}

	const bool result = !newlyFoundAccounts->empty();
	return result;
}

bool KChannelsManager::get(
	const account_key_t& key,
	const SChannelInfo::EKind kind,
	channels_t* channels ) const
{
	auto it = m_account2channels.find( key );
	if ( it != m_account2channels.end() )
	{
		const channelset_t& accountChannels = it->second;

		auto begin = std::find_if(
			accountChannels.begin(),
			accountChannels.end(),
			[kind] (const SChannelInfo& ci) { return ci.m_kind == kind; } );

		auto end = std::find_if(
			begin,
			accountChannels.end(),
			[kind] (const SChannelInfo& ci) { return ci.m_kind != kind; } );

		// reverse copy because there is higher probability that channel with bigger index is ready to connect
		std::reverse_copy( begin, end, std::back_inserter( *channels ) );
	}

	const bool result = !channels->empty();
	return result;
}

void KChannelsManager::store( const channels_t& channels, account_keys_t* newlyFoundAccounts )
{
	for ( const SChannelInfo& ci : channels )
	{
		const SAccountInfo& accountInfo = ci.m_accountInfo;

		account_key_t accountKey = m_accountManager.getKey( accountInfo );
		if ( accountKey.isNull() )
		{
			accountKey = m_accountManager.add( accountInfo );
			newlyFoundAccounts->push_back( accountKey );
		}

		channelset_t& accountChannels = m_account2channels[ accountKey ];
		accountChannels.insert( ci );
	}
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

struct SSymbolNote
{
	note::EKind m_note;
	SAccountInfo m_accountInfo;
	std::string m_label;
};

struct INoteObserver
{
	virtual void onSymbolNote(const SSymbolNote& symbolNote) = 0;
};

// ---------------------------------------------------------------------------

class KNoteProcessor
{
	public:
		KNoteProcessor(INoteObserver* observer);

		void run();

	private:
		void processingLoop();

		bool parseSymbolNote(
			const std::string& packedNote,
			SSymbolNote* symbolNote) const;

		void processSymbolNote(const SSymbolNote& symbolNote);

	private:
		INoteObserver& m_observer;

};

// ---------------------------------------------------------------------------

KNoteProcessor::KNoteProcessor(INoteObserver* observer)
	: m_observer( *observer )
{
}

void KNoteProcessor::run()
{
	std::thread noteSlotThread(&KNoteProcessor::processingLoop, this);
	noteSlotThread.detach();
}

void KNoteProcessor::processingLoop()
{
	const std::string& noteSlotPath = utils::preparePath(EMedium::MailSlot, consts::NoteSlotName);
	KMailSlotReceiver mailSlot(noteSlotPath);
	std::string packedNote;
	SSymbolNote symbolNote;
	while (mailSlot.read(&packedNote))
	{
		std::cout << packedNote << std::endl;
		if (parseSymbolNote(packedNote, &symbolNote))
		{
			processSymbolNote(symbolNote);
		}
	}
}

bool KNoteProcessor::parseSymbolNote(
	const std::string& packedNote,
	SSymbolNote* symbolNote) const
{
	bool result = false;

	cpp::strings_t noteTokens;
	if ( note::unpackNote( packedNote, 4, &noteTokens ) )
	{
		static const std::map<std::string, note::EKind> s_label2note =
		{
			{note::RegisterSymbolLabel, note::RegisterSymbol},
			{note::UnregisterSymbolLabel, note::UnregisterSymbol},
		};

		const std::string& noteStr = noteTokens[ 0 ];
		const std::string& accountLoginStr = noteTokens[ 2 ];

		auto it = s_label2note.find(noteStr);
		if ( (it != s_label2note.end()) && cpp::su::isInteger( accountLoginStr ))
		{
			symbolNote->m_note = it->second;
			const std::string& rawBrokerName = noteTokens[ 1 ];
			symbolNote->m_accountInfo.m_broker = utils::normalizeBrokerName( rawBrokerName );
			symbolNote->m_accountInfo.m_accountLogin = account_login_t( std::stoll( accountLoginStr ) );
			symbolNote->m_label = noteTokens[ 3 ];

			result = true;
		}
	}

	return result;
}

void KNoteProcessor::processSymbolNote(const SSymbolNote& symbolNote)
{
	switch (symbolNote.m_note)
	{
		case note::RegisterSymbol:
		case note::UnregisterSymbol:
			m_observer.onSymbolNote(symbolNote);
			break;

		default:
			assert(!"unknown note!");
	}
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

class KConnection : public IConnection
{
	public:
		KConnection(ITraderSink* sink);
		virtual ~KConnection();

	public:
		// IConnection
		virtual bool isConnected() const;
		virtual void sendCommand( HCommand command );

	public:
		bool isChannelConnected(const SChannelInfo::EKind channelKind) const;
		void runChannelLoop(const SChannelInfo::EKind channelKind, KNamedPipeClient* rawTickChannel);

	private:
		void tickLoop(KNamedPipeClient* rawChannelPipe);
		void symbolLoop(KNamedPipeClient* rawChannelPipe);
		void orderLoop(KNamedPipeClient* rawChannelPipe);
		void cmdLoop(KNamedPipeClient* rawChannelPipe);

		void setChannelConnected(const SChannelInfo::EKind channelKind);
		void setChannelDisconnected(const SChannelInfo::EKind channelKind);

	private:
		ITraderSink* m_sink;

		// logical OR of SChannelInfo::EKind flags
		std::atomic<int> m_disconnectedChannels = SChannelInfo::All;

		command_queue m_cmdQueue;

};

typedef std::shared_ptr< KConnection > HKConnection;

// ---------------------------------------------------------------------------

KConnection::KConnection(ITraderSink* sink) : m_sink(sink)
{
}

KConnection::~KConnection()
{

}

bool KConnection::isConnected() const
{
	const bool result = (m_disconnectedChannels == 0);
	return result;
}

void KConnection::sendCommand(HCommand command)
{
	m_cmdQueue.push(command);
}

// ---------------------------------------------------------------------------

bool KConnection::isChannelConnected(const SChannelInfo::EKind channelKind) const
{
	const bool result = (m_disconnectedChannels & channelKind) == 0;
	return result;
}

void KConnection::runChannelLoop(const SChannelInfo::EKind channelKind, KNamedPipeClient* channelPipe)
{
	typedef void (KConnection::*TChannelLoopRoutine)(KNamedPipeClient* /*channelPipe*/);
	static std::map< SChannelInfo::EKind, TChannelLoopRoutine > s_kind2routine =
	{
		{ SChannelInfo::Tick, &KConnection::tickLoop },
		{ SChannelInfo::Symbol, &KConnection::symbolLoop },
		{ SChannelInfo::Order, &KConnection::orderLoop },
		{ SChannelInfo::Cmd, &KConnection::cmdLoop }
	};

	auto it = s_kind2routine.find(channelKind);
	assert( it != s_kind2routine.end() );
	auto channelLoopRoutine = it->second;

	std::thread cmdLoopThread(channelLoopRoutine, this, channelPipe);
	cmdLoopThread.detach();
}

void KConnection::tickLoop(KNamedPipeClient* rawChannelPipe)
{
	std::unique_ptr< KNamedPipeClient > channelPipe(rawChannelPipe);
	setChannelConnected(SChannelInfo::Tick);

	STick tick;
	while (channelPipe->isValid())
	{
		if (channelPipe->read(&tick))
		{
			m_sink->onTick(tick);
		}
	}

	setChannelDisconnected(SChannelInfo::Tick);
}

void KConnection::symbolLoop(KNamedPipeClient* rawChannelPipe)
{
	std::unique_ptr< KNamedPipeClient > channelPipe(rawChannelPipe);
	setChannelConnected(SChannelInfo::Symbol);

	SSymbolInfo symbol;
	while (channelPipe->isValid())
	{
		if (channelPipe->read(&symbol))
		{
			m_sink->onSymbol(symbol);
		}
	}

	setChannelDisconnected(SChannelInfo::Symbol);
}

void KConnection::orderLoop(KNamedPipeClient* rawChannelPipe)
{
	std::unique_ptr< KNamedPipeClient > channelPipe(rawChannelPipe);
	setChannelConnected(SChannelInfo::Order);

	SOrder order;
	while (channelPipe->isValid())
	{
		if (channelPipe->read(&order))
		{
			m_sink->onOrder(order);
		}
	}

	setChannelDisconnected(SChannelInfo::Order);
}

void KConnection::cmdLoop(KNamedPipeClient* rawChannelPipe)
{
	std::unique_ptr< KNamedPipeClient > channelPipe(rawChannelPipe);
	setChannelConnected(SChannelInfo::Cmd);

	HCommand command;
	std::string output;
	while (channelPipe->isValid())
	{
		command = m_cmdQueue.pop();
		const std::string& cmdStr = command->toString();
		if (channelPipe->write(cmdStr))
		{
			if (channelPipe->read(&output))
			{
				m_sink->onCmdResult(output);
			}
			else
			{
			}
		}
	}

	setChannelDisconnected(SChannelInfo::Cmd);
}

void KConnection::setChannelConnected(const SChannelInfo::EKind channelKind)
{
	m_disconnectedChannels &= ~channelKind;
}

void KConnection::setChannelDisconnected(const SChannelInfo::EKind channelKind)
{
	m_disconnectedChannels |= channelKind;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

class KConnector
{
	public:
		KConnector(
			const account_key_t& key,
			const KChannelsManager& channels,
			HKConnection connection);

	public:
		void run();

	private:
		void connectChannel(const SChannelInfo::EKind kind);
		KNamedPipeClient* findPipe(const SChannelInfo::EKind kind);

	private:
		const account_key_t m_key;
		const KChannelsManager& m_channels;
		HKConnection m_connection;

};

// ---------------------------------------------------------------------------

KConnector::KConnector(
	const account_key_t& key,
	const KChannelsManager& channels,
	HKConnection connection)
	: m_key( key )
	, m_channels( channels )
	, m_connection( connection )
{
}

void KConnector::run()
{
	connectChannel(SChannelInfo::Tick);
	connectChannel(SChannelInfo::Symbol);
	connectChannel(SChannelInfo::Order);
	connectChannel(SChannelInfo::Cmd);
}

void KConnector::connectChannel(const SChannelInfo::EKind kind)
{
	if (!m_connection->isChannelConnected( kind ))
	{
		KNamedPipeClient* channelPipe = findPipe(kind);
		if (channelPipe != nullptr)
		{
			m_connection->runChannelLoop(kind, channelPipe);
		}
	}
}

KNamedPipeClient* KConnector::findPipe( const SChannelInfo::EKind kind )
{
	KNamedPipeClient* result = nullptr;

	channels_t channels;
	if ( m_channels.get( m_key, kind, &channels ) )
	{
		for ( const SChannelInfo& channelInfo : channels )
		{
			assert( channelInfo.m_kind == kind );
			const std::string& pipePath = channelInfo.m_pipePath;

			std::unique_ptr<KNamedPipeClient> channelPipe(new KNamedPipeClient(pipePath));
			if (channelPipe->open())
			{
				result = channelPipe.release();
				break;
			}
		}
	}

	return result;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

typedef std::map< account_key_t, HKConnection > connections_t;

class KCommunicator : public ICommunicator, public INoteObserver
{
	public:
		KCommunicator(IAccountManager* accountManager);
		virtual ~KCommunicator();

	public:
		// ICommunicator
		virtual void setObserver(ICommunicatorObserver* observer);
		virtual void run();
		virtual HConnection connect(const account_key_t& key, ITraderSink* sink);

	public:
		// INoteObserver
		virtual void onSymbolNote(const SSymbolNote& symbolNote);

	private:
		bool isConnected( const account_key_t& accountKey ) const;
		bool detectChannels();
		void connectChannels(const account_key_t& key, HKConnection connection);

	private:
		KNoteProcessor m_noteProcessor;
		IAccountManager& m_accountManager;
		ICommunicatorObserver* m_observer = nullptr;
		KChannelsManager m_channels;
		connections_t m_connections;

};

// ---------------------------------------------------------------------------

KCommunicator::KCommunicator(IAccountManager* accountManager)
	: m_noteProcessor( this )
	, m_accountManager( *accountManager )
	, m_channels( accountManager )
{
}

KCommunicator::~KCommunicator()
{
}

// ---------------------------------------------------------------------------

void KCommunicator::setObserver(ICommunicatorObserver* observer)
{
	assert( m_observer == nullptr );
	m_observer = observer;
}

void KCommunicator::run()
{
	detectChannels();
	m_noteProcessor.run();
}

HConnection KCommunicator::connect(const account_key_t& key, ITraderSink* sink)
{
	assert(m_connections.count(key) == 0);
	HKConnection connection( new KConnection(sink) );
	m_connections.insert( std::make_pair( key, connection ) );
	connectChannels( key, connection );
	return connection;
}

// ---------------------------------------------------------------------------

void KCommunicator::onSymbolNote(const SSymbolNote& symbolNote)
{
	const note::EKind noteKind = symbolNote.m_note;
	assert( (noteKind == note::RegisterSymbol) || (noteKind == note::UnregisterSymbol) );
	const SAccountInfo& accountInfo = symbolNote.m_accountInfo;
	account_key_t accountKey = m_accountManager.getKey(accountInfo);
	if ( !isConnected( accountKey ) )
	{
		if (detectChannels())
		{
			accountKey = m_accountManager.getKey(accountInfo);
		}
	}

	if ( accountKey.isValid() )
	{
		const std::string& symbolLabel = symbolNote.m_label;
		m_observer->onSymbolNote(accountKey, noteKind, symbolLabel);
	}
}

// ---------------------------------------------------------------------------

bool KCommunicator::isConnected( const account_key_t& accountKey ) const
{
	bool result = false;
	auto it = m_connections.find( accountKey );
	if ( it != m_connections.end() )
	{
		HKConnection connection = it->second;
		result = connection->isConnected();
	}
	return result;
}

bool KCommunicator::detectChannels()
{
	bool result = false;

	account_keys_t newlyFoundAccounts;
	if ( m_channels.detect( &newlyFoundAccounts ) )
	{
		result = true;
		for ( auto accountKey : newlyFoundAccounts )
		{
			m_observer->onNewAccountDetected( accountKey );
		}
	}
	return result;
}

void KCommunicator::connectChannels( const account_key_t& key, HKConnection connection )
{
	KConnector connector( key, m_channels, connection );
	connector.run();
}

} // anonymous namespace

// ---------------------------------------------------------------------------

ICommunicator* createCommunicator(IAccountManager* accountManager)
{
	ICommunicator* communicator = new KCommunicator(accountManager);
	return communicator;
}

} // namespace fx
