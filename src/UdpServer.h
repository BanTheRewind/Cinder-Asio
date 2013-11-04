#pragma once

#include "ServerInterface.h"
#include "UdpSession.h"

typedef std::shared_ptr<class UdpServer> UdpServerRef;

class UdpServer : public ServerInterface, public std::enable_shared_from_this<UdpServer>
{
public:
	static UdpServerRef create( boost::asio::io_service& io );
	~UdpServer();

	template< typename T, typename Y >
	inline void			connectAcceptEventHandler( T eventHandler, Y* eventHandlerObject )
	{
		connectAcceptEventHandler( std::bind( eventHandler, eventHandlerObject, std::placeholders::_1 ) );
	}
	void				connectAcceptEventHandler( const std::function<void( UdpSessionRef )>& eventHandler );
	
	virtual void		accept( uint16_t port );
protected:
	UdpServer( boost::asio::io_service& io );

	std::function<void( UdpSessionRef )>	mAcceptEventHandler;
};
	