#ifndef INC_COMMON_NOTES_H
#define INC_COMMON_NOTES_H

#include "cpp/types.h"

namespace fx
{

namespace note
{

extern const std::string RegisterSymbolLabel;
extern const std::string UnregisterSymbolLabel;

enum EKind
{
	RegisterSymbol,
	UnregisterSymbol
};

std::string packNote( const cpp::strings_t& tokens );
bool unpackNote( const std::string& packedNote, const unsigned int expectedTokensCount, cpp::strings_t* tokens );

} // namespace note

} // namespace fx

#endif
