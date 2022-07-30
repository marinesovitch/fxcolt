// author: Darek Slusarczyk alias marines marinesovitch 2012-2013, 2022
#ifndef INC_CPP_CONVERTER_H
#define INC_CPP_CONVERTER_H

#include "types.h"

namespace cpp
{

//-----------------------------------------------------------------------------
// enum_map template
//-----------------------------------------------------------------------------

enum enum_mapping
{
	ENUM2STR_ONLY,
	STR2ENUM_ONLY,
	ENUM_STR_BIDIR
}; //enum_mapping

inline bool is_enum2str(const enum_mapping mapping)
{
	return (mapping == ENUM_STR_BIDIR) || (mapping == ENUM2STR_ONLY);
} //is_enum2str

inline bool is_str2enum(const enum_mapping mapping)
{
	return (mapping == ENUM_STR_BIDIR) || (mapping == STR2ENUM_ONLY);
} //is_str2enum

template<typename Enum,typename Traits = std::less<std::string> >
class enum_map
{
	public:
		enum_map(const bool bidir = true):
			m_mapping(bidir ? ENUM_STR_BIDIR : ENUM2STR_ONLY)
		{
		}

		enum_map(const enum_mapping mapping):m_mapping(mapping)
		{
		}

		bool empty()const
		{
			return m_enum2str.empty();
		}

		enum_map& operator()(
			const Enum enumval,
			const std::string& strval)
		{
			if(is_enum2str(m_mapping))
			{
				assert(m_enum2str.count(enumval) == 0);
				m_enum2str[enumval] = strval;
				m_enums.push_back(enumval);
			}

			if(is_str2enum(m_mapping))
			{
				//U try to register two different keys with the same
				//value
				//if U don't want to map values to key, then use
				//constructor with flag bidir==false (default is true)
				assert(m_str2enum.count(strval) == 0);
				m_str2enum[strval] = enumval;
				m_strings.push_back(strval);
			}

			return *this;
		}

	public:
		typedef std::map<Enum,std::string>  enum2str_t;
		typedef typename enum2str_t::const_iterator  enum2str_c_it;

		typedef std::vector<Enum>           enums_t;
		typedef typename enums_t::const_iterator     enums_c_it;

		typedef std::map<std::string,Enum,Traits>  str2enum_t;
		typedef typename str2enum_t::const_iterator  str2enum_c_it;

	public:
		//determines whether maps value to key too
		const enum_mapping m_mapping;

		enum2str_t m_enum2str;
		enums_t m_enums;

		str2enum_t m_str2enum;
		strings_t m_strings;

}; //enum_map

//-----------------------------------------------------------------------------
// enum_conv template
//-----------------------------------------------------------------------------

template<typename Enum,typename Traits = std::less<std::string> >
class enum_conv
{
	public:
		enum_conv(const enum_map<Enum,Traits>& enummap);

	public:
		bool to_str(const Enum value,std::string* str)const;
		const std::string& to_str(const Enum value)const;
		const std::string& to_str(const Enum value,const std::string& default_value)const;

		bool from_str(const std::string& str,Enum* value)const;
		Enum from_str(const std::string& str)const;
		Enum from_str(const std::string& str,const Enum default_value)const;

	public:
		strings_cit beginEnums()const;
		strings_cit endEnums()const;

		strings_cit beginStrings()const;
		strings_cit endStrings()const;

	public:
		typedef std::map<Enum,std::string> enum2str_t;
		typedef typename enum2str_t::const_iterator enum2str_c_it;

		typedef std::vector<Enum>           enums_t;
		typedef typename enums_t::const_iterator     enums_c_it;

		typedef std::map<std::string,Enum,Traits> str2enum_t;
		typedef typename str2enum_t::const_iterator str2enum_c_it;

	public:
		const enum_mapping m_mapping;

		enum2str_t m_enum2str;
		enums_t m_enums;

