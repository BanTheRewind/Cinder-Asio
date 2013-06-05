#pragma once

#include "DispatcherInterface.h"

typedef std::shared_ptr<class WaitTimer> WaitTimerRef;

class WaitTimer : public DispatcherInterface, public std::enable_shared_from_this<WaitTimer>
{
public:
	static WaitTimerRef					create( boost::asio::io_service& io );

	//! Tells timer to wait \a interval microseconds. Set \a repeat to true to repeat
	virtual void						wait( size_t interval, bool repeat = false );

	virtual size_t						getInterval() const;
	virtual void						setInterval( size_t interval );
	
	virtual bool						isRepeating() const;
	virtual void						enableRepeat( bool repeat );
	
	virtual void						stop();
	
	template<typename T, typename Y>
	inline uint32_t						addWaitCallback( T callback, Y* callbackObject )
	{
		uint32_t id = mCallbacks.empty() ? 0 : mCallbacks.rbegin()->first + 1;
		mCallbacks.insert( std::make_pair( id, CallbackRef( new Callback( mSignalWait.connect( std::bind( callback, callbackObject ) ) ) ) ) );
		return id;
	}
protected:
	WaitTimer( boost::asio::io_service& io );

	bool								mStopped;
	boost::asio::deadline_timer			mTimer;
	size_t								mTimerInterval;
	bool								mTimerRepeat;

	virtual void						onWait( const boost::system::error_code& err );

	boost::signals2::signal<void ()>	mSignalWait;
};
