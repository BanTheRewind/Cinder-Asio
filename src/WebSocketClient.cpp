#include "WebSocketClient.h"

using namespace ci;
using namespace std;

WebSocketClientRef WebSocketClient::create()
{
	return WebSocketClientRef( new WebSocketClient() );
}

WebSocketClient::WebSocketClient()
: HttpClient()
{
	mMethod			= "GET";
	mHttpVersion	= HttpVersion::HTTP_1_1;
	
	setHeaderField( "Host",						"mHost" );
	setHeaderField( "Upgrade",					"WebSocket" );
	setHeaderField( "Connection",				"Upgrade" );
	setHeaderField( "Sec-WebSocket-Key",		"" );
	setHeaderField( "Origin",					"http://" + mHost );
	setHeaderField( "Sec-WebSocket-Protocol",	mPath );
	setHeaderField( "Sec-WebSocket-Version",	"13" );
	
	concatenateHeader();
}

WebSocketClient::~WebSocketClient()
{
}

void WebSocketClient::connect( const string& host, uint16_t port )
{
	HttpClient::connect( host, port );
	setHeaderField( "Host",	mHost );
}
