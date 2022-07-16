#include "ph.h"
#include "commandManager.h"
#include "common/command.h"
#include "common/traderCommandParser.h"
#include "common/commandQueue.h"
#include "common/consts.h"
#include "common/namedPipe.h"
#include "common/utils.h"
#include "cpp/streams.h"
#include "cpp/strUtils.h"

namespace fx
{

namespace
{

struct auto_clear_atomic
{
	auto_clear_atomic(std::atomic<bool>& atm) : m_atomic(atm)
	{
	}

	~auto_clear_atomic()
	{
		m_atomic = false;
	}

	std::atomic<bool>& m_atomic;
};

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

class KCommandManager : public ICommandManager
{
	public:
		KCommandManager(const SAccountInfo& accountInfo);
		virtual ~KCommandManager();

	public:
		virtual void run();

		virtual bool getCommand(wchar_t* wcmdName, int* argCount, MqlStr* wargs, int* ticketCount, int tickets[]);
		virtual void onCommandCompleted(const wchar_t* result);

	private:
		void cmdLoop();

	private:
		const SAccountInfo m_accountInfo;
		command_queue m_cmdQueue;
		std::atomic<bool> m_executionPending = false;

};

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

KCommandManager::KCommandManager(const SAccountInfo& accountInfo)
	: m_accountInfo(accountInfo)
{
}

KCommandManager::~KCommandManager()
{
}

// ---------------------------------------------------------------------------

void KCommandManager::run()
{
	std::thread cmdLoopThread(&KCommandManager::cmdLoop, this);
	cmdLoopThread.detach();
}

void KCommandManager::cmdLoop()
{
	int index = 0;
	while (true)
	{
		const std::string& cmdPipePath = utils::preparePath(EMedium::NamedPipe, m_accountInfo, consts::CmdPipeName, index);
		++index;
		KNamedPipeServer cmdPipe(cmdPipePath);
		if (cmdPipe.create())
		{
			std::string rawCommand;
			std::string parseError;
			while (cmdPipe.isValid())
			{
				if (cmdPipe.read(&rawCommand))
				{
					cpp::cout << "command " << rawCommand << std::endl;
					HCommand command = fx::parseTraderCommand(rawCommand, &parseError);
					if (command)
					{
						m_cmdQueue.push(command);
						cmdPipe.write(consts::CmdSendSuccess);
					}
					else
					{
						cmdPipe.write(parseError);
					}
				}
			}
		}
		else
		{
			const int SleepDuration = 1000;
			utils::sleep(SleepDuration);
		}
	}
}

bool KCommandManager::getCommand(wchar_t* wcmdName, int* argCount, MqlStr* wargs, int* ticketCount, int tickets[])
{
	bool result = false;
	bool rt = false;
	if (m_executionPending.compare_exchange_weak(rt, true))
	{
		HCommand command = m_cmdQueue.try_pop();
		if (command)
		{
			cpp::cout << "wcmdName  " << std::hex << (void*)wcmdName << std::endl;

			const std::string& cmdName = command->name();
			cpp::su::str2w(cmdName, wcmdName);

			const cpp::strings_t& cmdArgs = command->args();
			*argCount = 0;
			cpp::cout << "before store args  " << *argCount << std::endl;
			for (const std::string& arg : cmdArgs)
			{
				MqlStr& warg = wargs[(*argCount)++];
				assert(warg.m_length == consts::MaxCmdArgLen);
				cpp::cout << arg << " -- assert(warg->m_length == consts::MaxCmdArgLen); " << (warg.m_length == consts::MaxCmdArgLen)<< std::endl;
				cpp::cout << "warg.m_length " << warg.m_length << std::endl;
				cpp::cout << "warg.m_data ptr " << warg.m_data << std::endl;
				cpp::cout << "warg.m_data '" << cpp::su::w2str(warg.m_data) << "'" << std::endl;
				cpp::cout << "warg.reserved " << warg.reserved << std::endl;
				cpp::su::str2w(arg, warg.m_data);
				cpp::cout << "warg " << cpp::su::w2str(warg.m_data) << std::endl;
			}

			cpp::cout << "after store args" << std::endl;
			const tickets_t& cmdTickets = command->tickets();
			*ticketCount = cmdTickets.size();
			assert(*ticketCount <= consts::MaxCmdTicketCount);
			memcpy(tickets, cmdTickets.data(), (*ticketCount) * sizeof(ticket_t));

			cpp::cout << "getCommand " << cpp::su::w2str(wcmdName) << ' ' << *argCount << ' ' << *ticketCount << std::endl;

			result = true;
		}
		else
		{
			m_executionPending = false;
		}
	}
	return result;
}

void KCommandManager::onCommandCompleted(const wchar_t* wresult)
{
	auto_clear_atomic clearExecutionPending(m_executionPending);
	const std::string& result = cpp::su::w2str(wresult);
	cpp::cout << "KCommandManager::onCommandCompleted: " << result << std::endl;
}

} // anonymous namespace

// ---------------------------------------------------------------------------

ICommandManager::~ICommandManager()
{
}

ICommandManager* createCommandManager(const SAccountInfo& accountInfo)
{
	ICommandManager* commandManager = new KCommandManager(accountInfo);
	return commandManager;
}

} // namespace fx
