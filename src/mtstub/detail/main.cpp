#include "ph.h"
#include "adapter/adapter.h"
#include "cpp/strUtils.h"
#include <random>

namespace
{

const wchar_t* Symbol = L"EURPLN";

void generateTicks()
{
	constexpr double MinExchangeRate = 4.23;
	constexpr double MaxExchangeRate = 4.25;

    std::random_device rd;
    std::default_random_engine eng(rd());
    std::uniform_real_distribution<double> distr(MinExchangeRate, MaxExchangeRate);

	while (true)
	{
		const long time = static_cast<long>(std::time(nullptr));
		const double bid = distr(eng);
		const double ask = bid + distr(eng) / 1000.0;
		const double last = (bid + ask + distr(eng)) / 3.0;
		DumpTick(Symbol, time, bid, ask, last);
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

bool executeCommand(const wchar_t* wcmd, const int argCount, fx::MqlStr args[], const int ticketCount, int tickets[])
{
	const std::string& cmd = cpp::su::w2str(wcmd);
	std::cout << cmd;

	for (int i = 0; i < argCount; ++i)
	{
		const wchar_t* arg = args[i].m_data;
		std::cout << ' ' << cpp::su::w2str(arg);
	}

	if (0 < ticketCount)
	{
		std::cout << " --";

		for (int i = 0; i < ticketCount; ++i)
		{
			const int ticket = tickets[i];
			std::cout << ' ' << ticket;
		}
	}

	std::cout << std::endl;

	OnCommandCompleted(L"error - not implemented");

	return true;
}

void cmdLoop()
{
	const int MaxCmdArgCount = GetMaxCmdArgCount();
	typedef wchar_t* wchars_t;
	fx::MqlStr* cmdArgs = new fx::MqlStr[MaxCmdArgCount];

	int cmdArgCount = 0;

	int MaxCmdStringLen = GetMaxCmdStringLen();
	for (int i = 0; i < MaxCmdArgCount; ++i)
	{
		cmdArgs[i].m_length = MaxCmdStringLen;
		cmdArgs[i].m_data = new wchar_t[MaxCmdStringLen];
		cmdArgs[i].reserved = 0;
	}

	int cmdTicketCount = 0;

	int MaxCmdTicketCount = GetMaxCmdTicketCount();
	int* cmdTickets = new int[MaxCmdTicketCount];

	std::wstring CmdResult;

	wchar_t* cmd = new wchar_t[MaxCmdStringLen];

	bool run = true;
	while (run)
	{
		while(GetCommand(cmd, &cmdArgCount, cmdArgs, &cmdTicketCount, cmdTickets))
		{
			run = executeCommand(cmd, cmdArgCount, cmdArgs, cmdTicketCount, cmdTickets);
		}
		const std::chrono::milliseconds dura(1000);
		std::this_thread::sleep_for(dura);
	}

	delete[] cmd;
	delete[] cmdTickets;
	for (int i = 0; i < MaxCmdArgCount; ++i)
	{
		delete[] cmdArgs[i].m_data;
	}
	delete[] cmdArgs;
}

}

int main()
{
	//std::thread noteSlotThread(&cmdLoop);
	//noteSlotThread.detach();

	const wchar_t* broker = L"FakeBroker";
	const fx::account_login_t accountLogin(12345678);
	RegisterSymbol(broker, accountLogin, Symbol);

	std::thread ticksThread(generateTicks);
	ticksThread.detach();

	cmdLoop();

	UnregisterSymbol(Symbol);
}
