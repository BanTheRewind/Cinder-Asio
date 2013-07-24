#pragma once

#include "boost/asio.hpp"
#include "boost/bind.hpp"
#include "boost/signals2.hpp"

#include <string>
#include <functional>

class DispatcherInterface
{
public:
	~DispatcherInterface();

	template< typename T, typename Y >
	inline void		connectErrorEventHandler( T eventHandler, Y* eventHandlerObject )
	{
		mErrorEventHandler	= std::bind( eventHandler, eventHandlerObject, std::placeholders::_1, std::placeholders::_2 );
	}

	void			connectErrorEventHandler( const std::function< void( std::string, size_t ) >& eventHandler );

protected:
	DispatcherInterface( boost::asio::io_service& io );

	boost::asio::io_service&							mIoService;
	boost::asio::strand									mStrand;

	std::function< void( std::string, size_t ) >		mErrorEventHandler;
};
