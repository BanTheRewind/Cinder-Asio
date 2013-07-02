#include "cinder/app/AppBasic.h"
#include "cinder/gl/Texture.h"
#include "cinder/params/Params.h"
#include "cinder/Text.h"

#include "boost/algorithm/string.hpp"

#include "HttpRequest.h"
#include "HttpResponse.h"
#include "TcpClient.h"

class HttpClientApp : public ci::app::AppBasic 
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
	
	HttpRequest					mHttpRequest;
	HttpResponse				mHttpResponse;
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

void HttpClientApp::draw()
{
	gl::clear( Colorf::black() );
	
	if ( mTexture ) {
		gl::enableAlphaBlending();
		gl::draw( mTexture, getWindowCenter() - mTextSize * 0.5f );
	}

	mParams->draw();
}

void HttpClientApp::onClose()
{
	mText = "Disconnected";
}

void HttpClientApp::onConnect( TcpSessionRef session )
{
	mText = "Connected";
	
	mHttpResponse = HttpResponse();
	
	mSession = session;
	mSession->addCloseCallback( &HttpClientApp::onClose, this );
	mSession->addErrorCallback( &HttpClientApp::onError, this );
	mSession->addReadCallback( &HttpClientApp::onRead, this );
	mSession->addReadCompleteCallback( &HttpClientApp::onReadComplete, this );
	mSession->addWriteCallback( &HttpClientApp::onWrite, this );
	mSession->write( mHttpRequest.toBuffer() );
}

void HttpClientApp::onError( string err, size_t bytesTransferred )
{
	mText = "Error";
	if ( !err.empty() ) {
		mText += ": " + err;
	}
}

void HttpClientApp::onRead( ci::Buffer buffer )
{
	mText		= toString( buffer.getDataSize() ) + " bytes read";
	mResponse	+= TcpSession::bufferToString( buffer );
	mSession->read();
}

void HttpClientApp::onReadComplete()
{
	mText = "Read complete";
	
	try {
		mHttpResponse.parseResponse( mResponse );
	} catch ( HttpResponse::ExcKeyValuePairInvalid ex ) {
		console() << ex.what() << endl;
	}
	
	console() << mHttpResponse << endl;
	
	mSession->close();
}

void HttpClientApp::onResolve()
{
	mText = "Endpoint resolved";
}

void HttpClientApp::onWrite( size_t bytesTransferred )
{
	mText = toString( bytesTransferred ) + " bytes written";
	
	mSession->read( "\r\n" );
}

void HttpClientApp::setup()
{
	gl::enable( GL_TEXTURE_2D );
	
	mFrameRate		= 0.0f;
	mFullScreen		= false;
	
	mHost			= "libcinder.org";
	mPort			= 80;
	
	mHttpRequest	= HttpRequest( "GET", "/", HttpVersion::HTTP_1_0 );
	mHttpRequest.setHeader( "Host", mHost );
	mHttpRequest.setHeader( "Accept", "*/*" );
	mHttpRequest.setHeader( "Connection", "close" );
	
	mFont			= Font( "Georgia", 60 );
	mText			= "";
	mTextPrev		= mText;
	mTextSize		= Vec2f( getWindowSize() );

	mParams = params::InterfaceGl::create( "Params", Vec2i( 200, 150 ) );
	mParams->addParam( "Frame rate",	&mFrameRate,					"", true );
	mParams->addParam( "Full screen",	&mFullScreen,					"key=f" );
	mParams->addParam( "Host",			&mHost );
	mParams->addParam( "Port",			&mPort,							"min=0 max=65535 step=1 keyDecr=p keyIncr=P" );
	mParams->addButton( "Write", bind(	&HttpClientApp::write, this ),	"key=w" );
	mParams->addButton( "Quit", bind(	&HttpClientApp::quit, this ),	"key=q" );
	
	mClient			= TcpClient::create( io_service() );
	mClient->addConnectCallback( &HttpClientApp::onConnect, this );
	mClient->addErrorCallback( &HttpClientApp::onError, this );
	mClient->addResolveCallback( &HttpClientApp::onResolve, this );
}

void HttpClientApp::update()
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

void HttpClientApp::write()
{
	// This sample is meant to work with only one session at a time
	if ( mSession && mSession->getSocket()->is_open() ) {
		return;
	}
		
	mText = "Connecting to:\n" + mHost + ":" + toString( mPort );
	
	mClient->connect( mHost, (uint16_t)mPort );		
}

CINDER_APP_BASIC( HttpClientApp, RendererGl )
