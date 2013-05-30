#pragma once

#include "SessionInterface.h"

typedef std::shared_ptr<class TcpSession>				TcpSessionRef;
typedef std::shared_ptr<boost::asio::ip::tcp::socket>	TcpSocketRef;

class TcpClient;
class TcpServer;

class TcpSession : public SessionInterface, public std::enable_shared_from_this<TcpSession>
{
public:
	static TcpSessionRef	create( boost::asio::io_service& io );
	~TcpSession();
	
	virtual void			close();
	
	virtual void			read();
	virtual void			read( const std::string& delim );
	virtual void			read( size_t bufferSize );
	
	virtual void			write( const ci::Buffer& buffer );

	const TcpSocketRef&		getSocket() const;
protected:
	TcpSession( boost::asio::io_service& io );

	TcpSocketRef			mSocket;

	friend class			TcpClient;
	friend class			TcpServer;
};
