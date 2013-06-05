#pragma once

#include "SessionInterface.h"

class ServerInterface : public DispatcherInterface
{
public:
	virtual void		accept( uint16_t port ) = 0;
	virtual void		cancel() = 0;
	virtual void		close() = 0;

	template<typename T, typename Y>
	inline uint32_t		addCancelCallback( T callback, Y* callbackObject )
	{
		uint32_t id = mCallbacks.empty() ? 0 : mCallbacks.rbegin()->first + 1;
		mCallbacks.insert( std::make_pair( id, CallbackRef( new Callback( mSignalCancel.connect( std::bind( callback, callbackObject ) ) ) ) ) );
		return id;
	}
	
	template<typename T, typename Y>
	inline uint32_t		addCloseCallback( T callback, Y* callbackObject )
	{
		uint32_t id = mCallbacks.empty() ? 0 : mCallbacks.rbegin()->first + 1;
		mCallbacks.insert( std::make_pair( id, CallbackRef( new Callback( mSignalClose.connect( std::bind( callback, callbackObject ) ) ) ) ) );
		return id;
	}
protected:
	ServerInterface( boost::asio::io_service& io );

	boost::signals2::signal<void ( std::shared_ptr<class SessionInterface> )>	mSignalAccept;
	boost::signals2::signal<void ()>	mSignalCancel;
	boost::signals2::signal<void ()>	mSignalClose;
};
