#include "ServerInterface.h"

ServerInterface::ServerInterface( boost::asio::io_service& io )
	: DispatcherInterface( io ), mCancelEventHandler( nullptr )
{
}

void ServerInterface::connectCancelEventHandler( const std::function<void()>& eventHandler )
{
	mCancelEventHandler = eventHandler;
}