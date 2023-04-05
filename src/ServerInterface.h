#pragma once

#include "DispatcherInterface.h"

class ServerInterface : public DispatcherInterface
{
public:
	virtual void accept( uint16_t port ) = 0;
protected:
	ServerInterface( asio::io_service& io );
};
	