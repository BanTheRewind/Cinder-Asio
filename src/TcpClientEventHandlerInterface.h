#pragma once

#include "ClientEventHandlerInterface.h"
#include "TcpSession.h"

class TcpClientEventHandlerInterface : public ClientEventHandlerInterface
{
public:
	virtual void onConnect( TcpSessionRef session ) = 0;
};
 