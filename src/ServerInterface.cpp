#include "ServerInterface.h"

ServerInterface::ServerInterface( asio::io_service& io )
	: DispatcherInterface( io )
{
}
 