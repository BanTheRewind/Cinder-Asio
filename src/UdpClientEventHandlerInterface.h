#pragma once

#include "ClientEventHandlerInterface.h"
#include "UdpSession.h"

class UdpClientEventHandlerInterface : public ClientEventHandlerInterface
{
public:
	virtual void onConnect( UdpSessionRef session ) = 0;
};
