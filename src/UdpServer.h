#pragma once

#include "ServerInterface.h"
#include "UdpSession.h"

typedef std::shared_ptr<class UdpServer>				UdpServerRef;
typedef std::shared_ptr<boost::asio::ip::udp::socket>	UdpSocketRef;

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
	
	virtual void	accept( uint16_t port );
	virtual void	cancel();
	
protected:
	UdpServer( boost::asio::io_service& io );

	void			read();
	void			process( size_t numBytes );
	void			onReceiveFrom( const boost::system::error_code& err, size_t bytesReceived );
	
	enum { kMaxLength = 1024 };
	
	UdpSocketRef					mSocket;
	uint8_t							mData[ kMaxLength ];
	boost::asio::ip::udp::endpoint	mSenderEndPoint;
};
	