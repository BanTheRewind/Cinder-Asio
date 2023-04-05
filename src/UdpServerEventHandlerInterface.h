#pragma once

#include "ServerEventHandlerInterface.h"
#include "UdpSession.h"

class UdpServerEventHandlerInterface : public ServerEventHandlerInterface
{
public:
	virtual void onAccept( UdpSessionRef session ) = 0;
};
