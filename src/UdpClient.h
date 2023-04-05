#pragma once

#include "ClientInterface.h"
#include "UdpSession.h"

typedef std::shared_ptr<class UdpClient>			UdpClientRef;
typedef std::shared_ptr<asio::ip::udp::resolver>	UdpResolverRef;

class UdpClient : public ClientInterface, public std::enable_shared_from_this<UdpClient>
{
public:
	static UdpClientRef	create( asio::io_service& io );
	~UdpClient();

	virtual void	connect( const std::string& host, uint16_t port );
	virtual void	connect( const std::string& host, const std::string& protocol );

	template< typename T, typename Y >
	inline void		connectConnectEventHandler( T eventHandler, Y* eventHandlerObject )
	{
		connectConnectEventHandler( std::bind( eventHandler, eventHandlerObject, std::placeholders::_1 ) );
	}

	void			connectConnectEventHandler( const std::function< void( UdpSessionRef ) >& eventHandler );
	
	UdpResolverRef	getResolver() const;
protected:
	UdpClient( asio::io_service& io );
	
	virtual void	onConnect( UdpSessionRef session, const asio::error_code& err );
	virtual void	onResolve( const asio::error_code& err, asio::ip::udp::resolver::iterator iter );

	UdpResolverRef	mResolver { nullptr };

	std::function< void( UdpSessionRef ) > mConnectEventHandler { nullptr };
};
