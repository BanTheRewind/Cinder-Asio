#include "WaitTimer.h"

WaitTimerRef WaitTimer::create( boost::asio::io_service& io )
{
	return WaitTimerRef( new WaitTimer( io ) )->shared_from_this();
}

WaitTimer::WaitTimer( boost::asio::io_service& io )
	: DispatcherInterface( io ), mStopped( false ),
mTimer( io ), mTimerInterval( 0 ), mTimerRepeat( false )
{
}

void WaitTimer::wait( size_t micros, bool repeat )
{
	mStopped		= false;
	mTimerInterval	= micros;
	mTimerRepeat	= repeat;
	if ( mTimerInterval > 0 ) {
		mTimer.expires_from_now( boost::posix_time::microsec( mTimerInterval ) );
		mTimer.async_wait( 
			mStrand.wrap( boost::bind( &WaitTimer::onWait, shared_from_this(), 
			boost::asio::placeholders::error ) ) );
	}
}

size_t WaitTimer::getInterval() const
{
	return mTimerInterval;
}

void WaitTimer::setInterval( size_t interval )
{
	mTimerInterval = interval;
}

bool WaitTimer::isRepeating() const
{
	return mTimerRepeat;
}

void WaitTimer::enableRepeat( bool repeat )
{
	mTimerRepeat = repeat;
}

void WaitTimer::stop()
{
	mStopped = true;
}

void WaitTimer::onWait( const boost::system::error_code& err )
{
	if ( err ) {
		mSignalError( err.message(), 0 );
	} else {
		if ( !mStopped ) {
			mSignalWait();
			if ( mTimerRepeat ) {
				wait( mTimerInterval, true );
			}
		}
	}
}
