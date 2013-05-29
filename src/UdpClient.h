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

	template<typename T, typename Y>
	inline uint32_t		addConnectCallback( T callback, Y* callbackObject )
	{
		uint32_t id = mCallbacks.empty() ? 0 : mCallbacks.rbegin()->first + 1;
		mCallbacks.insert( std::make_pair( id, CallbackRef( new Callback( mSignalConnect.connect( std::bind( callback, callbackObject, std::placeholders::_1 ) ) ) ) ) );
		return id;
	}
protected:
	typedef std::shared_ptr<boost::asio::ip::udp::resolver>	UdpResolverRef;

	UdpClient( boost::asio::io_service& io );
	
	virtual void	onConnect( UdpSessionRef session, const boost::system::error_code& err );
	virtual void	onResolve( const boost::system::error_code& err, 
							  boost::asio::ip::udp::resolver::iterator iter );

	UdpResolverRef	mResolver;

	boost::signals2::signal<void ( UdpSessionRef )>	mSignalConnect;
};
