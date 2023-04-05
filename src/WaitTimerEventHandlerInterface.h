#pragma once

#include "DispatcherEventHandlerInterface.h"

class WaitTimerEventHandlerInterface : public DispatcherEventHandlerInterface
{
public:
	virtual void onWait() = 0;
};
