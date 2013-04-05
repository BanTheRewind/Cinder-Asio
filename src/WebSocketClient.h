#pragma once

#include "Client.h"

typedef std::shared_ptr<class WebSocketClient>	WebSocketClientRef;

class WebSocketClient : public Client
{
public:
	static WebSocketClientRef	create();
	~WebSocketClient();
	
	void					connect( const std::string& host = "localhost", uint16_t port = 80 );

	void					send( uint_fast8_t* buffer, size_t count );
protected:
	WebSocketClient();
};
