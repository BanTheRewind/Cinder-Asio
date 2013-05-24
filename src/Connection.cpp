#include "Connection.h"

#include "cinder/app/App.h"
#include "cinder/Utilities.h"

using namespace ci;
using namespace std;

Connection::Connection( boost::asio::io_service& io )
: mStrand( io ), mTimer( io ), mTimerInterval( 0 ), mTimerRepeat( false )
{
}

void Connection::wait( size_t millis, bool repeat )
{
	mTimerInterval	= millis;
	mTimerRepeat	= repeat;
	if ( mTimerInterval > 0 ) {
		mTimer.expires_from_now( boost::posix_time::milliseconds( mTimerInterval ) );
	}
}

void Connection::removeCallback( uint32_t id )
{
	if ( mCallbacks.find( id ) != mCallbacks.end() ) {
		mCallbacks.find( id )->second->disconnect();
		mCallbacks.erase( id );
	}
}

void Connection::onDisconnect( const boost::system::error_code& err )
{
	if ( err ) {
		mSignalError( err.message(), 0 );
	} else {
		mSignalDisconnect();
	}
}

void Connection::onRead( const boost::system::error_code& err, size_t bytesTransferred )
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

void Connection::onWait( const boost::system::error_code& err )
{
	if ( err ) {
		mSignalError( err.message(), 0 );
	} else {
		mSignalWait();
		if ( mTimerRepeat ) {
			wait( mTimerInterval, true );
		}
	}
}

void Connection::onWrite( const boost::system::error_code& err, size_t bytesTransferred )
{
	if ( err ) {
		mSignalError( err.message(), bytesTransferred );
	} else {
		mSignalWrite( bytesTransferred );
	}
}
