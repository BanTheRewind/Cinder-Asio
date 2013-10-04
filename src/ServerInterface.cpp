#include "ServerInterface.h"

ServerInterface::ServerInterface( boost::asio::io_service& io )
	: DispatcherInterface( io )
{
}

void ServerInterface::connectCancelEventHandler( const std::function<void()>& eventHandler )
{
	mCancelEventHandler = eventHandler;
}