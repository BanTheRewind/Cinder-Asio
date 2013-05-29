#pragma once

#include "ServerInterface.h"
#include "TcpSession.h"

typedef std::shared_ptr<class TcpServer> TcpServerRef;

class TcpServer : public ServerInterface, public std::enable_shared_from_this<TcpServer>
{
public:
	static TcpServerRef	create( boost::asio::io_service& io );
	~TcpServer();

	virtual void		accept( uint16_t port );
	virtual void		cancel();
protected:
	typedef std::shared_ptr<boost::asio::ip::tcp::acceptor>	TcpAcceptorRef;

	TcpServer( boost::asio::io_service& io );

	void				onAccept( TcpSessionRef session, const boost::system::error_code& err );

	TcpAcceptorRef		mAcceptor;
};
