#include "DispatcherInterface.h"

DispatcherInterface::DispatcherInterface( boost::asio::io_service& io )
	: mIoService( io ), mStrand( io )
{
}

DispatcherInterface::~DispatcherInterface()
{
	
}

void DispatcherInterface::connectErrorEventHandler( const std::function< void( std::string, size_t ) >& eventHandler )
{
	mErrorEventHandler	= eventHandler;
}
