#pragma once

#include "ServerInterface.h"
#include "TcpSession.h"

typedef std::shared_ptr<class TcpServer> TcpServerRef;

class TcpServer : public ServerInterface, public std::enable_shared_from_this<TcpServer>
{
public:
	static TcpServerRef	create( boost::asio::io_service& io );
	~TcpServer();
	
	template< typename T, typename Y >
	inline void			connectAcceptEventHandler( T eventHandler, Y* eventHandlerObject )
	{
		connectAcceptEventHandler( std::bind( eventHandler, eventHandlerObject, std::placeholders::_1 ) );
	}
	void				connectAcceptEventHandler( const std::function<void( TcpSessionRef )>& eventHandler );

	virtual void		accept( uint16_t port );
	virtual void		cancel();
protected:
	typedef std::shared_ptr<boost::asio::ip::tcp::acceptor>	TcpAcceptorRef;

	TcpServer( boost::asio::io_service& io );

	void				onAccept( TcpSessionRef session, const boost::system::error_code& err );
	
	TcpAcceptorRef		mAcceptor;
	std::function<void( TcpSessionRef )>		mAcceptEventHandler;
};
