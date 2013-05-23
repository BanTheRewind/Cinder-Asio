#pragma once

#include "boost/asio.hpp"
#include "boost/bind.hpp"
#include "boost/signals2.hpp"

#include "cinder/Buffer.h"
#include <string>

class Connection
{
public:
	virtual void			disconnect() = 0;
	
	virtual void			read() = 0;
	virtual void			write( const ci::Buffer& buffer ) = 0;

	virtual void			wait( size_t millis, bool repeat );

	template<typename T, typename Y>
	inline uint32_t			addConnectCallback( T callback, Y *callbackObject )
	{
		uint32_t id = mCallbacks.empty() ? 0 : mCallbacks.rbegin()->first + 1;
		mCallbacks.insert( std::make_pair( id, CallbackRef( new Callback( mSignalConnect.connect( std::bind( callback, callbackObject ) ) ) ) ) );
		return id;
	}
	
	template<typename T, typename Y>
	inline uint32_t			addErrorCallback( T callback, Y *callbackObject )
	{
		uint32_t id = mCallbacks.empty() ? 0 : mCallbacks.rbegin()->first + 1;
		mCallbacks.insert( std::make_pair( id, CallbackRef( new Callback( mSignalError.connect( std::bind( callback, callbackObject, std::placeholders::_1, std::placeholders::_2 ) ) ) ) ) );
		return id;
	}
	
	template<typename T, typename Y>
	inline uint32_t			addReadCallback( T callback, Y *callbackObject )
	{
		uint32_t id = mCallbacks.empty() ? 0 : mCallbacks.rbegin()->first + 1;
		mCallbacks.insert( std::make_pair( id, CallbackRef( new Callback( mSignalRead.connect( std::bind( callback, callbackObject, std::placeholders::_1 ) ) ) ) ) );
		return id;
	}
	
	template<typename T, typename Y>
	inline uint32_t			addReadCompleteCallback( T callback, Y *callbackObject )
	{
		uint32_t id = mCallbacks.empty() ? 0 : mCallbacks.rbegin()->first + 1;
		mCallbacks.insert( std::make_pair( id, CallbackRef( new Callback( mSignalReadComplete.connect( std::bind( callback, callbackObject ) ) ) ) ) );
		return id;
	}
	
	template<typename T, typename Y>
	inline uint32_t			addResolveCallback( T callback, Y *callbackObject )
	{
		uint32_t id = mCallbacks.empty() ? 0 : mCallbacks.rbegin()->first + 1;
		mCallbacks.insert( std::make_pair( id, CallbackRef( new Callback( mSignalResolve.connect( std::bind( callback, callbackObject ) ) ) ) ) );
		return id;
	}

	template<typename T, typename Y>
	inline uint32_t			addWaitCallback( T callback, Y *callbackObject )
	{
		uint32_t id = mCallbacks.empty() ? 0 : mCallbacks.rbegin()->first + 1;
		mCallbacks.insert( std::make_pair( id, CallbackRef( new Callback( mSignalWait.connect( std::bind( callback, callbackObject ) ) ) ) ) );
		return id;
	}

	template<typename T, typename Y>
	inline uint32_t			addWriteCallback( T callback, Y *callbackObject )
	{
		uint32_t id = mCallbacks.empty() ? 0 : mCallbacks.rbegin()->first + 1;
		mCallbacks.insert( std::make_pair( id, CallbackRef( new Callback( mSignalWrite.connect( std::bind( callback, callbackObject, std::placeholders::_1 ) ) ) ) ) );
		return id;
	}
	
	void					removeCallback( uint32_t id );
protected:
	Connection( boost::asio::io_service& io );

	typedef boost::signals2::connection		Callback;
	typedef std::shared_ptr<Callback>		CallbackRef;
	typedef std::map<uint32_t, CallbackRef>	CallbackList;

	virtual void				onConnect( const boost::system::error_code& err ) = 0;
	virtual void				onRead( const boost::system::error_code& err,
									   size_t bytesTransferred );
	virtual void				onWait( const boost::system::error_code& err );
	virtual void				onWrite( const boost::system::error_code& err,
										size_t bytesTransferred );
	
	boost::asio::streambuf		mRequest;
	boost::asio::streambuf		mResponse;
	boost::asio::strand			mStrand;
	boost::asio::deadline_timer	mTimer;
	
	size_t						mTimerInterval;
	bool						mTimerRepeat;
	
	CallbackList											mCallbacks;
	boost::signals2::signal<void ()>						mSignalConnect;
	boost::signals2::signal<void ()>						mSignalResolve;
	boost::signals2::signal<void ( std::string, size_t )>	mSignalError;
	boost::signals2::signal<void ( ci::Buffer )>			mSignalRead;
	boost::signals2::signal<void ()>						mSignalReadComplete;
	boost::signals2::signal<void ()>						mSignalWait;
	boost::signals2::signal<void ( size_t )>				mSignalWrite;
};
