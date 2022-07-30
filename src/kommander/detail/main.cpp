// author: Darek Slusarczyk alias marines marinesovitch 2012-2013, 2022
#include "ph.h"
#include "backend/instance.h"
#include "backend/executor.h"
#include "cpp/strUtils.h"

namespace fx
{

namespace
{

void runAppLoop()
{
	KInstance instance( &std::cout, &std::cerr );
	instance.run();

	IExecutor& executor = instance.executor();

	bool exit = false;
	std::string cmdLine;
	std::string error;
	while (!exit)
	{
		std::cout << "$ ";
		std::getline(std::cin, cmdLine);
		cpp::su::trim(&cmdLine);
		if (cmdLine == consts::CmdExit)
		{
			exit = true;
		}
		else
		{
			if (!executor.executeCommand(cmdLine, &error))
			{
				std::cerr << error << std::endl;
			}
		}
	}
}

} // anonymous namespace

} // namespace fx

int main()
{
	fx::runAppLoop();
}
