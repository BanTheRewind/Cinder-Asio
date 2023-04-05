#pragma once

#include "SessionInterface.h"

typedef std::shared_ptr<class UdpSession>		UdpSessionRef;
typedef std::shared_ptr<asio::ip::udp::socket>	UdpSocketRef;

class UdpClient;
class UdpServer;

class UdpSession : public SessionInterface, public std::enable_shared_from_this<UdpSession>
{
public:
	static UdpSessionRef			create( asio::io_service& io );
	~UdpSession();
	
	virtual void					read();
	virtual void					read( size_t bufferSize );
	virtual void					write( const ci::BufferRef& buffer );
	virtual void					write( const std::string& str );

	const asio::ip::udp::endpoint&	getLocalEndpoint() const;
	const asio::ip::udp::endpoint&	getRemoteEndpoint() const;
	const UdpSocketRef&				getSocket() const;
protected:
	UdpSession( asio::io_service& io );

	asio::ip::udp::endpoint	mEndpointLocal;
	asio::ip::udp::endpoint	mEndpointRemote;
	UdpSocketRef			mSocket;

	friend class					UdpClient;
	friend class					UdpServer;
};
