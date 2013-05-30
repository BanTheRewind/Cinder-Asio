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
	
	template<typename T, typename Y>
	inline uint32_t		addAcceptCallback( T callback, Y* callbackObject )
	{
		uint32_t id = mCallbacks.empty() ? 0 : mCallbacks.rbegin()->first + 1;
		mCallbacks.insert( std::make_pair( id, CallbackRef( new Callback( mSignalAccept.connect( std::bind( callback, callbackObject, std::placeholders::_1 ) ) ) ) ) );
		return id;
	}
protected:
	typedef std::shared_ptr<boost::asio::ip::tcp::acceptor>	TcpAcceptorRef;

	TcpServer( boost::asio::io_service& io );

	void				onAccept( TcpSessionRef session, const boost::system::error_code& err );

	TcpAcceptorRef		mAcceptor;
	boost::signals2::signal<void ( TcpSessionRef )>	mSignalAccept;
};
