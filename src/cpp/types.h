// author: Darek Slusarczyk alias marines marinesovitch 2012-2013, 2022
#ifndef INC_CPP_TYPES_H
#define INC_CPP_TYPES_H

namespace cpp 
{

typedef std::vector< unsigned int > uints_t;

typedef std::vector< int > ints_t;

typedef std::vector< char > chars_t;

typedef std::vector< std::string > strings_t;
typedef strings_t::const_iterator strings_cit;
typedef strings_t::iterator strings_it;

typedef std::vector< std::wstring > wstrings_t;

typedef std::set< std::string > stringset_t;

typedef std::map< std::string, std::string > stringmap_t;

} // namespace cpp

#endif
