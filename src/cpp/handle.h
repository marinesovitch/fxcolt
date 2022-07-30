// author: Darek Slusarczyk alias marines marinesovitch 2012-2013, 2022
#ifndef INC_CPP_HANDLE_H
#define INC_CPP_HANDLE_H

#include <cstdint>

namespace cpp
{

template< class TAG >
class THandle
{
	public:
		THandle() : m_value( INVALID ) 
		{
		}

		explicit THandle( const int32_t v ) : m_value( v ) 
		{
		}

		THandle( const THandle& rhs ) : m_value( rhs.m_value ) 
		{
		}

		const THandle& operator=( const THandle& rhs ) 
		{ 
			m_value = rhs.m_value; 
			return *this; 
		}

		operator int32_t() const 
		{ 
			return m_value; 
		}

		bool isValid() const 
		{ 
			const bool result = ( m_value != INVALID );
			return result; 
		}

		bool isNull() const 
		{ 
			const bool result = ( m_value == INVALID );
			return result; 
		}

		THandle& operator++()
		{
			m_value++;
			return *this;
		}

		THandle operator++(int)
		{
			const THandle result(*this);
			m_value++;
			return result;
		}

	private:
		int32_t m_value;

	public:
		typedef TAG tag_type;
		static const int32_t INVALID = INT32_MIN;
};

// ---------------------------------------------------------------------------

template< class TAG >
class THandle64
{
	public:
		THandle64() : m_value( INVALID ) 
		{
		}

		explicit THandle64( const int64_t v ) : m_value( v ) 
		{
		}

		THandle64( const THandle64& rhs ) : m_value( rhs.m_value ) 
		{
		}

		const THandle64& operator=( const THandle64& rhs ) 
		{ 
			m_value = rhs.m_value; 
			return *this; 
		}

		operator int64_t() const 
		{ 
			return m_value; 
		}

		bool isValid() const 
		{ 
			const bool result = ( m_value != INVALID );
			return result; 
		}

		bool isNull() const 
		{ 
			const bool result = ( m_value == INVALID );
			return result; 
		}

		THandle64& operator++()
		{
			m_value++;
			return *this;
		}

		THandle64 operator++(int)
		{
			const THandle result(*this);
			m_value++;
			return result;
		}

	private:
		int64_t m_value;

	public:
		typedef TAG tag_type;
		static const int64_t INVALID = INT64_MIN;
};

// ---------------------------------------------------------------------------

#define fx_define_handle_type(handle_name) \
	struct tag_fx_ ## handle_name {}; \
	typedef cpp::THandle< tag_fx_ ## handle_name > handle_name##_t;

#define fx_define_handle_type64(handle_name) \
	struct tag_fx_ ## handle_name {}; \
	typedef cpp::THandle64< tag_fx_ ## handle_name > handle_name##_t;

} // namespace cpp

namespace std
{

template< typename Tag >
std::istream& operator>>(std::istream& is, cpp::THandle< Tag >& handle)
{
	int32_t rawHandle;
	if (is >> rawHandle)
	{
		handle = cpp::THandle< Tag >(rawHandle);
	}
	return is;
}

template< typename Tag >
std::istream& operator>>(std::istream& is, cpp::THandle64< Tag >& handle)
{
	int64_t rawHandle;
	if (is >> rawHandle)
	{
		handle = cpp::THandle64< Tag >(rawHandle);
	}
	return is;
}

} // namespace std

#endif
