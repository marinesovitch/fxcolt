// author: Darek Slusarczyk alias marines marinesovitch 2012-2013, 2022
#include "ph.h"
#include "strUtils.h"
#include <codecvt>

namespace cpp
{

namespace su
{

std::string w2str(const wchar_t* wstr)
{
    using convert_type = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_type, wchar_t> converter;
    return converter.to_bytes(wstr);
}

std::string w2str(const std::wstring& wstr)
{
    using convert_type = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_type, wchar_t> converter;
    return converter.to_bytes(wstr.data(), wstr.data() + wstr.length());
}

std::wstring str2w(const char* str)
{
	const std::size_t len = strlen(str);
	const char* end = str + len;
	std::wstring result(str, end);
	return result;
}

std::wstring str2w(const std::string& str)
{
	std::wstring result(str.begin(), str.end());
	return result;
}

void str2w(const std::string& str, wchar_t* wchr)
{
	const std::wstring& wstr = str2w(str);
	wcscpy(wchr, wstr.data());
}

//------------------------------------------------------------------------

bool isInteger(const std::string& str)
{
	const bool result = std::all_of(str.begin(), str.end(), &::isdigit);
	return result;
}

//------------------------------------------------------------------------

std::string trim(const std::string& str)
{
	const std::string& result = boost::algorithm::trim_copy(str);
	return result;
}

void trim(std::string* str)
{
	boost::algorithm::trim_copy(*str);
}

bool split(const std::string& str, const char separator, cpp::strings_t* tokens)
{
	const std::string filter(1, separator);
	boost::algorithm::split(*tokens, str, boost::algorithm::is_any_of(filter));
	const bool result = !tokens->empty();
	return result;
}

bool split(const std::string& str, const std::string& separator, cpp::strings_t* tokens)
{
    auto begin = 0;
    auto end = str.find( separator );
    while (end != std::string::npos)
    {
		const std::string token( str, begin, end - begin);
		tokens->push_back( token );

        begin = end + separator.length();
        end = str.find( separator, begin );
    }

	const bool result = !tokens->empty();
	return result;
}

} // namespace su

} // namespace cpp
