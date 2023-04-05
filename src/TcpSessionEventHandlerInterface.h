#pragma once

#include "SessionEventHandlerInterface.h"

class TcpSessionEventHandlerInterface : public SessionEventHandlerInterface
{
public:
	virtual void onClose() = 0;
};
