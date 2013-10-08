#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/params/Params.h"
#include "cinder/Text.h"
#include "cinder/Utilities.h"

#include "UdpClient.h"
#include "UdpSession.h"

class UdpClientApp : public ci::app::AppBasic
{
public:
	void setup();
	void update();
	void draw();

private:
	void						setupParams();
	void						setupUdp();
	void						onError( std::string errMsg, size_t errCode );
	void						write();

	UdpClientRef				mClient;
	UdpSessionRef				mSession;
	std::string					mHost;
	std::string					mHostPrev;
	int32_t						mPort;
	int32_t						mPortPrev;

	ci::Font					mFont;
	std::string					mText;
	std::string					mTextPrev;
	ci::Vec2f					mTextSize;
	ci::gl::TextureRef			mTexture;

	float						mFrameRate;
	ci::params::InterfaceGlRef	mParams;
	std::string					mTestStr;
};

using namespace ci;
using namespace ci::app;
using namespace std;

void UdpClientApp::draw()
{
	gl::clear( Colorf::black() );

	if ( mTexture ) {
		gl::enableAlphaBlending();
		gl::draw( mTexture, getWindowCenter() - mTextSize * 0.5f );
	}

	mParams->draw();
}

void UdpClientApp::onError( string errMsg, size_t errCode )
{
	mText = "Udp Error " + toString( errCode ) + ": " + errMsg;
	console() << mText << endl;
}

void UdpClientApp::setup()
{
	setupParams();
	setupUdp();

	mClient->connect( mHost, mPort );
}

void UdpClientApp::setupParams()
{
	mFrameRate	= 0.0f;
	mHost		= "127.0.0.1";
	mHostPrev	= mHost;
	mPort		= 2000;
	mPortPrev	= mPort;
	mTestStr	= "Hello, UDP";
	mFont		= Font( "Georgia", 50.0f );
	mText		= "";
	mTextPrev	= mText;
	mTextSize	= Vec2f( getWindowSize() );

	mParams = params::InterfaceGl::create( "Params", Vec2i( 250, 250 ) );
	mParams->addParam( "Host", &mHost );
	mParams->addParam( "Port", &mPort, "step=1" );
	mParams->addParam( "Test Str", &mTestStr );
	mParams->addSeparator();
	mParams->addButton( "Write", bind( &UdpClientApp::write, this ), "key=w" );
}

void UdpClientApp::setupUdp()
{
	mClient = UdpClient::create( io_service() );
	mClient->connectResolveEventHandler(
		[&]() {
			mText ="UdpClient Resolved";
			console() << mText << endl;
		}
	);

	mClient->connectConnectEventHandler(
		[&]( UdpSessionRef session ) {
			mText = "UdpClient Connected";
			console() << mText << endl;

			mSession = session;

			mSession->connectReadCompleteEventHandler(
				[&]() {
					mText = "Session read complete";
					console() << mText << endl;
				}
			);

			mSession->connectReadEventHandler(
				[&]( Buffer buffer ) {
					mText = "Session read " + toString( buffer.getDataSize() ) + " bytes";
					console() << mText << endl;
				}
			);

			mSession->connectWriteEventHandler(
				[&]( size_t numBytes ) {
					mText = "Session write " + toString( numBytes ) + " bytes";
					console() << mText << endl;
				}
			);

			mSession->connectErrorEventHandler( &UdpClientApp::onError, this );
		}
	);

	mClient->connectErrorEventHandler( &UdpClientApp::onError, this );
}

void UdpClientApp::update()
{
	mFrameRate = getFrameRate();

	if ( mPortPrev != mPort ||
		mHostPrev != mHost
	) {
		mClient->connect( mHost, mPort );

		mPortPrev = mPort;
		mHostPrev = mHostPrev;
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

void UdpClientApp::write()
{
	if ( mSession && mSession->getSocket()->is_open() ) {
		mSession->write( UdpSession::stringToBuffer( mTestStr ) );
	}
}

CINDER_APP_BASIC( UdpClientApp, RendererGl )
 