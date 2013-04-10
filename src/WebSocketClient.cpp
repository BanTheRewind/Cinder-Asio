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
	
	setHeaderField( "Host",						"mHost"				);
	setHeaderField( "Upgrade",					"WebSocket"			);
	setHeaderField( "Connection",				"Upgrade"			);
	setHeaderField( "Sec-WebSocket-Key",		""					);
	setHeaderField( "Origin",					"http://" + mHost	);
	setHeaderField( "Sec-WebSocket-Protocol",	mPath				);
	setHeaderField( "Sec-WebSocket-Version",	"13"				);
	
	concatenateHeader();
}

WebSocketClient::~WebSocketClient()
{
	mProtocols.clear();
}

void WebSocketClient::connect( const string& host, uint16_t port )
{
	HttpClient::connect( host, port );
	setHeaderField( "Host",	mHost );
}

const string& WebSocketClient::getKey() const
{
	return getHeaderField( "Sec-WebSocket-Key" );
}

void WebSocketClient::setKey( const string& value )
{
	setHeaderField( "Sec-WebSocket-Key", value );
}

const vector<string>& WebSocketClient::getProtocols() const
{
	return mProtocols;
}

void WebSocketClient::setProtocols( const vector<string>& values )
{
	mProtocols = values;
	
	string protocols	= "";
	size_t count		= mProtocols.size();
	if  ( count > 0 ) {
		vector<string>::const_iterator iter = mProtocols.begin();
		protocols += *iter;
		++iter;
		for ( ; iter != mProtocols.end(); ++iter ) {
			protocols += ", " + *iter;
		}
	}
	
	setHeaderField( "Sec-WebSocket-Protocol", protocols );
}

const string& WebSocketClient::getVersion() const
{
	return getHeaderField( "Sec-WebSocket-Version" );
}

void WebSocketClient::setVersion( const string& value )
{
	setHeaderField( "Sec-WebSocket-Version", value );
}
