#pragma once

#include "cinder/Buffer.h"
#include "DispatcherEventHandlerInterface.h"

class SessionEventHandlerInterface : public DispatcherEventHandlerInterface
{
public:
	virtual void onRead( ci::BufferRef buffer ) = 0;
	virtual void onReadComplete() = 0;
	virtual void onWrite( size_t bytesTransferred ) = 0;
};
