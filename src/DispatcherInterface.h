#pragma once

#include "boost/asio.hpp"
#include "boost/bind.hpp"
#include "boost/signals2.hpp"

#include <string>

class DispatcherInterface
{
public:
	~DispatcherInterface();

	template<typename T, typename Y>
	inline uint32_t	addErrorCallback( T callback, Y* callbackObject )
	{
		uint32_t id = mCallbacks.empty() ? 0 : mCallbacks.rbegin()->first + 1;
		mCallbacks.insert( std::make_pair( id, CallbackRef( new Callback( mSignalError.connect( std::bind( callback, callbackObject, std::placeholders::_1, std::placeholders::_2 ) ) ) ) ) );
		return id;
	}

	void			removeCallback( uint32_t id );
protected:
	typedef boost::signals2::connection		Callback;
	typedef std::shared_ptr<Callback>		CallbackRef;
	typedef std::map<uint32_t, CallbackRef>	CallbackList;

	DispatcherInterface( boost::asio::io_service& io );

	CallbackList											mCallbacks;
	boost::asio::io_service&								mIoService;
	boost::signals2::signal<void ( std::string, size_t )>	mSignalError;
	boost::asio::strand										mStrand;
};
