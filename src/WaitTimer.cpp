/*
* 
* Copyright (c) 2014, Wieden+Kennedy, 
* Stephen Schieberl, Michael Latzoni
* All rights reserved.
* 
* Redistribution and use in source and binary forms, with or 
* without modification, are permitted provided that the following 
* conditions are met:
* 
* Redistributions of source code must retain the above copyright 
* notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright 
* notice, this list of conditions and the following disclaimer in 
* the documentation and/or other materials provided with the 
* distribution.
* 
* Neither the name of the Ban the Rewind nor the names of its 
* contributors may be used to endorse or promote products 
* derived from this software without specific prior written 
* permission.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
* COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* 
*/

#include "WaitTimer.h"

WaitTimerRef WaitTimer::create( boost::asio::io_service& io )
{
	return WaitTimerRef( new WaitTimer( io ) )->shared_from_this();
}

WaitTimer::WaitTimer( boost::asio::io_service& io )
	: DispatcherInterface( io ), mTimer( io ), mTimerInterval( 0 ), mTimerRepeat( false ), 
	mWaitEventHandler( nullptr )
{
}

WaitTimer::~WaitTimer()
{
	mWaitEventHandler = nullptr;
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

void WaitTimer::connectWaitEventHandler( const std::function<void()>& eventHandler )
{
	mWaitEventHandler = eventHandler;
}
