#include "cinder/app/AppBasic.h"
#include "cinder/gl/Texture.h"
#include "cinder/params/Params.h"
#include "cinder/Text.h"

#include "TcpServer.h"

/*
 * This application demonstrates how to create a basic TCP
 * echo server. This is meant to communicate with the TcpClient
 * sample running on the same machine. This sample only handles a
 * single session.
 *
 * To communicate with a client, you must create a server and add a
 * connect event callback. Once connected, the server will pass a 
 * session object through the callback. Use the session to communicate
 * by adding callbacks to it. Close the session when you no longer 
 * need it.
 */ 
class TcpServerApp : public ci::app::AppBasic 
{
public:
	void						draw();
	void						setup();
	void						update();
private:
	void						accept();
	TcpServerRef				mServer;
	TcpSessionRef				mSession;
	int32_t						mPort;
	int32_t						mPortPrev;
	
	void						onCancel();
	void						onClose();
	void						onAccept( TcpSessionRef session );
	void						onError( std::string err, size_t bytesTransferred );
	void						onRead( ci::Buffer buffer );
	void						onReadComplete();
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

void TcpServerApp::accept()
{
	if ( mServer ) {
		// This is how you start listening for a connection. Once
		// a connection occurs, a session will be created and passed
		// in through the onAccept method.
		mServer->accept( (uint16_t)mPort );
	
		mText = "Listening on port: " + toString( mPort );
		console() << mText << endl;
	}
}

void TcpServerApp::draw()
{
	gl::clear( Colorf::black() );
	
	if ( mTexture ) {
		gl::enableAlphaBlending();
		gl::draw( mTexture, getWindowCenter() - mTextSize * 0.5f );
	}

	mParams->draw();
}

void TcpServerApp::onCancel()
{
	mText = "Canceled";
	console() << mText << endl;
	
	// Automatically listen on cancel.
	accept();
}

void TcpServerApp::onClose()
{
	mText = "Disconnected";
	console() << mText << endl;
	
	// Automatically listen on close.
	accept();
}

void TcpServerApp::onAccept( TcpSessionRef session )
{
	mText = "Connected";
	console() << mText << endl;
	
	// Get the session from the argument and add callbacks to it.
	mSession = session;
	mSession->connectErrorEventHandler( &TcpServerApp::onError, this );
	mSession->connectReadEventHandler( &TcpServerApp::onRead, this );
	mSession->connectReadCompleteEventHandler( &TcpServerApp::onReadComplete, this );
	mSession->connectWriteEventHandler( &TcpServerApp::onWrite, this );
	mSession->connectCloseEventHandler([&] {
		console() << "Session closed" << endl;
	} );
	
	// Start reading data from the client. 
	mSession->read();
}

void TcpServerApp::onError( string err, size_t bytesTransferred )
{
	mText = "Error";
	if ( !err.empty() ) {
		mText += ": " + err;
	}
	console() << mText << endl;
}

void TcpServerApp::onRead( ci::Buffer buffer )
{
	mText = toString( buffer.getDataSize() ) + " bytes read";
	console() << mText << endl;

	
	// Data is packaged as a ci::Buffer. This allows 
	// you to send any kind of data. Because it's more common to
	// work with strings, the session object has convenience methods
	// for converting between std::string and ci::Buffer.
	string response	= TcpSession::bufferToString( buffer );
	console() << response << endl;
	
	// We're simply echoing what the client sent us. Write the response
	// back to them to acknowledge the succesful communication.
	mSession->write( TcpSession::stringToBuffer( response ) );
}

// This event is triggered when the connection is closed
// remotely. The TcpClient sample is written to close the session
// after it reads the echo, meaning that this will be fired when
// a round of communication is complete.
void TcpServerApp::onReadComplete()
{
	// Close the session.
	mSession->close();

	// Cancel any remaining IO operations (there shouldn't be any).
	// This will tell the server to start listening for the next 
	// connection. See the onCancel method.
	mServer->cancel();
}

void TcpServerApp::onWrite( size_t bytesTransferred )
{
	mText = toString( bytesTransferred ) + " bytes written";
	console() << mText << endl;
	
	// Read after writing to look for a EOF, or disconnect signal
	// from the client.
	mSession->read();
}

void TcpServerApp::setup()
{	
	mFrameRate	= 0.0f;
	mFullScreen	= false;
	
	gl::enable( GL_TEXTURE_2D );

	mPort		= 2000;
	mPortPrev	= mPort;
	
	mFont		= Font( "Georgia", 50 );
	mText		= "";
	mTextPrev	= mText;
	mTextSize	= Vec2f( getWindowSize() );

	mParams = params::InterfaceGl::create( "Params", Vec2i( 200, 110 ) );
	mParams->addParam( "Frame rate",	&mFrameRate,					"", true );
	mParams->addParam( "Full screen",	&mFullScreen,					"key=f" );
	mParams->addParam( "Port",			&mPort,
					  "min=0 max=65535 step=1 keyDecr=p keyIncr=P" );
	mParams->addButton( "Quit", bind(	&TcpServerApp::quit, this ),	"key=q" );
	
	// Initialize a server by passing a boost::asio::io_service to it.
	// ci::App already has one that it polls on update, so we'll use that.
	// You can use your own io_service, but you will have to manage it 
	// manually (ie , call poll(), poll_one(), run(), etc).
	mServer = TcpServer::create( io_service() );

	// Add callbacks to work with the server asynchronously.
	mServer->connectAcceptEventHandler( &TcpServerApp::onAccept, this );
	mServer->connectCancelEventHandler( &TcpServerApp::onCancel, this );
	mServer->connectErrorEventHandler( &TcpServerApp::onError, this );
	
	// Start listening
	accept();
}

void TcpServerApp::update()
{
	mFrameRate = getFrameRate();
	
	// Toggle full screen.
	if ( mFullScreen != isFullScreen() ) {
		setFullScreen( mFullScreen );
		mFullScreen = isFullScreen();
	}
	
	// Close the connection when the port changes. This will 
	// automatically reset the server to listen on the new port.
	// See the onClose method.
	if ( mPortPrev != mPort ) {
		mServer->cancel();
		mPortPrev = mPort;
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

CINDER_APP_BASIC( TcpServerApp, RendererGl )
