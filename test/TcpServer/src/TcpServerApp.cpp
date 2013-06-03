#include "cinder/app/AppBasic.h"
#include "cinder/gl/Texture.h"
#include "cinder/params/Params.h"
#include "cinder/Text.h"

#include "TcpServer.h"

class TcpServerApp : public ci::app::AppBasic 
{
public:
	void						draw();
	void						setup();
	void						update();
private:
	TcpServerRef				mServer;
	TcpSessionRef				mSession;
	int32_t						mPort;
	std::string					mRequest;
	std::string					mResponse;
	void						write();
	
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
}

void TcpServerApp::onClose()
{
	mText = "Disconnected";
}

void TcpServerApp::onAccept( TcpSessionRef session )
{
	mText = "Connected";
	mResponse.clear();

	mSession = session;
	mSession->addCloseCallback( &TcpServerApp::onClose, this );
	mSession->addErrorCallback( &TcpServerApp::onError, this );
	mSession->addReadCallback( &TcpServerApp::onRead, this );
	mSession->addReadCompleteCallback( &TcpServerApp::onReadComplete, this );
	mSession->addWriteCallback( &TcpServerApp::onWrite, this );
	mSession->write( TcpSession::stringToBuffer( mRequest ) );
}

void TcpServerApp::onError( string err, size_t bytesTransferred )
{
	mText = "Error";
	if ( !err.empty() ) {
		mText += ": " + err;
	}
}

void TcpServerApp::onRead( ci::Buffer buffer )
{
	mText		= toString( buffer.getDataSize() ) + " bytes read";
	mResponse	+= TcpSession::bufferToString( buffer );
	mSession->read();
}

void TcpServerApp::onReadComplete()
{
	mText = "Read complete";
	if ( !mResponse.empty() ) {
		console() << mResponse << endl;
		mText += ": " + toString( mResponse.size() ) + " bytes";
	}
	mSession->close();
}

void TcpServerApp::onWrite( size_t bytesTransferred )
{
	mText = toString( bytesTransferred ) + " bytes written";
	
	mSession->read();
}

void TcpServerApp::setup()
{	
	mFrameRate	= 0.0f;
	mFullScreen	= false;
	
	mPort		= 2000;
	mRequest	= "Hello, client!";
	
	gl::enable( GL_TEXTURE_2D );
	
	mFont		= Font( "Georgia", 60 );
	mText		= "";
	mTextPrev	= mText;
	mTextSize	= Vec2f( getWindowSize() );

	mParams = params::InterfaceGl::create( "Params", Vec2i( 200, 150 ) );
	mParams->addParam( "Frame rate",	&mFrameRate,					"", true );
	mParams->addParam( "Full screen",	&mFullScreen,					"key=f" );
	mParams->addParam( "Port",			&mPort,
					  "min=0 max=65535 step=1 keyDecr=p keyIncr=P" );
	mParams->addParam( "Request",		&mRequest								);
	mParams->addButton( "Write", bind(	&TcpServerApp::write, this ),	"key=w" );
	mParams->addButton( "Quit", bind(	&TcpServerApp::quit, this ),	"key=q" );
	
	mServer = TcpServer::create( io_service() );
	mServer->addAcceptCallback( &TcpServerApp::onAccept, this );
	mServer->addCancelCallback( &TcpServerApp::onCancel, this );
	mServer->addErrorCallback( &TcpServerApp::onError, this );
	
	mText = "Listening on port: " + toString( mPort );
	
	mServer->accept( (uint16_t)mPort );
}

void TcpServerApp::update()
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

void TcpServerApp::write()
{
	// This sample is meant to work with only one session at a time
	if ( mSession && mSession->getSocket()->is_open() ) {
		return;
	}
}

CINDER_APP_BASIC( TcpServerApp, RendererGl )
