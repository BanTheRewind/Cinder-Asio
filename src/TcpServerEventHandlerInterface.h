#pragma once

#include "ServerEventHandlerInterface.h"
#include "TcpSession.h"

class TcpServerEventHandlerInterface : public ServerEventHandlerInterface
{
public:
	virtual void onAccept( TcpSessionRef session ) = 0;
	virtual void ocCancel() = 0;
};
