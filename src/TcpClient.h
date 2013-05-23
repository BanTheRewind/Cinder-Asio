#pragma once

#include "Client.h"

typedef std::shared_ptr<class TcpClient>				TcpClientRef;
typedef std::shared_ptr<boost::asio::ip::tcp::resolver>	TcpResolverRef;
typedef std::shared_ptr<boost::asio::ip::tcp::socket>	TcpSocketRef;

class TcpClient : public Client, public std::enable_shared_from_this<TcpClient>
{
public:
	static TcpClientRef	create( boost::asio::io_service& io );
	~TcpClient();
	
	virtual void		connect( const std::string& host, uint16_t port );
	virtual void		connect( const std::string& host, const std::string& protocol );
	virtual void		disconnect();
	
	virtual void		read();
	virtual void		read( const std::string& delim );
	virtual void		read( size_t bufferSize );
	
	virtual void		wait( size_t millis, bool repeat = false );

	virtual void		write( const ci::Buffer& buffer );
protected:
	TcpClient( boost::asio::io_service& io );
	
	virtual void		onConnect( const boost::system::error_code& err );
	virtual void		onResolve( const boost::system::error_code& err,
								  boost::asio::ip::tcp::resolver::iterator iter );

	TcpResolverRef		mResolver;
	TcpSocketRef		mSocket;
};
