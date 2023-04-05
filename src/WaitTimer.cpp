#include "WaitTimer.h"

WaitTimerRef WaitTimer::create( asio::io_service& io )
{
	return WaitTimerRef { new WaitTimer { io } }->shared_from_this();
}

WaitTimer::WaitTimer( asio::io_service& io )
	: DispatcherInterface( io ), mTimer( io )
{
}

WaitTimer::~WaitTimer()
{
	mWaitEventHandler = nullptr;
}

void WaitTimer::wait( size_t millis, bool repeat )
{
	mTimerInterval = millis;
	mTimerRepeat = repeat;
	if ( mTimerInterval > 0 ) {
		mTimer.expires_from_now( std::chrono::milliseconds( mTimerInterval ) );
		mTimer.async_wait( 
			mStrand.wrap( std::bind( &WaitTimer::onWait, shared_from_this(),
			std::placeholders::_1 ) ) );
	}
}

void WaitTimer::onWait( const asio::error_code& err )
{
	if ( err ) {
		if ( mErrorEventHandler != nullptr ) {
			mErrorEventHandler( err.message(), 0 );
		}
	} else {
		if ( mWaitEventHandler != 0 ) {
			mWaitEventHandler();
		}
		if ( mTimerRepeat ) {
			wait( mTimerInterval, true );
		}
	}
}

void WaitTimer::connectWaitEventHandler( const std::function<void ()>& eventHandler )
{
	mWaitEventHandler = eventHandler;
}
