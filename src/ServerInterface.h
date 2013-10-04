#pragma once

#include "DispatcherInterface.h"

class ServerInterface : public DispatcherInterface
{
public:
	virtual void			accept( uint16_t port ) = 0;
	virtual void			cancel() = 0;

	template< typename T, typename Y >
	inline void				connectCancelEventHandler( T eventHandler, Y* eventHandlerObject )
	{
		connectCancelEventHandler( std::bind( eventHandler, eventHandlerObject ) );
	}
	void					connectCancelEventHandler( const std::function<void()>& eventHandler );
protected:
	ServerInterface( boost::asio::io_service& io );

	std::function<void()>	mCancelEventHandler;
};
	