#pragma once

#include "DispatcherInterface.h"

class ClientInterface : public DispatcherInterface
{
public:
	~ClientInterface();

	virtual void	connect( const std::string& host, uint16_t port ) = 0;
	virtual void	connect( const std::string& host, const std::string& protocol ) = 0;

	template< typename T, typename Y >
	inline void		connectResolveEventHandler( T eventHandler, Y* eventHandlerObject )
	{
		connectResolveEventHandler( std::bind( eventHandler, eventHandlerObject ) );
	}

	void			connectResolveEventHandler( const std::function<void ()>& eventHandler );

protected:
	ClientInterface( asio::io_service& io );
	
	std::function<void()> mResolveEventHandler { nullptr };
};
