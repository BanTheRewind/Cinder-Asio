#include "cinder/app/AppBasic.h"
#include "cinder/params/Params.h"
#include "cinder/Json.h"

#include "TcpClient.h"

class WebSocketClientApp : public ci::app::AppBasic 
{
public:
	void						draw();
	void						setup();
	void						shutdown();
	void						update();
private:
	TcpClientRef				mClient;
	std::string					mHandshake;
	std::string					mHost;
	std::string					mKey;
	uint16_t					mPort;
	ci::JsonTree				mRequest;
	std::string					mResponse;
	void						send();
	size_t						toOctal( unsigned char c );

	void						onConnect();
	void						onError( std::string error, size_t bytesTransferred );
	void						onRead( ci::Buffer buffer );
	void						onReadComplete();
	void						onResolve();
	void						onWait();
	void						onWrite( size_t bytesTransferred );
	
	float						mFrameRate;
	bool						mFullScreen;
	ci::params::InterfaceGlRef	mParams;
};

#include "cinder/Base64.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;
using namespace std;

void WebSocketClientApp::draw()
{
	gl::clear( Colorf::black() );
	
	mParams->draw();
}

void WebSocketClientApp::onConnect()
{
	console() << "Connected." << endl;
	mClient->write( Buffer( &mHandshake[ 0 ], mHandshake.size() ) );

	mClient->wait( 1000, true );
}

void WebSocketClientApp::onError( string error, size_t bytesTransferred )
{
	console() << "Error: " << error << "." << endl;
	//mClient->connect( mHost, mPort );
}

void WebSocketClientApp::onReadComplete()
{
	console() << "Read complete." << endl;
	console() << mResponse << endl;
	
	// TODO process response
	mResponse.clear();
}

void WebSocketClientApp::onRead( ci::Buffer buffer )
{
	console() << buffer.getDataSize() << " bytes read." << endl;
	
	// Stringify buffer
	const char* response = static_cast<const char*>( buffer.getData() );

	console() << toString( (size_t)response[ 0 ] ) << endl;
	mResponse += response;
}

void WebSocketClientApp::onResolve()
{
	console() << "Endpoint resolved." << endl;
}

void WebSocketClientApp::onWait()
{
	size_t ping[ 1 ] = { 0x9 };
	mClient->write(	Buffer( ping, sizeof( size_t ) ) );
}	

void WebSocketClientApp::onWrite( size_t bytesTransferred )
{
	console() << bytesTransferred << " bytes written." << endl;

	mClient->read();
}

void WebSocketClientApp::send()
{
	// Convert message to octal
	string msg = "echo";

	vector<size_t> buffer;
	for ( size_t i = 0; i < msg.size(); ++i ) {
		buffer.push_back( toOctal( msg[ i ] ) );
	}
	buffer[ 0 ] = 0x1 & buffer[ 0 ];

	mClient->write( Buffer( &buffer[ 0 ], buffer.size() * sizeof( size_t ) ) );
}

void WebSocketClientApp::setup()
{
	mFrameRate	= 0.0f;
	mFullScreen	= false;
	mHost		= "echo.websocket.org";
	mPort		= 80;
	mResponse	= "";
	
	 // Generate client key
	const size_t keyLength = 16;
    int32_t key[ keyLength ];
    for ( int i = 0; i < keyLength; ++i ) {
        key[ i ] = randInt();
    }
	mKey = toBase64( Buffer( key, keyLength ) );

	// Build handshake
	mHandshake = "GET /?encoding=text HTTP/1.1\r\n";
	mHandshake += "Connection: Upgrade\r\n";
	mHandshake += "Upgrade: WebSocket\r\n";
	mHandshake += "Host: " + mHost + "\r\n";
	mHandshake += "Pragma: no-cache\r\n";
	mHandshake += "Cache-Control: no-cache\r\n";
	mHandshake += "Origin: ws://" + mHost + "\r\n";
	mHandshake += "Sec-WebSocket-Key: " + mKey + "\r\n";
	mHandshake += "Sec-WebSocket-Version: 13\r\n";
	mHandshake += "\r\n";

	console() << mHandshake;
	
	// Set up parameters
	mParams = params::InterfaceGl::create( "Params", Vec2i( 200, 150 ) );
	mParams->addParam( "Frame rate",	&mFrameRate,						"", true );
	mParams->addParam( "Full screen",	&mFullScreen,						"key=f" );
	mParams->addButton( "Send", bind(	&WebSocketClientApp::send, this ),	"key=s" );
	mParams->addButton( "Quit", bind(	&WebSocketClientApp::quit, this ),	"key=q" );
	
	// Initialize client and add callbacks
	mClient = TcpClient::create( io_service() );
	mClient->addConnectCallback(		&WebSocketClientApp::onConnect,			this );
	mClient->addErrorCallback(			&WebSocketClientApp::onError,			this );
	mClient->addReadCallback(			&WebSocketClientApp::onRead,			this );
	mClient->addReadCompleteCallback(	&WebSocketClientApp::onReadComplete,	this );
	mClient->addResolveCallback(		&WebSocketClientApp::onResolve,			this );
	mClient->addWaitCallback(			&WebSocketClientApp::onWait,			this );
	mClient->addWriteCallback(			&WebSocketClientApp::onWrite,			this );
	
	// Connect to client
	try {
		mClient->connect( mHost, mPort );
	} catch ( Exception ex ) {
		console() << ex.what() << endl;
		quit();
	}
}

void WebSocketClientApp::shutdown()
{
	mClient->wait( 0 );
	size_t close[ 1 ] = { 0x8 };
	mClient->write(	Buffer( close, sizeof( size_t ) ) );
}

size_t WebSocketClientApp::toOctal( unsigned char c )
{
	char buffer[ 4 ];
	buffer[ 0 ] = '0' + ( ( c >> 6 ) & 7 );
	buffer[ 1 ] = '0' + ( ( c >> 3 ) & 7 );
	buffer[ 2 ] = '0' + ( c & 7 );
	buffer[ 3 ] = '\0';
	return fromString<size_t>( buffer );
}

void WebSocketClientApp::update()
{
	mFrameRate = getFrameRate();
	
	if ( mFullScreen != isFullScreen() ) {
		setFullScreen( mFullScreen );
		mFullScreen = isFullScreen();
	}
}

CINDER_APP_BASIC( WebSocketClientApp, RendererGl )