		str2enum_t m_str2enum;
		strings_t m_strings;

}; //enum_conv

template<typename Enum,typename Traits>
enum_conv<Enum,Traits>::enum_conv(const enum_map<Enum,Traits>& enummap):
	m_mapping(enummap.m_mapping),
	m_enum2str(enummap.m_enum2str),
	m_enums(enummap.m_enums),
	m_str2enum(enummap.m_str2enum),
	m_strings(enummap.m_strings)
{
} //enum_conv<Enum,Traits>::enum_conv

template<typename Enum,typename Traits>
bool enum_conv<Enum,Traits>::to_str(const Enum value,std::string* str)const
{
	assert(is_enum2str(m_mapping));
	enum2str_c_it it = m_enum2str.find(value);
	if(it != m_enum2str.end())
	{
		*str = it->second;
		return true;
	}
	else
	{
		return false;
	}
} //enum_conv<Enum,Traits>::to_str

template<typename Enum,typename Traits>
const std::string& enum_conv<Enum,Traits>::to_str(const Enum value)const
{
	assert(is_enum2str(m_mapping));
	enum2str_c_it it = m_enum2str.find(value);
	assert(it != m_enum2str.end());
	const std::string& sigtypestr = it->second;
	return sigtypestr;
} //enum_conv<Enum>::to_str

template<typename Enum,typename Traits>
const std::string& enum_conv<Enum,Traits>::to_str(
	const Enum value,
	const std::string& default_value)const
{
	assert(is_enum2str(m_mapping));
	enum2str_c_it it = m_enum2str.find(value);
	if(it != m_enum2str.end())
	{
		const std::string& sigtypestr = it->second;
		return sigtypestr;
	}
	else
	{
		return default_value;
	}
} //enum_conv<Enum,Traits>::to_str

template<typename Enum,typename Traits>
bool enum_conv<Enum,Traits>::from_str(const std::string& str,Enum* value)const
{
	assert(is_str2enum(m_mapping));
	str2enum_c_it it = m_str2enum.find(str);
	if(it != m_str2enum.end())
	{
		*value = it->second;
		return true;
	}
	else
	{
		return false;
	}
} //enum_conv<Enum,Traits>::from_str

template<typename Enum,typename Traits>
Enum enum_conv<Enum,Traits>::from_str(const std::string& str)const
{
	assert(is_str2enum(m_mapping));
	str2enum_c_it it = m_str2enum.find(str);
	assert(it != m_str2enum.end());
	const Enum value = it->second;
	return value;
} //enum_conv<Enum,Traits>::from_str

template<typename Enum,typename Traits>
Enum enum_conv<Enum,Traits>::from_str(
	const std::string& str,
	const Enum default_value)const
{
	assert(is_str2enum(m_mapping));
	str2enum_c_it it = m_str2enum.find(str);
	if(it != m_str2enum.end())
	{
		const Enum value = it->second;
		return value;
	}
	else
	{
		return default_value;
	}
} //enum_conv<Enum>::from_str

template<typename Enum,typename Traits>
strings_cit enum_conv<Enum,Traits>::beginEnums()const
{
	assert(is_enum2str(m_mapping));
	return m_enums.begin();
} //enum_conv<Enum,Traits>::beginEnums

template<typename Enum,typename Traits>
strings_cit enum_conv<Enum,Traits>::endEnums()const
{
	assert(is_enum2str(m_mapping));
	return m_enums.end();
} //enum_conv<Enum,Traits>::endEnums

template<typename Enum,typename Traits>
strings_cit enum_conv<Enum,Traits>::beginStrings()const
{
	assert(is_str2enum(m_mapping));
	return m_strings.begin();
} //enum_conv<Enum,Traits>::beginStrings

template<typename Enum,typename Traits>
strings_cit enum_conv<Enum,Traits>::endStrings()const
{
	assert(is_str2enum(m_mapping));
	return m_strings.end();
} //enum_conv<Enum,Traits>::endStrings

//-----------------------------------------------------------------------------
// predef_inserter template
//-----------------------------------------------------------------------------

template<typename T>
class predef_inserter
{
	public:
		typedef std::vector<T> values_t;

	public:
		predef_inserter& operator()(const T& value)
		{
			m_values.push_back(value);
			return *this;
		}

	public:
		values_t m_values;

}; //predef_inserter

//-----------------------------------------------------------------------------
// predef_values template
//-----------------------------------------------------------------------------

template<typename Key,typename Traits = std::less<Key> >
class predef_values
{
	public:
		typedef std::set<Key,Traits> values_t;
		typedef typename values_t::iterator values_it;
		typedef typename values_t::const_iterator values_c_it;

