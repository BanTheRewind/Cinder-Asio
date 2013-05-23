#pragma once

#include "Client.h"

typedef std::shared_ptr<class UdpClient>				UdpClientRef;
typedef std::shared_ptr<boost::asio::ip::udp::resolver>	UdpResolverRef;
typedef std::shared_ptr<boost::asio::ip::udp::socket>	UdpSocketRef;

class UdpClient : public Client, public std::enable_shared_from_this<UdpClient>
{
public:
	static UdpClientRef	create( boost::asio::io_service& io );
	
	virtual void		connect( const std::string& host, uint16_t port );
	virtual void		connect( const std::string& host, const std::string& protocol );
	virtual void		disconnect();
	
	virtual void		read();

	virtual void		wait( size_t millis, bool repeat = false );

	virtual void		write( const ci::Buffer& buffer );
protected:
	UdpClient( boost::asio::io_service& io );
	
	virtual void		onConnect( const boost::system::error_code& err );
	virtual void		onResolve( const boost::system::error_code& err,
								  boost::asio::ip::udp::resolver::iterator iter );

	UdpResolverRef		mResolver;
	UdpSocketRef		mSocket;
};
