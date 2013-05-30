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
		mSignalError( err.message(), 0 );
	} else {
		mSignalClose();
	}
}

void SessionInterface::onRead( const boost::system::error_code& err, size_t bytesTransferred )
{
	if ( err ) {
		if ( err == boost::asio::error::eof ) {
			mSignalReadComplete();
		} else {
			mSignalError( err.message(), bytesTransferred );
		}
	} else {
		char* data = new char[ bytesTransferred + 1 ];
		data[ bytesTransferred ] = 0;
		istream stream( &mResponse );
		stream.read( data, bytesTransferred );
		mSignalRead( Buffer( data, bytesTransferred ) );
		delete [] data;
	}
	mResponse.consume( mResponse.size() );
}

void SessionInterface::onWrite( const boost::system::error_code& err, size_t bytesTransferred )
{
	if ( err ) {
		mSignalError( err.message(), bytesTransferred );
	} else {
		mSignalWrite( bytesTransferred );
	}
}
