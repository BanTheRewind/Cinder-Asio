#include "ServerInterface.h"

ServerInterface::ServerInterface( boost::asio::io_service& io )
	: DispatcherInterface( io )
{
}
