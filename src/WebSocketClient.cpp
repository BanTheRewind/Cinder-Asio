#include "WebSocketClient.h"

using namespace ci;
using namespace std;

WebSocketClientRef WebSocketClient::create()
{
	return WebSocketClientRef( new WebSocketClient() );
}

WebSocketClient::WebSocketClient()
	: Client()
{
}

WebSocketClient::~WebSocketClient()
{
}

void WebSocketClient::connect( const string& host, uint16_t port )
{
}

void WebSocketClient::send( uint_fast8_t *buffer, size_t count ) 
{
}
