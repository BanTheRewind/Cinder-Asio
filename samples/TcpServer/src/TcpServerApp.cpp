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
	
	accept();
}

void TcpServerApp::onClose()
{
	mText = "Disconnected";
	console() << mText << endl;
}

void TcpServerApp::onAccept( TcpSessionRef session )
{
	mText = "Connected";
	console() << mText << endl;
	
	mSession = session;
	mSession->addCloseCallback( &TcpServerApp::onClose, this );
	mSession->addErrorCallback( &TcpServerApp::onError, this );
	mSession->addReadCallback( &TcpServerApp::onRead, this );
	mSession->addReadCompleteCallback( &TcpServerApp::onReadComplete, this );
	mSession->addWriteCallback( &TcpServerApp::onWrite, this );
	
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
	
	string response	= TcpSession::bufferToString( buffer );
	console() << response << endl;
	
	mSession->write( TcpSession::stringToBuffer( response ) );
}

void TcpServerApp::onReadComplete()
{
	mSession->close();
	mServer->cancel();
}

void TcpServerApp::onWrite( size_t bytesTransferred )
{
	mText = toString( bytesTransferred ) + " bytes written";
	console() << mText << endl;
	
	mSession->read();
}

void TcpServerApp::setup()
{	
	mFrameRate	= 0.0f;
	mFullScreen	= false;
	
	mPort		= 2000;
	mPortPrev	= mPort;
	
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
	mParams->addButton( "Quit", bind(	&TcpServerApp::quit, this ),	"key=q" );
	
	mServer = TcpServer::create( io_service() );
	mServer->addAcceptCallback( &TcpServerApp::onAccept, this );
	mServer->addCancelCallback( &TcpServerApp::onCancel, this );
	mServer->addErrorCallback( &TcpServerApp::onError, this );
	
	accept();
}

void TcpServerApp::update()
{
	mFrameRate = getFrameRate();
	
	if ( mFullScreen != isFullScreen() ) {
		setFullScreen( mFullScreen );
		mFullScreen = isFullScreen();
	}
	
	if ( mPortPrev != mPort ) {
		mServer->cancel();
		mPortPrev = mPort;
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

CINDER_APP_BASIC( TcpServerApp, RendererGl )
