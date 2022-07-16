#include "ph.h"
#include "cpp/streams.h"

namespace
{

void attach()
{
	const char* log_file_path = "c:/fxcolt/session.log";
	std::remove(log_file_path);
	static cpp::IStreamOutput* s_output = cpp::IStreamOutput::create_file_output( log_file_path );
	cpp::init_streams(s_output);
}

}

BOOL APIENTRY DllMain(
	HMODULE /*hModule*/,
	DWORD  ul_reason_for_call,
	LPVOID /*lpReserved*/)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			attach();
			break;

		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}
