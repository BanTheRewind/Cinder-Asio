#pragma once

#include "CinderAsio.h"

#include <string>
#include <functional>

class DispatcherInterface
{
public:
	~DispatcherInterface();

	template< typename T, typename Y >
	inline void	connectErrorEventHandler( T eventHandler, Y* eventHandlerObject )
	{
		connectErrorEventHandler( std::bind( eventHandler, eventHandlerObject, std::placeholders::_1, std::placeholders::_2 ) );
	}
	void		connectErrorEventHandler( const std::function<void( std::string, size_t )>& eventHandler );
protected:
	DispatcherInterface( asio::io_service& io );

	asio::io_service&			mIoService;
	asio::io_service::strand	mStrand;

	std::function<void( std::string, size_t )> mErrorEventHandler { nullptr };
};
