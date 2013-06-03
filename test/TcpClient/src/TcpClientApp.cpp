#include "cinder/app/AppBasic.h"
#include "cinder/gl/Texture.h"
#include "cinder/params/Params.h"
#include "cinder/Text.h"

#include "TcpClient.h"

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
	std::string					mResponse;
	void						write();
	
	void						onClose();
	void						onConnect( TcpSessionRef session );
	void						onError( std::string err, size_t bytesTransferred );
	void						onRead( ci::Buffer buffer );
	void						onReadComplete();
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
}

void TcpClientApp::onConnect( TcpSessionRef session )
{
	mText = "Connected";
	mResponse.clear();

	mSession = session;
	mSession->addCloseCallback( &TcpClientApp::onClose, this );
	mSession->addErrorCallback( &TcpClientApp::onError, this );
	mSession->addReadCallback( &TcpClientApp::onRead, this );
	mSession->addReadCompleteCallback( &TcpClientApp::onReadComplete, this );
	mSession->addWriteCallback( &TcpClientApp::onWrite, this );
	mSession->write( TcpSession::stringToBuffer( mRequest ) );
}

void TcpClientApp::onError( string err, size_t bytesTransferred )
{
	mText = "Error";
	if ( !err.empty() ) {
		mText += ": " + err;
	}
}

void TcpClientApp::onRead( ci::Buffer buffer )
{
	mText		= toString( buffer.getDataSize() ) + " bytes read";
	mResponse	+= TcpSession::bufferToString( buffer );
	mSession->read();
}

void TcpClientApp::onReadComplete()
{
	mText = "Read complete";
	if ( !mResponse.empty() ) {
		console() << mResponse << endl;
		mText += ": " + toString( mResponse.size() ) + " bytes";
	}
	mSession->close();
}

void TcpClientApp::onResolve()
{
	mText = "Endpoint resolved";
}

void TcpClientApp::onWrite( size_t bytesTransferred )
{
	mText = toString( bytesTransferred ) + " bytes written";
	
	mSession->read();
}

void TcpClientApp::setup()
{	
	mFrameRate	= 0.0f;
	mFullScreen	= false;
	
	mHost		= "localhost";
	mPort		= 2000;
	mRequest	= "Hello, server!";
	
	gl::enable( GL_TEXTURE_2D );
	
	mFont		= Font( "Georgia", 60 );
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
	
	mClient = TcpClient::create( io_service() );
	mClient->addConnectCallback( &TcpClientApp::onConnect, this );
	mClient->addErrorCallback( &TcpClientApp::onError, this );
	mClient->addResolveCallback( &TcpClientApp::onResolve, this );
}

void TcpClientApp::update()
{
	mFrameRate = getFrameRate();
	
	if ( mFullScreen != isFullScreen() ) {
		setFullScreen( mFullScreen );
		mFullScreen = isFullScreen();
	}

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
	
	mClient->connect( mHost, (uint16_t)mPort );		
}

CINDER_APP_BASIC( TcpClientApp, RendererGl )