		typedef values_c_it iterator;
		typedef values_c_it const_iterator;

		typedef predef_inserter<Key> inserter_t;

	public:
		predef_values(const predef_inserter<Key>& inserter):
			m_values(
				inserter.m_values.begin(),
				inserter.m_values.end())
		{
		}

	public:
		bool empty()const
		{
			return m_values.empty();
		}

		size_t count(const Key& value)const
		{
			return m_values.count(value);
		}

		iterator find(const Key& value)
		{
			return m_values.find(value);
		}

		const_iterator find(const Key& value)const
		{
			return m_values.find(value);
		}

	public:
		iterator begin()
		{
			return m_values.begin();
		}

		iterator end()
		{
			return m_values.end();
		}

		const_iterator begin()const
		{
			return m_values.begin();
		}

		const_iterator end()const
		{
			return m_values.end();
		}

	public:
		values_t m_values;

}; //predef_values

//-----------------------------------------------------------------------------
// predef_map_inserter template
//-----------------------------------------------------------------------------

template<typename K,typename V>
class predef_map_inserter
{
	public:
		typedef std::pair<K,V> value_t;
		typedef std::vector<value_t> values_t;

	public:
		predef_map_inserter& operator()(const K& key,const V& value)
		{
			m_values.push_back(value_t(key,value));
			return *this;
		}

	public:
		values_t m_values;

}; //predef_map_inserter

//-----------------------------------------------------------------------------
// predef_map_values template
//-----------------------------------------------------------------------------

template<typename Key,typename Value,typename Traits = std::less<Key> >
class predef_map_values
{
	public:
		typedef std::map<Key,Value,Traits> values_t;
		typedef typename values_t::iterator values_it;
		typedef typename values_t::const_iterator values_c_it;

		typedef values_c_it iterator;
		typedef values_c_it const_iterator;

		typedef predef_map_inserter<Key,Value> inserter_t;

	public:
		predef_map_values(
			const predef_map_inserter<Key,Value>& inserter):
			m_allow_default_value(false),
			m_values(
				inserter.m_values.begin(),
				inserter.m_values.end())
		{
		}

		predef_map_values(
			const Value& default_value,
			const predef_map_inserter<Key,Value>& inserter):
			m_allow_default_value(true),
			m_default_value(default_value),
			m_values(
				inserter.m_values.begin(),
				inserter.m_values.end())
		{
		}

	public:
		bool empty()const
		{
			return m_values.empty();
		}

		size_t count(const Key& key)const
		{
			return m_values.count(key);
		}

		iterator find(const Key& key)
		{
			return m_values.find(key);
		}

		const_iterator find(const Key& key)const
		{
			return m_values.find(key);
		}

		const Value& operator[](const Key& key)const
		{
			const_iterator it = m_values.find(key);
			if(it != m_values.end())
			{
				const Value& value = it->second;
				return value;
			}
			else
			{
				if(m_allow_default_value)
				{
					return m_default_value;
				}
				else
				{
					assert(!"U may use constructor with default value to avoid this exception");
					throw std::invalid_argument("incorrect key");
				}
			}
		}

	public:
		iterator begin()
		{
			return m_values.begin();
		}

		iterator end()
		{
			return m_values.end();
		}

		const_iterator begin()const
		{
			return m_values.begin();
		}

		const_iterator end()const
		{
			return m_values.end();
		}

	public:
		const bool m_allow_default_value;
		const Value m_default_value;

		values_t m_values;

}; //predef_map_values

//------------------------------------------------------------------------
// reference <-> pointer converters
//------------------------------------------------------------------------

template<typename T>
struct ref2ptr
{
	T* operator()(T& ref)const
	{
		return &ref;
	}
}; //ref2ptr

template<typename T>
struct cref2ptr
{
	const T* operator()(const T& ref)const
	{
		return &ref;
	}
}; //cref2ptr

template<typename T>
struct cptr2ref
{
	const T& operator()(const T* ptr)const
	{
		return *ptr;
	}
}; //cref2ptr

} //namespace cpp

#endif
