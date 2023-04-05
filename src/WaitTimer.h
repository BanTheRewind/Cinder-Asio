#pragma once

#include "DispatcherInterface.h"
#include "asio/basic_waitable_timer.hpp"

typedef std::shared_ptr<class WaitTimer> WaitTimerRef;

class WaitTimer : public DispatcherInterface, public std::enable_shared_from_this<WaitTimer>
{
public:
	static WaitTimerRef	create( asio::io_service& io );
	~WaitTimer();

	virtual void		wait( size_t millis, bool repeat );

	template< typename T, typename Y >
	inline void			connectWaitEventHandler( T eventHandler, Y* eventHandlerObject )
	{
		connectWaitEventHandler( std::bind( eventHandler, eventHandlerObject ) );
	}

	void				connectWaitEventHandler( const std::function<void ()>& eventHandler );
protected:
	WaitTimer( asio::io_service& io );

	virtual void		onWait( const asio::error_code& err );

	asio::basic_waitable_timer<std::chrono::high_resolution_clock> mTimer;
	
	size_t	mTimerInterval { 0 };
	bool	mTimerRepeat { false };

	std::function<void()>	mWaitEventHandler { nullptr };
};
