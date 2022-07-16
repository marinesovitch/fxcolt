#include "ph.h"
#include "utils.h"

namespace fx
{

namespace utils
{

std::string normalizeBrokerName(const std::string& rawBrokerName)
{
	std::string result;
	std::replace_copy_if(
		rawBrokerName.begin(), 
		rawBrokerName.end(), 
		std::back_inserter(result),		  
		[](const char chr)-> bool { return !std::isalnum(chr); },
		'_');
	return result;
}

bool isNormalizedBrokerName(const std::string& brokerName)
{
	const bool result = ( brokerName == utils::normalizeBrokerName( brokerName ) );
	return result;
}

namespace
{

std::string normalizePath(const std::string& rawPath)
{
	static const std::string s_allowableChars("\\/:.");
	std::string result;
	std::replace_copy_if(
		rawPath.begin(), 
		rawPath.end(), 
		std::back_inserter(result),		  
		[](const char chr)-> bool { return !std::isalnum(chr) && (s_allowableChars.find(chr) == std::string::npos); },
		'_');
	return result;
}

} // anonymous namespace

std::string preparePath(const EMedium kind, const std::string& dir)
{
	static const std::map<EMedium, std::string> s_kind2str = 
	{ 
		{EMedium::MailSlot, consts::MailSlotPrefix}, 
		{EMedium::NamedPipe, consts::PipePrefix} 
	};

	std::ostringstream os;
	auto it = s_kind2str.find(kind);
	assert(it != s_kind2str.end());
	const std::string& kindStr = it->second;
	os << "\\\\.\\" << kindStr << "\\" << consts::AppTag;
	if (!dir.empty())
	{
		os << "\\" + dir;
	}
		
	const std::string& rawPath = os.str();
	const std::string& result = normalizePath(rawPath);
	return result;
}

std::string preparePath(const EMedium kind, const SAccountInfo& accountInfo, const std::string& tag, const int index)
{
	std::ostringstream os;
	os << accountInfo.m_broker << '\\' << accountInfo.m_accountLogin << '\\' << tag;
	if (index != -1)
	{
		os << '\\' << index;
	}
	const std::string& dir = os.str();
	const std::string& result = preparePath(kind, dir);
	return result;
}

void sleep(const int durationMilliseconds)
{
	std::chrono::milliseconds dura(durationMilliseconds);
	std::this_thread::sleep_for(dura);
}

} // namespace utils

} // namespace fx
