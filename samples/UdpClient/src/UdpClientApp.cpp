#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"

#include "UdpClient.h"
#include "UdpSession.h"

class UdpClientApp : public ci::app::AppBasic
{
public:
	void setup();
	void update();
	void draw();

private:
	void setupParams();
	void setupUdp();
	void onError( std::string errMsg, size_t errCode );
	void write();

	UdpClientRef	mClient;
	UdpSessionRef	mSession;
	std::string		mHost;
	int32_t			mPort;

	ci::params::InterfaceGlRef	mParams;
	std::string					mData;
};

using namespace ci;
using namespace ci::app;
using namespace std;

void UdpClientApp::draw()
{
	gl::clear( Colorf::black() );

	mParams->draw();
}

void UdpClientApp::onError( string errMsg, size_t errCode )
{
	console() << "UDP Error " << errCode << ": " << errMsg << endl;
}

void UdpClientApp::setup()
{
	setupParams();
	setupUdp();

	mClient->connect( mHost, mPort );
}

void UdpClientApp::setupParams()
{
	mHost	= "127.0.0.1";
	mPort	= 13;
	mData	= "Hello, UDP";

	mParams = params::InterfaceGl::create( "Params", Vec2i( 250, 250 ) );
	mParams->addParam( "Host", &mHost );
	mParams->addParam( "Port", &mPort, "step=1" );
	mParams->addParam( "Data", &mData );
	mParams->addSeparator();
	mParams->addButton( "Write", bind( &UdpClientApp::write, this ), "key=w" );
}

void UdpClientApp::setupUdp()
{
	mClient = UdpClient::create( io_service() );
	mClient->connectResolveEventHandler(
		[&]() {
			console() << "UdpClient Resolved" << endl;
		}
	);

	mClient->connectConnectEventHandler(
		[&]( UdpSessionRef session ) {
			console() << "UdpClient Connected" << endl;
			mSession = session;

			mSession->connectCloseEventHandler(
				[&]() {
					console() << "Session closed" << endl;
				}
			);

			mSession->connectReadCompleteEventHandler(
				[&]() {
					console() << "Session read complete" << endl;
				}
			);

			mSession->connectReadEventHandler(
				[&]( Buffer buffer ) {
					console() << "Session read " << buffer.getDataSize() << " bytes" << endl;
				}
			);

			mSession->connectWriteEventHandler(
				[&]( size_t numBytes ) {
					console() << "Session write " << numBytes << " bytes" << endl;
				}
			);

			mSession->connectErrorEventHandler( &UdpClientApp::onError, this );
		}
	);

	mClient->connectErrorEventHandler( &UdpClientApp::onError, this );
}

void UdpClientApp::update()
{
}

void UdpClientApp::write()
{
	if ( mSession && mSession->getSocket()->is_open() ) {
		mSession->write( UdpSession::stringToBuffer( mData ) );
	}
}

CINDER_APP_BASIC( UdpClientApp, RendererGl )
