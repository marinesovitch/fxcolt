// author: Darek Slusarczyk alias marines marinesovitch 2012-2013, 2022
#include "ph.h"
#include "namedPipe.h"
#include "fileUtils.h"
#include "consts.h"
#include "common/utils.h"

namespace fx
{

KNamedPipeBase::KNamedPipeBase(const std::string& path, const std::size_t bufferSize)
	: m_path(path)
	, m_handle(INVALID_HANDLE_VALUE)
	, m_bufferSize(bufferSize)
	, m_buffer(new char[bufferSize])
{
}

KNamedPipeBase::~KNamedPipeBase()
{
	close();
	delete[] m_buffer;
}

bool KNamedPipeBase::isValid() const
{
	const bool result = (m_handle != INVALID_HANDLE_VALUE);
	return result;
}

void KNamedPipeBase::close()
{
	if (m_handle != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(m_handle);
		m_handle = INVALID_HANDLE_VALUE;
	}
}

// ---------------------------------------------------------------------------

bool KNamedPipeBase::read(std::string* msg)
{
	const std::size_t msgSize = read();
	const bool result = (0 < msgSize);
	if (result)
	{
		msg->assign(m_buffer, m_buffer + msgSize);
	}
	return result;
}

std::size_t KNamedPipeBase::read(const std::size_t dataSize)
{
	assert(dataSize <= m_bufferSize);
	std::size_t result = 0;

	unsigned long bytesRead;
	bool success = ::ReadFile(
		m_handle,
		m_buffer,
		dataSize ? dataSize : m_bufferSize,
		&bytesRead,
		nullptr) ? true : false;

	if (success)
	{
		if (0 < bytesRead)
		{
			result = bytesRead;
		}
		else
		{
			success = false;
		}
	}

	if (!success || (bytesRead == 0))
	{
		const int lastError = GetLastError();
		switch (lastError)
		{
			case ERROR_BROKEN_PIPE:
				close();
				break;

			case ERROR_MORE_DATA:
				//result = read(msg);
				break;

			case ERROR_PIPE_LISTENING:
				//waiting for connection
				utils::sleep(1000);
				break;
		}
	}

	return result;
}

// ---------------------------------------------------------------------------

bool KNamedPipeBase::write(const std::string& msg)
{
	const bool result = write(msg.c_str(), msg.length());
	return result;
}

bool KNamedPipeBase::write(const char* data, const std::size_t dataSize)
{
	unsigned long written;
	bool result = ::WriteFile(
		m_handle,
		data,
		dataSize,
		&written,
		nullptr) ? true : false;

	if (!result || (dataSize != written))
	{
		result = false;
		close();
	}

	return result;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

KNamedPipeServer::KNamedPipeServer(const std::string& path)
	: KNamedPipeBase(path, consts::NamedPipeBufferSize)
{
}

bool KNamedPipeServer::create()
{
	assert(m_handle == INVALID_HANDLE_VALUE);

	m_handle = ::CreateNamedPipe(
		m_path.c_str()
		, /*GENERIC_READ | */PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT
		, PIPE_UNLIMITED_INSTANCES
		, m_bufferSize
		, m_bufferSize
		, 0
		, nullptr);

	bool result = false;
	if (m_handle != INVALID_HANDLE_VALUE)
	{
		if (::ConnectNamedPipe(m_handle, NULL) ? true : (GetLastError() == ERROR_PIPE_CONNECTED))
		{
			result = true;
		}
		else
		{
			close();
		}
	}

	return result;
}

void KNamedPipeServer::close()
{
	if (m_handle != INVALID_HANDLE_VALUE)
	{
		::FlushFileBuffers(m_handle);
		::DisconnectNamedPipe(m_handle);
		KNamedPipeBase::close();
	}
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

KNamedPipeClient::KNamedPipeClient(const std::string& path)
	: KNamedPipeBase(path, consts::NamedPipeBufferSize)
{
}

// ---------------------------------------------------------------------------

bool KNamedPipeClient::open()
{
	assert(m_handle == INVALID_HANDLE_VALUE);

	//if (fs::pathExists(m_path))
	{
		m_handle = ::CreateFile(
			m_path.c_str(),
			GENERIC_READ | GENERIC_WRITE,
			0,
			nullptr,
			OPEN_EXISTING,
			0, //FILE_ATTRIBUTE_NORMAL,
			nullptr);
	}

	const bool result = (m_handle != INVALID_HANDLE_VALUE);
	return result;
}

} // namespace fx
