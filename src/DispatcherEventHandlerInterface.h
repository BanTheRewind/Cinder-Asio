#pragma once

#include <string>

class DispatcherEventHandlerInterface
{
public:
	virtual void onError( std::string err, size_t bytesTransferred ) = 0;
};
