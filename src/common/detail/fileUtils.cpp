#include "ph.h"
#include "fileUtils.h"
#include <cstring>

namespace fx
{

namespace utils
{

bool pathExists(const std::string& path)
{
	const bool result = ::PathFileExists(path.c_str()) ? true : false;
	return result;
}

bool enumNamedPipes(const std::string& filter, cpp::strings_t* pipes)
{
	const std::string NamedPipePrefix = "\\\\.\\pipe\\";
    const std::string TargetMask = NamedPipePrefix + "*";

    WIN32_FIND_DATA findFileData;

    memset(&findFileData, 0, sizeof(findFileData));
    HANDLE hFind = ::FindFirstFile(TargetMask.c_str(), &findFileData);
    if (hFind != INVALID_HANDLE_VALUE) 
    {
        do
        {
			if (strstr(findFileData.cFileName, filter.c_str()))
			{
				const std::string& pipe = NamedPipePrefix + findFileData.cFileName;
				pipes->push_back(pipe);
			}
        }
        while (::FindNextFile(hFind, &findFileData));

        ::FindClose(hFind);
    }

	const bool result = !pipes->empty();
    return result;
}

} // namespace utils

} // namespace fx
