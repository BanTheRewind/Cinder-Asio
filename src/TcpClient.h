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

	template<typename T, typename Y>
	inline uint32_t		addConnectCallback( T callback, Y* callbackObject )
	{
		uint32_t id = mCallbacks.empty() ? 0 : mCallbacks.rbegin()->first + 1;
		mCallbacks.insert( std::make_pair( id, CallbackRef( new Callback( mSignalConnect.connect( std::bind( callback, callbackObject, std::placeholders::_1 ) ) ) ) ) );
		return id;
	}
protected:
	typedef std::shared_ptr<boost::asio::ip::tcp::resolver>	TcpResolverRef;

	TcpClient( boost::asio::io_service& io );
	
	virtual void	onConnect( TcpSessionRef session, const boost::system::error_code& err );
	virtual void	onResolve( const boost::system::error_code& err, 
							  boost::asio::ip::tcp::resolver::iterator iter );

	TcpResolverRef	mResolver;

	boost::signals2::signal<void ( TcpSessionRef )>	mSignalConnect;
};
