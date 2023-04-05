#include "DispatcherInterface.h"

using namespace std;

DispatcherInterface::DispatcherInterface( asio::io_service& io )
	: mIoService( io ), mStrand( io )
{
}

DispatcherInterface::~DispatcherInterface()
{
	mErrorEventHandler = nullptr;
}

void DispatcherInterface::connectErrorEventHandler( const std::function<void( string, size_t )>& eventHandler )
{
	mErrorEventHandler = eventHandler;
}
