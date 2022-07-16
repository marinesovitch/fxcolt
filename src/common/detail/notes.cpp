#include "ph.h"
#include "notes.h"
#include "cpp/strUtils.h"

namespace fx
{

namespace note
{

namespace
{

const std::string NoteSeparator = "@#%&@";

} // anonymous namespace

const std::string RegisterSymbolLabel = "reg_symbol";
const std::string UnregisterSymbolLabel = "unreg_symbol";

std::string packNote( const cpp::strings_t& tokens )
{
	std::ostringstream os;
	std::copy(
		tokens.begin(),
		tokens.end(),
		std::ostream_iterator< std::string >( os, NoteSeparator.c_str() ));
	const std::string& result = os.str();
	return result;
}

bool unpackNote( const std::string& packedNote, const unsigned int expectedTokensCount, cpp::strings_t* tokens )
{
	const bool result = cpp::su::split(packedNote, NoteSeparator, tokens) && (tokens->size() == expectedTokensCount);
	return result;
}

} // namespace note

} // namespace fx
