#include "cinder/app/AppBasic.h"
#include "cinder/gl/Texture.h"
#include "cinder/params/Params.h"
#include "cinder/Text.h"

#include "TcpClient.h"

/*
 * This application demonstrates how to create a basic TCP
 * echo client. This is meant to communicate with the TcpServer
 * sample running on the same machine. This sample only handles a
 * single session.
 *
 * To communicate with a host, you must create a client and add a
 * connect event callback. Once connected, the client will pass a 
 * session object through the callback. Use the session to communicate
 * by adding callbacks to it. Close the session when you no longer 
 * need it.
 */ 
class TcpClientApp : public ci::app::AppBasic 
{
public:
	void						draw();
	void						setup();
	void						update();
private:
	TcpClientRef				mClient;
	TcpSessionRef				mSession;
	std::string					mHost;
	int32_t						mPort;
	std::string					mRequest;
	void						write();
	
	void						onClose();
	void						onConnect( TcpSessionRef session );
	void						onError( std::string err, size_t bytesTransferred );
	void						onRead( ci::Buffer buffer );
	void						onResolve();
	void						onWrite( size_t bytesTransferred );
	
	ci::Font					mFont;
	std::string					mText;
	std::string					mTextPrev;
	ci::Vec2f					mTextSize;
	ci::gl::Texture				mTexture;

	float						mFrameRate;
	bool						mFullScreen;
	ci::params::InterfaceGlRef	mParams;
};

#include "cinder/Utilities.h"

using namespace ci;
using namespace ci::app;
using namespace std;

void TcpClientApp::draw()
{
	gl::clear( Colorf::black() );
	
	if ( mTexture ) {
		gl::enableAlphaBlending();
		gl::draw( mTexture, getWindowCenter() - mTextSize * 0.5f );
	}

	mParams->draw();
}

void TcpClientApp::onClose()
{
	mText = "Disconnected";
	console() << mText << endl;
}

void TcpClientApp::onConnect( TcpSessionRef session )
{
	mText = "Connected";
	console() << mText << endl;

	// Get the session from the argument and add callbacks to it.
	mSession = session;
	mSession->connectCloseEventHandler( &TcpClientApp::onClose, this );
	mSession->connectErrorEventHandler( &TcpClientApp::onError, this );
	mSession->connectReadEventHandler( &TcpClientApp::onRead, this );
	mSession->connectWriteEventHandler( &TcpClientApp::onWrite, this );

	// Write data is packaged as a ci::Buffer. This allows 
	// you to send any kind of data. Because it's more common to
	// work with strings, the session object has convenience methods
	// for converting between std::string and ci::Buffer.
	Buffer buffer = TcpSession::stringToBuffer( mRequest );
	mSession->write( buffer );
}

void TcpClientApp::onError( string err, size_t bytesTransferred )
{
	mText = "Error";
	if ( !err.empty() ) {
		mText += ": " + err;
	}
	console() << mText << endl;
}

void TcpClientApp::onRead( ci::Buffer buffer )
{
	mText		= toString( buffer.getDataSize() ) + " bytes read";
	console() << mText << endl;
	
	// Responses are passed in the read callback as ci::Buffer. Use
	// a convenience method on the session object to convert it to
	// a std::string.
	string response	= TcpSession::bufferToString( buffer );
	console() << response << endl;
	
	// Closing the connection after reading mimics the behavior
	// of a HTTP client. To keep the connection open and continue
	// communicating with the server, comment the line below.
	mSession->close();
}

void TcpClientApp::onResolve()
{
	mText = "Endpoint resolved";
	console() << mText << endl;
}

void TcpClientApp::onWrite( size_t bytesTransferred )
{
	mText = toString( bytesTransferred ) + " bytes written";
	console() << mText << endl;
	
	// After successfully send your request to the server, you 
	// should expect a response. Start reading immediately.
	mSession->read();
}

void TcpClientApp::setup()
{	
	mFrameRate	= 0.0f;
	mFullScreen	= false;
	
	gl::enable( GL_TEXTURE_2D );
	
	mHost		= "localhost";
	mPort		= 2000;
	mRequest	= "echo";
	
	mFont		= Font( "Georgia", 50 );
	mText		= "";
	mTextPrev	= mText;
	mTextSize	= Vec2f( getWindowSize() );

	mParams = params::InterfaceGl::create( "Params", Vec2i( 200, 150 ) );
	mParams->addParam( "Frame rate",	&mFrameRate,					"", true );
	mParams->addParam( "Full screen",	&mFullScreen,					"key=f" );
	mParams->addParam( "Host",			&mHost );
	mParams->addParam( "Port",			&mPort,
					  "min=0 max=65535 step=1 keyDecr=p keyIncr=P" );
	mParams->addParam( "Request",		&mRequest								);
	mParams->addButton( "Write", bind(	&TcpClientApp::write, this ),	"key=w" );
	mParams->addButton( "Quit", bind(	&TcpClientApp::quit, this ),	"key=q" );
	
	// Initialize a client by passing a boost::asio::io_service to it.
	// ci::App already has one that it polls on update, so we'll use that.
	// You can use your own io_service, but you will have to manage it 
	// manually (ie , call poll(), poll_one(), run(), etc).
	mClient = TcpClient::create( io_service() );

	// Add callbacks to work with the client asynchronously.
	mClient->connectConnectEventHandler( &TcpClientApp::onConnect, this );
	mClient->connectErrorEventHandler( &TcpClientApp::onError, this );
	mClient->connectResolveEventHandler( &TcpClientApp::onResolve, this );
}

void TcpClientApp::update()
{
	mFrameRate = getFrameRate();
	
	// Toggle full screen.
	if ( mFullScreen != isFullScreen() ) {
		setFullScreen( mFullScreen );
		mFullScreen = isFullScreen();
	}

	// Update text.
	if ( mTextPrev != mText ) {
		mTextPrev = mText;
		if ( mText.empty() ) {
			mTexture.reset();
		} else {
			TextBox tbox = TextBox().alignment( TextBox::CENTER ).font( mFont ).size( Vec2i( mTextSize.x, TextBox::GROW ) ).text( mText );
			tbox.setColor( ColorAf( 1.0f, 0.8f, 0.75f, 1.0f ) );
			tbox.setBackgroundColor( ColorAf::black() );
			tbox.setPremultiplied( false );
			mTextSize.y	= tbox.measure().y;
			mTexture	= gl::Texture( tbox.render() );
		}
	}
}

void TcpClientApp::write()
{
	// This sample is meant to work with only one session at a time
	if ( mSession && mSession->getSocket()->is_open() ) {
		return;
	}
		
	mText = "Connecting to:\n" + mHost + ":" + toString( mPort );
	console() << mText << endl;
	
	// Before we can write, we need to esablish a connection 
	// and create a session. Check out the onConnect method.
	mClient->connect( mHost, (uint16_t)mPort );		
}

CINDER_APP_BASIC( TcpClientApp, RendererGl )
