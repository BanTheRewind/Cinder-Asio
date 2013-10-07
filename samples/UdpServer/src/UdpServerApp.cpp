#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"

#include "UdpServer.h"

class UdpServerApp : public ci::app::AppBasic
{
public:
	void setup();
	void update();
	void draw();

private:
	UdpServerRef	mServer;
};


using namespace ci;
using namespace ci::app;
using namespace std;

void UdpServerApp::draw()
{
	gl::clear( Colorf::black() );
}

void UdpServerApp::setup()
{
	mServer = UdpServer::create( io_service() );
	mServer->connectErrorEventHandler( 
		[&]( const string& errMsg, size_t errCode ) {
			console() << "UdpServer Error " << errCode << ": " << errMsg << endl;
		} 
	);

	mServer->connectCancelEventHandler( 
		[&]() {
			console() << "UdpServer Cancel" << endl;
		}
	);

	mServer->accept( 13 );
}

void UdpServerApp::update()
{
}

CINDER_APP_BASIC( UdpServerApp, RendererGl )
