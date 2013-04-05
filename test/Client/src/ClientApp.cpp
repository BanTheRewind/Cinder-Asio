#include "cinder/app/AppBasic.h"

class ClientApp : public ci::app::AppBasic 
{
public:
	void draw();
	void setup();
	void update();
};

void ClientApp::draw()
{
	gl::clear( Colorf::black() );
}

void ClientApp::setup()
{
}

void ClientApp::update()
{
}

CINDER_APP_BASIC( ClientApp, RendererGl )
