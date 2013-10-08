#include "ClientInterface.h"

ClientInterface::ClientInterface( boost::asio::io_service& io )
	: DispatcherInterface( io ), mResolveEventHandler( nullptr )
{
}

void ClientInterface::connectResolveEventHandler( const std::function<void()>& eventHandler )
{
	mResolveEventHandler = eventHandler;
}
