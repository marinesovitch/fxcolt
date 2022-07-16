#include "ph.h"
#include "streams.h"
#include "datetime.h"

namespace cpp
{

//------------------------------------------------------------------------
// messagestream::Impl class
//------------------------------------------------------------------------

class messagestream::Impl:public std::streambuf
{
    struct traits_type
    {
        typedef char char_type;
        typedef int int_type;
        static bool eq_int_type(int_type lhs,int_type rhs)
        {
            return lhs == rhs;
        }
        static int_type eof()
        {
            return EOF;
        }
        static bool not_eof(int_type c)
        {
            return c != EOF;
        }
        static char_type to_char_type(int_type c)
        {
            return char_type(c);
        }
    };
    typedef char char_type;
    typedef int int_type;

    public:
        explicit Impl();
        virtual ~Impl();

    public:
        void init(
            IStreamOutput* output,
            const ESeverity severity);

    protected:
        virtual int sync();
        virtual std::streambuf* setbuf(char_type*s,std::streamsize n);
        virtual std::streamsize xsputn(const char_type *s,std::streamsize n);
        virtual int_type overflow(int_type c);

    private:
        void reset();
        void dump_msg();

    public:
        IStreamOutput* m_output;
        ESeverity m_severity;
        bool m_bEnabled;
        std::string m_buffer;
        std::mutex m_mutex;

}; //messagestream::Impl

//------------------------------------------------------------------------
// messagestream::Impl routines
//------------------------------------------------------------------------

messagestream::Impl::Impl():
    m_output(0),
    m_severity(Normal),
    m_bEnabled(false)
{
} //messagestream::Impl::Impl

messagestream::Impl::~Impl()
{
    delete m_output;
} //messagestream::Impl::~Impl

void messagestream::Impl::init(
    IStreamOutput* output,
    const ESeverity severity)
{
    if ( output != m_output )
    {
        delete m_output;
        m_output = output;
    }
    m_severity = severity;
} //messagestream::Impl::init

int messagestream::Impl::sync()
{
    std::lock_guard< std::mutex > lock ( m_mutex );
    dump_msg();
    return 0;
} //messagestream::Impl::sync

std::streambuf *messagestream::Impl::setbuf(char_type* /*s*/,std::streamsize n)
{
    std::lock_guard< std::mutex > lock ( m_mutex );
    m_buffer.reserve(size_t(n));
    return 0;
} //messagestream::Impl::setbuf

std::streamsize messagestream::Impl::xsputn(const char_type *s, std::streamsize n)
{
    std::lock_guard< std::mutex > lock ( m_mutex );

    //cache given data
    const char_type* beginData = s;
    const char_type* endData = s+n;
    m_buffer.append(beginData,endData);

    // if there is '\n' then dump message
    if(beginData != endData)
    {
        if ( std::find ( beginData, endData, '\n' ) != endData)
            dump_msg();
    }

    return n;
} //messagestream::Impl::xsputn

messagestream::Impl::traits_type::int_type messagestream::Impl::overflow(int_type c)
{
    if(!traits_type::eq_int_type(c,traits_type::eof()))
    {
        const char_type chr=traits_type::to_char_type(c);
        xsputn(&chr,1);
        return traits_type::not_eof(c);
    }
    else
    {
        return traits_type::eof();
    }
} //messagestream::Impl::overflow

void messagestream::Impl::reset()
{
    std::lock_guard< std::mutex > lock ( m_mutex );
    m_buffer.clear();
} //messagestream::Impl::reset

void messagestream::Impl::dump_msg()
{
    if(!m_buffer.empty())
    {
        if ( m_output != 0 )
            m_output->dump(m_severity,m_buffer);
        m_buffer.clear();
    }
} //messagestream::Impl::dump_msg

//========================================================================
// messagestream routines
//========================================================================

messagestream::messagestream():
    inherited(0),
    impl(new Impl())
{
    std::ostream::init(impl.get());
} //messagestream::messagestream

messagestream::~messagestream()
{
} //messagestream::~messagestream

void messagestream::init ( IStreamOutput* pOutput, ESeverity severity )
{
    impl->init ( pOutput, severity );
}

void messagestream::enable ( bool bEnable )
{
    impl->m_bEnabled = bEnable;
}

bool messagestream::isEnabled() const
{
    return impl->m_bEnabled;
}

//=============================================================================
// IStreamOutput routines
//=============================================================================

// -----------------------------------------------------------------------------
namespace {
// -----------------------------------------------------------------------------

std::string severity2text(const ESeverity severity)
{
    switch(severity)
    {
        case Normal:
            return "";

        case Warning:
            return "Warning: ";

        case Error:
            return "Error: ";

        case Log:
            return "";

        case MsgLog:
            return "";

        default:
            return "?: ";
    }
} //severity2text

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

class CFileOutput : public IStreamOutput
{
    public:
        CFileOutput(
            const std::string& filepath,
            const bool print_severity,
            const bool print_time,
            const bool print_pid);

