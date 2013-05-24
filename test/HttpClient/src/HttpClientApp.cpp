#include "cinder/app/AppBasic.h"
#include "cinder/params/Params.h"

#include "TcpClient.h"

class HttpClientApp : public ci::app::AppBasic 
{
public:
	void						draw();
	void						setup();
	void						update();
private:
	TcpClientRef				mClient;
	std::string					mHost;
	uint16_t					mPort;
	std::string					mRequest;
	void						send();
	
	void						onConnect();
	void						onDisconnect();
	void						onError( std::string error, size_t bytesTransferred );
	void						onRead( ci::Buffer buffer );
	void						onReadComplete();
	void						onResolve();
	void						onWrite( size_t bytesTransferred );
	
	std::string					mResponse;
	
	float						mFrameRate;
	bool						mFullScreen;
	ci::params::InterfaceGlRef	mParams;
};

#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

void HttpClientApp::draw()
{
	gl::clear( Colorf::black() );
	
	mParams->draw();
}

void HttpClientApp::onConnect()
{
	console() << "Connected." << endl;
	mResponse.clear();
	mClient->write( Buffer( &mRequest[ 0 ], mRequest.size() ) );
}

void HttpClientApp::onDisconnect()
{
	console() << "Disconnected." << endl;
}

void HttpClientApp::onError( string error, size_t bytesTransferred )
{
	console() << "Error: " << error << "." << endl;
}

void HttpClientApp::onReadComplete()
{
	console() << "Read complete." << endl;
	console() << mResponse << endl;
	
	mClient->disconnect();
}

void HttpClientApp::onRead( ci::Buffer buffer )
{
	console() << buffer.getDataSize() << " bytes read." << endl;
	
	// Stringify buffer
	string response( static_cast<const char*>( buffer.getData() ) );
	mResponse += response;
	
	mClient->read();
}

void HttpClientApp::onResolve()
{
	console() << "Endpoint resolved." << endl;
}

void HttpClientApp::onWrite( size_t bytesTransferred )
{
	console() << bytesTransferred << " bytes written." << endl;
	
	mClient->read();
}

void HttpClientApp::send()
{
	console() << "Connecting to: " << mHost << ":" << mPort << endl;
	
	mClient->connect( mHost, mPort );
}

void HttpClientApp::setup()
{	
	mFrameRate	= 0.0f;
	mFullScreen	= false;
	
	mHost		= "libcinder.org";
	mPort		= 80;
	
	mRequest = "GET / HTTP/1.0\r\n";
	mRequest += "Host: " + mHost + "\r\n";
	mRequest += "Accept: */*\r\n";
	mRequest += "Connection: close\r\n\r\n";
	
	mParams = params::InterfaceGl::create( "Params", Vec2i( 200, 150 ) );
	mParams->addParam( "Frame rate",	&mFrameRate,					"", true );
	mParams->addParam( "Full screen",	&mFullScreen,					"key=f" );
	mParams->addButton( "Send", bind(	&HttpClientApp::send, this ),	"key=s" );
	mParams->addButton( "Quit", bind(	&HttpClientApp::quit, this ),	"key=q" );
	
	mClient = TcpClient::create( io_service() );
	mClient->addConnectCallback( &HttpClientApp::onConnect, this );
	mClient->addDisconnectCallback( &HttpClientApp::onDisconnect, this );
	mClient->addErrorCallback( &HttpClientApp::onError, this );
	mClient->addReadCallback( &HttpClientApp::onRead, this );
	mClient->addReadCompleteCallback( &HttpClientApp::onReadComplete, this );
	mClient->addResolveCallback( &HttpClientApp::onResolve, this );
	mClient->addWriteCallback( &HttpClientApp::onWrite, this );

	send();
}

void HttpClientApp::update()
{
	mFrameRate = getFrameRate();
	
	if ( mFullScreen != isFullScreen() ) {
		setFullScreen( mFullScreen );
		mFullScreen = isFullScreen();
	}
}

CINDER_APP_BASIC( HttpClientApp, RendererGl )
