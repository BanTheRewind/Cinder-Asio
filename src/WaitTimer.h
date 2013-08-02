#pragma once

#include "DispatcherInterface.h"

typedef std::shared_ptr<class WaitTimer> WaitTimerRef;

class WaitTimer : public DispatcherInterface, public std::enable_shared_from_this<WaitTimer>
{
public:
	static WaitTimerRef					create( boost::asio::io_service& io );

	virtual void						wait( size_t millis, bool repeat );

	template< typename T, typename Y >
	inline void							connectWaitEventHandler( T eventHandler, Y* eventHandlerObject )
	{
		connectWaitEventHandler( std::bind( eventHandler, eventHandlerObject ) );
	}

	void								connectWaitEventHandler( const std::function<void()>& eventHandler );

protected:
	WaitTimer( boost::asio::io_service& io );

	boost::asio::deadline_timer			mTimer;
	size_t								mTimerInterval;
	bool								mTimerRepeat;

	virtual void						onWait( const boost::system::error_code& err );

	std::function<void()>				mWaitEventHandler;
};
