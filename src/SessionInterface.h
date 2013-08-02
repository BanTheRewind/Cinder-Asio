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

	template< typename T, typename Y >
	inline void				connectCloseEventHandler( T eventHandler, Y* eventHandlerObject )
	{
		connectCloseEventHandler( std::bind( eventHandler, eventHandlerObject ) );
	}
	void					connectCloseEventHandler( const std::function<void ()>& eventHandler );

	template< typename T, typename Y >
	inline void				connectReadEventHandler( T eventHandler, Y* eventHandlerObject )
	{
		connectReadEventHandler( std::bind( eventHandler, eventHandlerObject, std::placeholders::_1 ) );
	}
	void					connectReadEventHandler( const std::function<void( ci::Buffer )>& eventHandler );

	template< typename T, typename Y >
	inline void				connectReadCompleteEventHandler( T eventHandler, Y* eventHandlerObject )
	{
		connectReadCompleteEventHandler( std::bind( eventHandler, eventHandlerObject ) );
	}
	void					connectReadCompleteEventHandler( const std::function<void ()>& eventHandler );

	template< typename T, typename Y >
	inline void				connectWriteEventHandler( T eventHandler, Y* eventHandlerObject )
	{
		connectWriteEventHandler( std::bind( eventHandler, eventHandlerObject, std::placeholders::_1 ) );
	}
	void					connectWriteEventHandler( const std::function<void( size_t )>& eventHandler );
protected:
	SessionInterface( boost::asio::io_service& io );

	virtual void			onClose( const boost::system::error_code& err );
	virtual void			onRead( const boost::system::error_code& err,
								   size_t bytesTransferred );
	virtual void			onWrite( const boost::system::error_code& err,
									size_t bytesTransferred );
	
	boost::asio::streambuf	mRequest;
	boost::asio::streambuf	mResponse;

	std::function<void()>				mCloseEventHandler;
	std::function<void( ci::Buffer )>	mReadEventHandler;
	std::function<void()>				mReadCompleteEventHandler;
	std::function<void( size_t )>		mWriteEventHandler;
};
