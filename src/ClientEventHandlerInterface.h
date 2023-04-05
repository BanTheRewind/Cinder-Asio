#pragma once

#include "DispatcherEventHandlerInterface.h"

class ClientEventHandlerInterface : public DispatcherEventHandlerInterface
{
public:
	virtual void onResolve() = 0;
};
