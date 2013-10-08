#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/params/Params.h"
#include "cinder/Text.h"
#include "cinder/Utilities.h"

#include "UdpServer.h"

class UdpServerApp : public ci::app::AppBasic
{
public:
	void						setup();
	void						update();
	void						draw();

private:
	void						setupParams();
	void						setupUdp();

	UdpServerRef				mServer;
	UdpSessionRef				mSession;
	int32_t						mPort;
	int32_t						mPortPrev;

	ci::Font					mFont;
	std::string					mText;
	std::string					mTextPrev;
	ci::Vec2f					mTextSize;
	ci::gl::TextureRef			mTexture;

	float						mFrameRate;
	ci::params::InterfaceGlRef	mParams;
};


using namespace ci;
using namespace ci::app;
using namespace std;

void UdpServerApp::draw()
{
	gl::clear( Colorf::black() );

	if ( mTexture ) {
		gl::enableAlphaBlending();
		gl::draw( mTexture, getWindowCenter() - mTextSize * 0.5f );
	}

	mParams->draw();
}

void UdpServerApp::setup()
{
	setupParams();
	setupUdp();
}

void UdpServerApp::setupParams()
{
	mFrameRate	= 0.0f;

	mPort		= 2000;
	mPortPrev	= mPort;
	mFont		= Font( "Georgia", 50.0f );
	mText		= "";
	mTextPrev	= mText;
	mTextSize	= Vec2f( getWindowSize() );

	mParams		= params::InterfaceGl::create( "Params", Vec2i( 200, 110 ) );

	mParams->addParam( "Frame rate",	&mFrameRate,					"", true );
	mParams->addParam( "Port",			&mPort, 
					"min=0 max=65535 step=1 keyDecr=p keyIncr=P" );
	mParams->addButton( "Quit", bind(	&UdpServerApp::quit, this ),	"key=q" );
}

void UdpServerApp::setupUdp()
{
	mServer = UdpServer::create( io_service() );
	mServer->connectErrorEventHandler( 
		[&]( const string& errMsg, size_t errCode ) {
			mText = "UdpServer Error " + toString( errCode ) + ": " + errMsg;
			console() << mText << endl;
		} 
	);

	mServer->connectAcceptEventHandler(
		[&]( UdpSessionRef session ) {
			mSession = session;

			mSession->connectReadEventHandler(
				[&]( Buffer buffer ) {
					mText = "UdpSession read " + toString( buffer.getDataSize() ) + " bytes";
					console() << mText << endl;
					mSession->read();
				}
			);

			mSession->connectErrorEventHandler(
				[&]( const string& errMsg, size_t errCode ) {
					mText = "UdpSession Error " + toString( errCode ) + ": " + errMsg;
					console() << mText << endl;
				}
			);

			mSession->connectReadCompleteEventHandler(
				[&]() {
					mText = "UdpSession Read Complete";
					console() << mText << endl;
				}
			);

			mSession->read();
		}
	);

	mServer->accept( mPort );
}

void UdpServerApp::update()
{
	mFrameRate = getFrameRate();

	if ( mPortPrev != mPort ) {
		mServer->accept( mPort );

		mPortPrev = mPort;
	}

	if ( mTextPrev != mText ) {
		mTextPrev = mText;
		if ( mText.empty() ) {
			mTexture->reset();
		} else {
			TextBox tbox = TextBox().alignment( TextBox::CENTER ).font( mFont ).size( Vec2i( mTextSize.x, TextBox::GROW ) ).text( mText );
			tbox.setColor( ColorAf( 1.0f, 0.8f, 0.75f, 1.0f ) );
			tbox.setBackgroundColor( ColorAf::black() );
			tbox.setPremultiplied( false );
			mTextSize.y	= tbox.measure().y;
			mTexture = gl::Texture::create( tbox.render() );
		}
	}
}

CINDER_APP_BASIC( UdpServerApp, RendererGl )
