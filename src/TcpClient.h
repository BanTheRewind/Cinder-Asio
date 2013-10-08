#pragma once

#include "ClientInterface.h"
#include "TcpSession.h"

typedef std::shared_ptr<class TcpClient> TcpClientRef;

class TcpClient : public ClientInterface, public std::enable_shared_from_this<TcpClient>
{
public:
	static TcpClientRef	create( boost::asio::io_service& io );

	virtual void	connect( const std::string& host, uint16_t port );
	virtual void	connect( const std::string& host, const std::string& protocol );

	template< typename T, typename Y >
	inline void		connectConnectEventHandler( T eventHandler, Y* eventHandlerObject )
	{
		connectConnectEventHandler( std::bind( eventHandler, eventHandlerObject, std::placeholders::_1 ) );
	}

	void			connectConnectEventHandler( const std::function<void( TcpSessionRef )>& eventHandler );
protected:
	typedef std::shared_ptr<boost::asio::ip::tcp::resolver>	TcpResolverRef;

	TcpClient( boost::asio::io_service& io );
	
	virtual void	onConnect( TcpSessionRef session, const boost::system::error_code& err );
	virtual void	onResolve( const boost::system::error_code& err, 
							  boost::asio::ip::tcp::resolver::iterator iter );

	TcpResolverRef	mResolver;

	std::function<void( TcpSessionRef )>	mConnectEventHandler;
};
