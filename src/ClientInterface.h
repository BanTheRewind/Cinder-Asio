#pragma once

#include "DispatcherInterface.h"

class ClientInterface : public DispatcherInterface
{
public:
	virtual void		connect( const std::string& host, uint16_t port ) = 0;
	virtual void		connect( const std::string& host, const std::string& protocol ) = 0;

	template<typename T, typename Y>
	inline uint32_t		addResolveCallback( T callback, Y* callbackObject )
	{
		uint32_t id = mCallbacks.empty() ? 0 : mCallbacks.rbegin()->first + 1;
		mCallbacks.insert( std::make_pair( id, CallbackRef( new Callback( mSignalResolve.connect( std::bind( callback, callbackObject ) ) ) ) ) );
		return id;
	}
protected:
	ClientInterface( boost::asio::io_service& io );
	
	boost::signals2::signal<void ()>	mSignalResolve;
};
