// author: Darek Slusarczyk alias marines marinesovitch 2012-2013, 2022
#ifndef INC_CPP_STREAMS_H
#define INC_CPP_STREAMS_H

namespace cpp 
{

//========================================================================
// IStreamOutput class
//========================================================================

enum ESeverity
{
    Normal,
    Warning,
    Error,
    Log,
    MsgLog
}; //ESeverity

struct IStreamOutput
{
    virtual ~IStreamOutput();

    virtual void dump(
        const ESeverity severity,
        const std::string& buffer) = 0;

    static IStreamOutput* create_file_output(
        const std::string& filepath,
        const bool print_severity = true,
        const bool print_time = true,
        const bool print_pid = true );

    static IStreamOutput* create_debug_output(
        const bool print_severity = true);

    static IStreamOutput* create_raw_file_output(
        const std::string& filepath);

}; //IStreamOutput

//========================================================================
// messagestream class
//========================================================================

/*
    it is comfortable std::ostream like interface for console
    you can dump any information to console using this stream just like
    any other std::ostream derived object
*/
//lint -esym(1714,messagestream::*)
class messagestream:public std::ostream
{
    public:
        typedef std::ostream inherited;

    public:
        explicit messagestream();
        virtual ~messagestream();

        void init ( IStreamOutput* pOutput, ESeverity severity );

        void enable ( bool bEnable );
        bool isEnabled() const;

    public:
        class Impl;
        std::unique_ptr<Impl>impl;

}; //messagestream

//========================================================================
// nullstream class
//========================================================================

class nullstream
{
    template< typename T >
    inline nullstream& operator << ( T )
    {
        return *this;
    }

    template< typename T >
    inline nullstream& operator << ( T (* /*fn*/)( T ) )
    {
        return *this;
    }
};

inline nullstream& operator << ( 
    nullstream& ns, std::ostream& (* /*fn*/)(std::ostream& ) )
{
    return ns;
}

//========================================================================
// global streams
//========================================================================

extern messagestream cout;
extern messagestream cwarn;
extern messagestream cerr;
extern messagestream clog;
extern messagestream cmsg;
extern messagestream cdbg;
extern nullstream cnul;
extern messagestream cusr;

#ifndef NDEBUG
    #define cdiag cdbg
#else
    #define cdiag cnul
#endif

void init_stream ( messagestream& stream, IStreamOutput* output, ESeverity severity );
void init_streams ( IStreamOutput* output );
void init_diag_stream();
void set_user_log_file(const std::string& filePath);

// -----------------------------------------------------------------------------

void stream_assert ( const char*, const char*, unsigned int, std::ostream& );

} // namespace cpp

// -----------------------------------------------------------------------------
#ifdef NDEBUG
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
#ifdef CPP_ASSERT_LOG_STREAM
// -----------------------------------------------------------------------------

#ifdef assert
#undef assert
#endif

#define _assert Error standard assert included.

// -----------------------------------------------------------------------------
#ifdef PE_DEBUG
// -----------------------------------------------------------------------------

#define assert(exp) (void)( (exp) || ( cpp::stream_assert(#exp, __FILE__, __LINE__,CPP_ASSERT_LOG_STREAM), 0 ) )

// -----------------------------------------------------------------------------
#else
// -----------------------------------------------------------------------------

#define assert(exp) ( ( void ) ( 0 ) )

// -----------------------------------------------------------------------------
#endif // PE_DEBUG
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
#endif // CPP_ASSERT_LOG_STREAM
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
#endif // NDEBUG
// -----------------------------------------------------------------------------

#endif // INC_CPPSTREAMS_H
