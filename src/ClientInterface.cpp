#include "ClientInterface.h"

ClientInterface::ClientInterface( boost::asio::io_service& io )
	: DispatcherInterface( io )
{
}
