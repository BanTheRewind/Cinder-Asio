#pragma once

#include "ClientInterface.h"
#include "UdpSession.h"

typedef std::shared_ptr<class UdpClient> UdpClientRef;

class UdpClient : public ClientInterface, public std::enable_shared_from_this<UdpClient>
{
public:
	static UdpClientRef	create( boost::asio::io_service& io );

	virtual void	connect( const std::string& host, uint16_t port );
	virtual void	connect( const std::string& host, const std::string& protocol );

	template< typename T, typename Y >
	inline void		connectConnectEventHandler( T eventHandler, Y* eventHandlerObject )
	{
		mConnectEventHandler		= std::bind( eventHandler, eventHandlerObject, std::placeholders::_1 );
	}

	void			connectConnectEventHandler( const std::function< void( UdpSessionRef ) >& eventHandler );

protected:
	typedef std::shared_ptr<boost::asio::ip::udp::resolver>	UdpResolverRef;

	UdpClient( boost::asio::io_service& io );
	
	virtual void	onConnect( UdpSessionRef session, const boost::system::error_code& err );
	virtual void	onResolve( const boost::system::error_code& err, 
							  boost::asio::ip::udp::resolver::iterator iter );

	UdpResolverRef	mResolver;

	std::function< void( UdpSessionRef ) >		mConnectEventHandler;
};
