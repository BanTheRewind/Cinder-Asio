#include "SessionInterface.h"

using namespace ci;
using namespace std;

string SessionInterface::bufferToString( const Buffer& buffer )
{
	return string( static_cast<const char*>( buffer.getData() ) );
}

Buffer SessionInterface::stringToBuffer( string& value )
{
	return Buffer( &value[ 0 ], value.size() );
}

SessionInterface::SessionInterface( boost::asio::io_service& io )
: DispatcherInterface( io )
{
}

SessionInterface::~SessionInterface()
{
	mRequest.consume( mRequest.size() );
	mResponse.consume( mResponse.size() );
}

void SessionInterface::onClose( const boost::system::error_code& err )
{
	if ( err ) {
		mErrorEventHandler( err.message(), 0 );
	} else {
		mCloseEventHandler();
	}
}


void SessionInterface::onRead( const boost::system::error_code& err, size_t bytesTransferred )
{
	if ( err ) {
		if ( err == boost::asio::error::eof ) {
			mReadCompleteEventHandler();
		} else {
			mErrorEventHandler( err.message(), bytesTransferred );
		}
	} else {
		char* data = new char[ bytesTransferred + 1 ];
		data[ bytesTransferred ] = 0;
		istream stream( &mResponse );
		stream.read( data, bytesTransferred );
		mReadEventHandler( Buffer( data, bytesTransferred ) );
		delete [] data;
	}
	mResponse.consume( mResponse.size() );
}

void SessionInterface::onWrite( const boost::system::error_code& err, size_t bytesTransferred )
{
	if ( err ) {
		mErrorEventHandler( err.message(), bytesTransferred );
	} else {
		mWriteEventHandler( bytesTransferred );
	}
}

void SessionInterface::connectCloseEventHandler( const std::function< void() >& eventHandler )
{
	mCloseEventHandler				= eventHandler;
}

void SessionInterface::connectReadEventHandler( const std::function< void( ci::Buffer ) >& eventHandler )
{
	mReadEventHandler				= eventHandler;
}

void SessionInterface::connectReadCompleteEventHandler( const std::function< void() >& eventHandler )
{
	mReadCompleteEventHandler		= eventHandler;
}

void SessionInterface::connectWriteEventHandler( const std::function< void( size_t ) >& eventHandler )
{
	mWriteEventHandler				= eventHandler;
}