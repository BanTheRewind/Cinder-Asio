#pragma once

#include "SessionInterface.h"

typedef std::shared_ptr<class UdpSession>				UdpSessionRef;
typedef std::shared_ptr<boost::asio::ip::udp::socket>	UdpSocketRef;

class UdpClient;
class UdpServer;

class UdpSession : public SessionInterface, public std::enable_shared_from_this<UdpSession>
{
public:
	static UdpSessionRef	create( boost::asio::io_service& io );
	~UdpSession();
	
	virtual void			close();

	virtual void			read();
	virtual void			read( size_t bufferSize );
	virtual void			write( const ci::Buffer& buffer );

	const UdpSocketRef&		getSocket() const;
protected:
	UdpSession( boost::asio::io_service& io );

	UdpSocketRef			mSocket;

	friend class			UdpClient;
	friend class			UdpServer;
};
