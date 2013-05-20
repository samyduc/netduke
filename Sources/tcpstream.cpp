#include "tcpstream.h"

#include "layer.h"
#include "listener.h"
#include "crc.h"

namespace NetDuke
{


TCPStream::TCPStream(const Peer& _peer)
	: m_peer(_peer)
	, m_isValid(false)
	, m_socket(0)
	, m_opt_compression(false)
	, m_opt_encryption(false)
	, m_opt_skipCRC(true)
	, m_pool(50)
	, m_fdmax(0)
{

}

void TCPStream::Tick()
{
	FlushSend();
	FlushRecv();
}

void TCPStream::Flush()
{
	for(channels_t::iterator it=m_sendChannels.begin(); it != m_sendChannels.end(); ++it)
	{
		delete (*it).second;
	}
	m_sendChannels.clear();

	for(listeners_t::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
	{
		Listener *listener = (*it);
		listener->UnRegisterStream(*this);
	}
	m_listeners.clear();

	for(connections_t::iterator it = m_connections.begin(); it != m_connections.end(); ++it)
	{
		ConnectionPacker *connection = (*it).second;
		delete connection;
	}
	m_connections.clear();

}

void TCPStream::FlushSend()
{
	for(channels_t::iterator it=m_sendChannels.begin(); it != m_sendChannels.end(); ++it)
	{
		Channel& channel = *(*it).second;
		const Peer &peer = channel.GetPeer();

		if(!channel.IsEmpty())
		{
			SerializerLess ser;
			channel.Pop(ser);
			SendTo(ser, peer);
		}
	}
}

void TCPStream::FlushRecv()
{
	struct timeval time_val;
	time_val.tv_sec = 0;
	time_val.tv_usec = 0;

	fd_set rset;
	FD_ZERO(&rset);
	FD_SET(m_fdmax, &rset);

	// si readable mais nexiste pas, enter a new connection

	// TODO: replace m_socket and all sockets in maps
	if(select(static_cast<int>(m_fdmax + 1), &rset, NULL, NULL, &time_val) >= 0)
	{
		if(FD_ISSET(m_socket, &rset) > 0)
		{
			// incoming connection
			Accept();
		}

		for(connections_t::iterator it = m_connections.begin(); it != m_connections.end(); ++it)
		{
			const Peer &peer = it->first;
			ConnectionPacker& connection = *it->second;
			
			if(FD_ISSET(connection.m_socket, &rset) > 0)
			{
				if(!RecvFrom(connection, peer))
				{
					m_connections.erase(it);
				}
			}
		}
	}	
}

void TCPStream::SendTo(Serializer& _ser, const Peer& _peer)
{
	// find the socket
	struct ConnectionPacker* connection = GetOrCreateConnection(_peer);
	assert(connection != nullptr);

	int len = send(
						connection->m_socket,
						reinterpret_cast<const char*>(_ser.GetBuffer()),
						static_cast<int>(_ser.GetSize()),
						0);

	// TODO : handle this case
	assert(len == _ser.GetSize());
}

netBool TCPStream::RecvFrom(struct ConnectionPacker& _connection, const Peer& _peer)
{
	netBool is_ok = false;
	// TODO : Optim, if already a serializer not full, try to append data
	Serializer& ser = m_pool.GetSerializer();

	int len = recv(
				_connection.m_socket,
				reinterpret_cast<char*>(ser.GetRawBuffer()),
				static_cast<int>(ser.GetBufferSize()),
				0
			);

	is_ok = len > 0;

	if(is_ok)
	{
		CheckIncomingPacket(len, _connection, ser, _peer);
	}

	return is_ok;
}

void TCPStream::CheckIncomingPacket(size_t _len, ConnectionPacker& _connection, Serializer& _ser, const Peer& _peer)
{
	netBool is_new = _connection.m_written == 0;
	if(is_new)
	{
		_connection.m_ser = SerializerLess(_ser);
	}
	else
	{
		memcpy(_connection.m_ser.GetRawBuffer()+_connection.m_ser.GetCursor(), _ser.GetRawBuffer(), _len);
	}

	_connection.m_ser.SetCursor(_connection.m_ser.GetCursor() + _len);
	_connection.m_written += _len;

	if(_connection.m_written > Serializer::GetHeaderSize())
	{
		size_t diff = _connection.m_written - _connection.m_ser.GetSize();
			
		if(diff == 0)
		{
			// packet strictly complete
			_connection.m_ser.SetCursor(_connection.m_ser.GetSize());
			UnPack(_connection.m_ser, _peer);
			_connection.m_written = 0;
		}
		else if(diff > 0)
		{
			// packet plus another one
			_connection.m_ser.SetCursor(_connection.m_ser.GetSize());
			UnPack(_connection.m_ser, _peer);
			_connection.m_written = 0;

			// take another Serializer
			Serializer& ser = m_pool.GetSerializer();
			memcpy(ser.GetRawBuffer(), _ser.GetRawBuffer() +  _ser.GetSize(), diff);

			// loop again
			CheckIncomingPacket(diff, _connection, ser, _peer);
		}
	}
}

void TCPStream::Accept()
{
	Peer peer;
	const sockaddr_in &native_addr = peer.GetNativeStruct();

	int native_len = sizeof(native_addr);
	SOCKET connection = accept(m_socket, (SOCKADDR*)&native_addr, &native_len);

	// add connection to socket pool

	if(connection > m_fdmax)
	{
		m_fdmax = connection;
	}

	ConnectionPacker* packer = new ConnectionPacker(connection);
	
	connections_t::iterator it = m_connections.find(peer);

	if(it == m_connections.end())
	{
		m_connections[peer] = packer;
	}
	else
	{
		// connection disconnected, must have been previously deleted
		assert(false);
	}
}

void TCPStream::Init()
{
	// just here to bypass CreateAndBind
	m_isValid = true;
}

void TCPStream::CreateAndBind()
{
	assert(IsValid() == false);

	const SOCKADDR_IN &native_addr = m_peer.GetNativeStruct();
	m_socket = socket(native_addr.sin_family, SOCK_STREAM, 0);

	assert(m_socket > 0);
	// no blocking socket
	//u_long iMode=1;
	//ioctlsocket(m_socket, FIONBIO, &iMode);

	char yes=1;
	setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(char));

	int err = bind(m_socket, (SOCKADDR*)&native_addr, sizeof(native_addr));

	assert(err == 0);

	// overwrite port (if automatic attribution)
	if(m_peer.GetPort() == 0)
	{
		// WARNING : incompatible with emscripten
		socklen_t native_size = sizeof(native_addr);
		err = getsockname(m_socket, (SOCKADDR*)&native_addr, &native_size);
		assert(err == 0);
	}

	listen(m_socket, 4);

	m_isValid = (m_socket != 0);

	m_fdmax = m_socket;
}

size_t TCPStream::GetHeaderSize() const
{
	// serializer + stream
	return Serializer::GetHeaderSize() + sizeof(netU32);
}

netBool TCPStream::Pull(SerializerLess &_ser, Peer& _peer)
{
	(void)_ser;
	(void)_peer;
	return false;
}

netBool TCPStream::Push(SerializerLess &_ser, const Peer& _peer)
{
	assert(m_isValid);

	Pack(_ser, _peer);

	return true;
}

netBool TCPStream::Pack(SerializerLess& _ser, const Peer& _peer)
{
	Channel& channel = GetChannel(m_sendChannels, _peer);

	_ser.ResetCursor();

	// read upper layer size, compute crc
	//_ser.Read(s_typeListener);
	SerializerLess upper_ser(_ser, GetHeaderSize(), _ser.GetBufferSize());

	netU32 crc = 0;

	if(!m_opt_skipCRC)
	{
		crc = CRC32::Compute(_ser.GetBuffer() + GetHeaderSize(), upper_ser.GetSize() - GetHeaderSize());
	}

	_ser.Write(s_typeTCPStream);
	_ser << crc;
	_ser.SetCursor(GetHeaderSize() + upper_ser.GetSize());
	_ser.Close();

	if(m_opt_compression)
	{
		// TODO : add compression
	}

	if(m_opt_encryption)
	{
		// TODO : add encryption
	}

	channel.Push(_ser);

	return true;
}

netBool TCPStream::UnPack(SerializerLess& _ser, const Peer& _peer)
{
	if(_ser.Read(s_typeTCPStream))
	{
		// check frame consistency (even if the type is correct)
		if(_ser.GetSize() <= _ser.GetBufferSize())
		{
			// Packet seems well formed, check data crc for consitency now
			netU32 crc_ser;
			netU32 crc_cmp;
			static_cast<Serializer&>(_ser) >> crc_ser;

			SerializerLess upper_ser(_ser, GetHeaderSize(), _ser.GetBufferSize());

			if(!m_opt_skipCRC)
			{
				crc_cmp = CRC32::Compute(_ser.GetBuffer() + GetHeaderSize(), upper_ser.GetSize() - GetHeaderSize());
			}
			else
			{
				crc_cmp = 0;
				crc_ser = 0;
			}

			if(crc_cmp == crc_ser)
			{
				PushToStream(upper_ser, _peer);
			}
			else
			{
				assert(false);
			}
		}
		else
		{
			assert(false);
		}
		_ser.Close();
	}

	return true;
}

void TCPStream::PushToStream(SerializerLess& _ser, const Peer& _peer)
{
	for(listeners_t::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
	{
		Listener* listener = (*it);
		if(listener->PullFromStream(_ser, _peer))
		{
			break;
		}
	}
}

Channel& TCPStream::GetChannel(channels_t& channels, const Peer& _peer)
{
	channels_t::iterator it = channels.find(_peer);
	Channel *channel_out = nullptr;

	if(it != channels.end())
	{
		channel_out = (*it).second;
	}
	else
	{
		channel_out = new Channel(_peer);
		channels[_peer] = channel_out;
	}

	return *channel_out;
}

netBool TCPStream::AttachListener(Listener& _listener)
{
	netBool is_valid = true;
	// avoid double type
	for(listeners_t::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
	{
		Listener *listener = (*it);
		if(listener->GetType() == _listener.GetType())
		{
			is_valid = false;
			break;
		}
	}

	if(is_valid)
	{
		m_listeners.push_back(&_listener);
		_listener.RegisterStream(*this);
	}
	
	return is_valid;
}

netBool TCPStream::DetachListener(Listener& _listener)
{
	netBool is_valid = false;

	for(listeners_t::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
	{
		Listener *listener = (*it);
		if(listener->GetType() == _listener.GetType())
		{
			is_valid = true;
			m_listeners.erase(it);
			listener->UnRegisterStream(*this);
			break;
		}
	}

	return is_valid;
}

netBool TCPStream::Connect(const Peer& _peer)
{
	sockaddr_in addr = _peer.GetNativeStruct();

	SOCKET connection = socket(addr.sin_family, SOCK_STREAM, 0);
	assert(connection > 0);

	int result = connect(connection, (SOCKADDR*)&addr, sizeof(addr));
	
	if(result == 0)
	{
		if(connection > m_fdmax)
		{
			m_fdmax = connection;
		}
		m_connections[_peer] = new struct ConnectionPacker(connection);
	}

	return result == 0;
}

struct ConnectionPacker* TCPStream::GetOrCreateConnection(const Peer& _peer)
{
	struct ConnectionPacker* connection = GetConnection(_peer);

	if(connection == nullptr)
	{
		if(Connect(_peer))
		{
			connection = GetConnection(_peer);
		}
	}

	return connection;
}

struct ConnectionPacker* TCPStream::GetConnection(const Peer& _peer)
{
	struct ConnectionPacker* connection = nullptr;
	connections_t::iterator it = m_connections.find(_peer);

	if(it != m_connections.end())
	{
		connection = (it->second);
	}

	return connection;
}


};
