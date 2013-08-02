#include "ServerInterface.h"

ServerInterface::ServerInterface( boost::asio::io_service& io )
	: DispatcherInterface( io )
{
}

void ServerInterface::connectAcceptEventHandler( const std::function<void( std::shared_ptr<class SessionInterface> )>& eventHandler )
{
	mAcceptEventHandler = eventHandler;
}

void ServerInterface::connectCancelEventHandler( const std::function<void()>& eventHandler )
{
	mCancelEventHandler = eventHandler;
}