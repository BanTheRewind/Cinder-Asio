#pragma once

#ifdef WIN32
    #include "sdkddkver.h"
#endif
#include "boost/asio.hpp"
#include "boost/signals2.hpp"
#include "cinder/Buffer.h"

class Client
{
private:
	typedef boost::signals2::connection		Callback;
	typedef std::shared_ptr<Callback>		CallbackRef;
	typedef std::map<uint32_t, CallbackRef>	CallbackList;
public:
	explicit Client();
	
	//! Connects to host
	virtual void	connect( const std::string &host, uint16_t port ) = 0;
	//! Returns true if connected
	bool			isConnected() const;

	//! Adds callback with signature is void( ci::Buffer ) and returns ID.
	template<typename T, typename Y> 
	inline uint32_t	addCallback( T callback, Y *callbackObject )
	{
		uint32_t id = mCallbacks.empty() ? 0 : mCallbacks.rbegin()->first + 1;
		mCallbacks.insert( std::make_pair( id, CallbackRef( new Callback( mSignal.connect( std::bind( callback, callbackObject, std::_1 ) ) ) ) ) );
		return id;
	}
protected:
	boost::signals2::signal<void ( ci::Buffer )>	mSignal;

	bool			mConnected;
};