    private:
        CFileOutput& operator=(const CFileOutput& rhs);

    public:
        virtual void dump(
           const ESeverity severity,
           const std::string& buffer);

    private:
        void printTime( std::ofstream& os );

    private:
        const std::string m_filepath;
        const bool m_print_severity;
        const bool m_print_time;
        const bool m_print_pid;

        uint64_t m_prevMiliCounter;

}; //CFileOutput

// -----------------------------------------------------------------------------

CFileOutput::CFileOutput(
    const std::string& filepath,
    const bool print_severity,
    const bool print_time,
    const bool print_pid):
    m_filepath(filepath),
    m_print_severity(print_severity),
    m_print_time(print_time),
    m_print_pid(print_pid),
    m_prevMiliCounter( cpp::miliCount() )
{
} //CFileOutput::CFileOutput

// -----------------------------------------------------------------------------

void CFileOutput::dump(
    const ESeverity severity,
    const std::string& buffer)
{
    std::ofstream os ( m_filepath.c_str(), std::ios_base::out | std::ios_base::app );
    if ( ! os.bad() )
    {
        if ( m_print_pid )
            os << std::this_thread::get_id() << ": ";
        if ( m_print_time )
            printTime( os );
        if ( m_print_severity )
            os << severity2text ( severity );
        os << buffer;
    }
} //CFileOutput::dump

void CFileOutput::printTime( std::ofstream& os )
{
    cpp::STimeInfo timeInfo;
    if ( cpp::getLocalTime( &timeInfo ) )
    {
        os << std::setfill( '0' )
            << std::setw( 2 ) << timeInfo.m_hour << ':'
            << std::setw( 2 ) << timeInfo.m_minute << ':'
            << std::setw( 2 ) << timeInfo.m_second << ' ';
    }

    const uint64_t miliCounter = cpp::miliCount();
    const uint64_t miliCounterDelta = miliCounter - m_prevMiliCounter;
    os << "(" << miliCounter;
    if ( 0 < miliCounterDelta )
        os << " +" << miliCounterDelta;
    os << ") ";
    m_prevMiliCounter = miliCounter;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

class CRawFileOutput : public IStreamOutput
{
    public:
        CRawFileOutput(const std::string& filepath);

    private:
        CRawFileOutput& operator=(const CRawFileOutput& rhs);

    public:
        virtual void dump(const ESeverity severity,const std::string& buffer);

    private:
        const std::string m_filepath;

}; //CRawFileOutput

// -----------------------------------------------------------------------------

CRawFileOutput::CRawFileOutput(const std::string& filepath):
    m_filepath(filepath)
{
} //CRawFileOutput::CRawFileOutput

// -----------------------------------------------------------------------------

void CRawFileOutput::dump(const ESeverity,const std::string& buffer)
{
    std::ofstream os ( m_filepath.c_str(), std::ios_base::out | std::ios_base::app );
    if ( ! os.bad() )
        os << buffer;
} //CRawFileOutput::dump

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

class CDebugOutput : public IStreamOutput
{
    public:
        CDebugOutput(
            const bool print_severity);

    private:
        CDebugOutput& operator=(const CDebugOutput& rhs);

    public:
        virtual void dump(
           const ESeverity severity,
           const std::string& buffer);

    private:
        const std::string m_filepath;
        const bool m_print_severity;

}; //CDebugOutput

// -----------------------------------------------------------------------------

CDebugOutput::CDebugOutput(
    const bool print_severity):
        m_print_severity(print_severity)
{
} //CDebugOutput::CDebugOutput

// -----------------------------------------------------------------------------

void CDebugOutput::dump(
    const ESeverity severity,
    const std::string& buffer)
{
    std::stringstream sst;
    sst << std::this_thread::get_id() << ": ";
    sst  << " (" << cpp::miliCount() << ") ";

    if (m_print_severity)
    {
        sst << severity2text(severity);
    }
    sst << buffer;
    #ifdef CPP_PLATFORM_WINDOWS
        ::OutputDebugString ( sst.str().c_str() );
    #else
        std::cout << sst.str();
    #endif
} //CDebugOutput::dump

// -----------------------------------------------------------------------------
} // anonymous namespace
// -----------------------------------------------------------------------------

IStreamOutput::~IStreamOutput()
{
} //IStreamOutput::~IStreamOutput

// -----------------------------------------------------------------------------

IStreamOutput* IStreamOutput::create_file_output(
    const std::string& filepath,
    const bool print_severity,
    const bool print_time,
    const bool print_pid)
{
    IStreamOutput* output = new CFileOutput(filepath,print_severity,print_time,print_pid);
    return output;
} //IStreamOutput::create_file_output

// -----------------------------------------------------------------------------

IStreamOutput* IStreamOutput::create_raw_file_output(
    const std::string& filepath)
{
    IStreamOutput* output = ( ! filepath.empty() ? new CRawFileOutput(filepath) : 0 );
    return output;
} //IStreamOutput::create_raw_file_output

// -----------------------------------------------------------------------------

IStreamOutput* IStreamOutput::create_debug_output(
    const bool print_severity)
{
    IStreamOutput* output = new CDebugOutput(print_severity);
    return output;
} //IStreamOutput::create_file_output

//=============================================================================
// global streams
//=============================================================================

messagestream cout;
messagestream cwarn;
messagestream cerr;
messagestream clog;
messagestream cmsg;
messagestream cdbg;
nullstream cnul;
messagestream cusr;

// -----------------------------------------------------------------------------

void init_stream ( messagestream& stream, IStreamOutput* output, ESeverity severity )
{
    stream.impl->init(output,severity);
} //init_stream

// -----------------------------------------------------------------------------

void init_streams ( IStreamOutput* output )
{
    cout.impl->init(output,Normal);
    cwarn.impl->init(output,Warning);
    cerr.impl->init(output,Error);
    clog.impl->init(output,Log);
    cmsg.impl->init(output,MsgLog);
    cdbg.impl->init(IStreamOutput::create_debug_output(true),MsgLog);
} //init_streams

// -----------------------------------------------------------------------------

void init_diag_stream()
{
    cdbg.impl->init(IStreamOutput::create_debug_output(true),MsgLog);
} //init_diag_stream

// -----------------------------------------------------------------------------

void set_user_log_file ( const std::string& filePath )
{
    static std::mutex s_mutex;
    std::lock_guard< std::mutex > lock ( s_mutex );
    cusr.impl->init(
        IStreamOutput::create_file_output( filePath, false, true, false ), MsgLog );
} //set_user_log_file

// -----------------------------------------------------------------------------

void stream_assert (
    const char* sExpression,
    const char* sFileName,
    unsigned int nLine,
    std::ostream& outputStream )
{
    outputStream
        << "Assertion failed in " << sFileName
        << " at line " << std::dec << nLine
        << ": " << sExpression
        << std::endl;
}

} // namespace cpp
