#pragma once

#include "DispatcherInterface.h"

class ClientInterface : public DispatcherInterface
{
public:
	virtual void		connect( const std::string& host, uint16_t port ) = 0;
	virtual void		connect( const std::string& host, const std::string& protocol ) = 0;

	template< typename T, typename Y >
	inline void			connectResolveEventHandler( T eventHandler, Y* eventHandlerObject )
	{
		mResolveEventHandler		= std::bind( eventHandler, eventHandlerObject );
	}

	void				connectResolveEventHandler( const std::function< void() >& eventHandler );

protected:
	ClientInterface( boost::asio::io_service& io );
	
	std::function< void() >			mResolveEventHandler;
};
