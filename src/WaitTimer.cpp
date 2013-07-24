#include "WaitTimer.h"

WaitTimerRef WaitTimer::create( boost::asio::io_service& io )
{
	return WaitTimerRef( new WaitTimer( io ) )->shared_from_this();
}

WaitTimer::WaitTimer( boost::asio::io_service& io )
	: DispatcherInterface( io ), mTimer( io ), mTimerInterval( 0 ), mTimerRepeat( false )
{
}

void WaitTimer::wait( size_t millis, bool repeat )
{
	mTimerInterval	= millis;
	mTimerRepeat	= repeat;
	if ( mTimerInterval > 0 ) {
		mTimer.expires_from_now( boost::posix_time::milliseconds( mTimerInterval ) );
		mTimer.async_wait( 
			mStrand.wrap( boost::bind( &WaitTimer::onWait, shared_from_this(), 
			boost::asio::placeholders::error ) ) );
	}
}

void WaitTimer::onWait( const boost::system::error_code& err )
{
	if ( err ) {
		mErrorEventHandler( err.message(), 0 );
	} else {
		mWaitEventHandler();
		if ( mTimerRepeat ) {
			wait( mTimerInterval, true );
		}
	}
}

void WaitTimer::connectWaitEventHandler( const std::function< void() >& eventHandler )
{
	mWaitEventHandler				= eventHandler;
}
