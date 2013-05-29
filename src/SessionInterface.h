#pragma once

#include "DispatcherInterface.h"
#include "cinder/Buffer.h"

class SessionInterface : public DispatcherInterface
{
public:
	static std::string		bufferToString( const ci::Buffer& buffer );
	static ci::Buffer		stringToBuffer( std::string& value );

	~SessionInterface();

	virtual void			close() = 0;

	virtual void			read() = 0;
	virtual void			write( const ci::Buffer& buffer ) = 0;	

	template<typename T, typename Y>
	inline uint32_t			addCloseCallback( T callback, Y* callbackObject )
	{
		uint32_t id = mCallbacks.empty() ? 0 : mCallbacks.rbegin()->first + 1;
		mCallbacks.insert( std::make_pair( id, CallbackRef( new Callback( mSignalClose.connect( std::bind( callback, callbackObject ) ) ) ) ) );
		return id;
	}

	template<typename T, typename Y>
	inline uint32_t			addReadCallback( T callback, Y* callbackObject )
	{
		uint32_t id = mCallbacks.empty() ? 0 : mCallbacks.rbegin()->first + 1;
		mCallbacks.insert( std::make_pair( id, CallbackRef( new Callback( mSignalRead.connect( std::bind( callback, callbackObject, std::placeholders::_1 ) ) ) ) ) );
		return id;
	}
	
	template<typename T, typename Y>
	inline uint32_t			addReadCompleteCallback( T callback, Y* callbackObject )
	{
		uint32_t id = mCallbacks.empty() ? 0 : mCallbacks.rbegin()->first + 1;
		mCallbacks.insert( std::make_pair( id, CallbackRef( new Callback( mSignalReadComplete.connect( std::bind( callback, callbackObject ) ) ) ) ) );
		return id;
	}

	template<typename T, typename Y>
	inline uint32_t			addWriteCallback( T callback, Y* callbackObject )
	{
		uint32_t id = mCallbacks.empty() ? 0 : mCallbacks.rbegin()->first + 1;
		mCallbacks.insert( std::make_pair( id, CallbackRef( new Callback( mSignalWrite.connect( std::bind( callback, callbackObject, std::placeholders::_1 ) ) ) ) ) );
		return id;
	}
protected:
	SessionInterface( boost::asio::io_service& io );

	virtual void			onClose( const boost::system::error_code& err );
	virtual void			onRead( const boost::system::error_code& err,
								   size_t bytesTransferred );
	virtual void			onWrite( const boost::system::error_code& err,
									size_t bytesTransferred );
	
	boost::asio::streambuf	mRequest;
	boost::asio::streambuf	mResponse;
	
	boost::signals2::signal<void ()>				mSignalClose;
	boost::signals2::signal<void ( ci::Buffer )>	mSignalRead;
	boost::signals2::signal<void ()>				mSignalReadComplete;
	boost::signals2::signal<void ( size_t )>		mSignalWrite;
};
