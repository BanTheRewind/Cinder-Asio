#pragma once

#include "SessionInterface.h"

class ServerInterface : public DispatcherInterface
{
public:
	virtual void		accept( uint16_t port ) = 0;
	virtual void		cancel() = 0;

	template< typename T, typename Y >
	inline void			connectAcceptEventHandler( T eventHandler, Y* eventHandlerObject )
	{
		connectAcceptEventHandler( std::bind( eventHandler, eventHandlerObject, std::placeholders::_1 ) );
	}
	void				connectAcceptEventHandler( const std::function<void( std::shared_ptr<class SessionInterface> )>& eventHandler );

	template< typename T, typename Y >
	inline void			connectCancelEventHandler( T eventHandler, Y* eventHandlerObject )
	{
		connectCancelEventHandler( std::bind( eventHandler, eventHandlerObject, std::placeholders::_1 ) );
	}
	void				connectCancelEventHandler( const std::function<void()>& eventHandler );
protected:
	ServerInterface( boost::asio::io_service& io );

	std::function<void( std::shared_ptr<class SessionInterface> )>	mAcceptEventHandler;
	std::function<void()>											mCancelEventHandler;
};
