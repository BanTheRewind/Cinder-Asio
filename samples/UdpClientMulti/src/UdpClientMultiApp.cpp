/*
* 
* Copyright (c) 2014, Wieden+Kennedy,
* Stephen Schieberl, Michael Latzoni
* All rights reserved.
* 
* Redistribution and use in source and binary forms, with or 
* without modification, are permitted provided that the following 
* conditions are met:
* 
* Redistributions of source code must retain the above copyright 
* notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright 
* notice, this list of conditions and the following disclaimer in 
* the documentation and/or other materials provided with the 
* distribution.
* 
* Neither the name of the Ban the Rewind nor the names of its 
* contributors may be used to endorse or promote products 
* derived from this software without specific prior written 
* permission.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
* COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* 
*/

#include "cinder/app/AppBasic.h"
#include "cinder/Font.h"
#include "cinder/gl/Texture.h"
#include "cinder/params/Params.h"

#include "UdpClient.h"
#include "UdpClientEventHandlerInterface.h"
#include "UdpServer.h"
#include "UdpServerEventHandlerInterface.h"
#include "UdpSessionEventHandlerInterface.h"

/*
 * This application demonstrates how to extend event handlers 
 * to create a UDP client which can send and receive data. 
 * This is meant to communicate with the UdpServerMulti sample 
 * running on the same machine.
 */ 
class UdpClientMultiApp :
public ci::app::AppBasic,
public UdpClientEventHandlerInterface,
public UdpServerEventHandlerInterface,
public UdpSessionEventHandlerInterface
{
public:
	void						draw();
	void						setup();
	void						update();
private:
	void						accept();
	UdpClientRef				mClient;
	std::string					mHost;
	int32_t						mPortLocal;
	int32_t						mPortRemote;
	std::string					mRequest;
	UdpServerRef				mServer;
	UdpSessionRef				mSessionRead;
	UdpSessionRef				mSessionWrite;
	void						write();

	// These methods implement event handlers
	void						onAccept( UdpSessionRef session );
	void						onConnect( UdpSessionRef session );
	void						onError( std::string err, size_t bytesTransferred );
	void						onRead( ci::Buffer buffer );
	void						onReadComplete();
	void						onResolve();
	void						onWrite( size_t bytesTransferred );

	ci::Font					mFont;
	std::vector<std::string>	mText;
	ci::gl::TextureRef			mTexture;

	float						mFrameRate;
	bool						mFullScreen;
	ci::params::InterfaceGlRef	mParams;
};

#include "cinder/app/RendererGl.h"
#include "cinder/Text.h"
#include "cinder/Utilities.h"

using namespace ci;
using namespace ci::app;
using namespace std;

void UdpClientMultiApp::accept()
{
	if ( mServer ) {
		mServer->accept( (uint16_t)mPortLocal );
		mText.push_back( "Listening on port: " + toString( mPortLocal ) );
	}
}

void UdpClientMultiApp::draw()
{
	gl::viewport( getWindowSize() );
	gl::clear( Colorf::black() );
	gl::setMatricesWindow( getWindowSize() );

	if ( mTexture ) {
		gl::draw( mTexture, Vec2i( 250, 20 ) );
	}

	mParams->draw();
}

void UdpClientMultiApp::onAccept( UdpSessionRef session )
{
	mSessionRead = session;
	mSessionRead->connectErrorEventHandler( &UdpClientMultiApp::onError, this );
	mSessionRead->connectReadCompleteEventHandler( &UdpClientMultiApp::onReadComplete, this );
	mSessionRead->connectReadEventHandler( &UdpClientMultiApp::onRead, this );
	
	mSessionRead->read();
}

void UdpClientMultiApp::onConnect( UdpSessionRef session )
{
	mText.push_back( "Connected" );

	mSessionWrite = session;
	mSessionWrite->connectErrorEventHandler( &UdpClientMultiApp::onError, this );
	mSessionWrite->connectWriteEventHandler( &UdpClientMultiApp::onWrite, this );

	write();
}

void UdpClientMultiApp::onError( string err, size_t bytesTransferred )
{
	string text = "Error";
	if ( !err.empty() ) {
		text += ": " + err;
	}
	 mText.push_back( text );
}

void UdpClientMultiApp::onRead( Buffer buffer )
{
	string text = "Response received: " + SessionInterface::bufferToString( buffer );
	console() << text << endl;
	mText.push_back( text );
}

void UdpClientMultiApp::onReadComplete()
{
	string text = "Read complete";
	console() << text << endl;
	mText.push_back( text );
}

void UdpClientMultiApp::onResolve()
{
	mText.push_back( "Endpoint resolved" );
}

void UdpClientMultiApp::onWrite( size_t bytesTransferred )
{
	mText.push_back( toString( bytesTransferred ) + " bytes written" );
}

void UdpClientMultiApp::setup()
{
	gl::enableAlphaBlending();
	gl::enable( GL_TEXTURE_2D );
	
	mFont		= Font( "Georgia", 24 );
	mFrameRate	= 0.0f;
	mFullScreen	= false;
	mHost		= "127.0.0.1";
	mPortLocal	= 0;
	mPortRemote	= 2000;
	mRequest	= "Hello, server!";
		
	mParams = params::InterfaceGl::create( "Params", Vec2i( 200, 150 ) );
	mParams->addParam( "Frame rate",	&mFrameRate,					"", true );
	mParams->addParam( "Full screen",	&mFullScreen,					"key=f" );
	mParams->addParam( "Host",			&mHost );
	mParams->addParam( "Port",			&mPortRemote,
					  "min=0 max=65535 step=1 keyDecr=p keyIncr=P" );
	mParams->addParam( "Request",		&mRequest );
	mParams->addButton( "Write", bind(	&UdpClientMultiApp::write, this ),	"key=w" );
	mParams->addButton( "Quit", bind(	&UdpClientMultiApp::quit, this ),	"key=q" );

	mClient = UdpClient::create( io_service() );
	mServer = UdpServer::create( io_service() );

	mClient->connectConnectEventHandler( &UdpClientMultiApp::onConnect, this );
	mClient->connectErrorEventHandler( &UdpClientMultiApp::onError, this );
	mClient->connectResolveEventHandler( &UdpClientMultiApp::onResolve, this );

	mServer->connectAcceptEventHandler( &UdpClientMultiApp::onAccept, this );
	mServer->connectErrorEventHandler( &UdpClientMultiApp::onError, this );
}

void UdpClientMultiApp::update()
{	
	mFrameRate = getFrameRate();
	
	// Toggle full screen.
	if ( mFullScreen != isFullScreen() ) {
		setFullScreen( mFullScreen );
		mFullScreen = isFullScreen();
	}

	// Render text.
	if ( !mText.empty() ) {
		TextBox tbox = TextBox().alignment( TextBox::LEFT ).font( mFont ).size( Vec2i( getWindowWidth() - 250, TextBox::GROW ) ).text( "" );
		for ( vector<string>::const_reverse_iterator iter = mText.rbegin(); iter != mText.rend(); ++iter ) {
			tbox.appendText( "> " + *iter + "\n" );
		}
		tbox.setColor( ColorAf( 1.0f, 0.8f, 0.75f, 1.0f ) );
		tbox.setBackgroundColor( ColorAf::black() );
		tbox.setPremultiplied( false );
		mTexture = gl::Texture::create( tbox.render() );
		while ( mText.size() > 75 ) {
			mText.erase( mText.begin() );
		}
	}
}

void UdpClientMultiApp::write()
{
	if ( mSessionWrite && mSessionWrite->getSocket()->is_open() ) {
		Buffer buffer = UdpSession::stringToBuffer( mRequest );
		mSessionWrite->write( buffer );
		
		mPortLocal = (int32_t)mSessionWrite->getLocalEndpoint().port();
		accept();
	} else {
		mText.push_back( "Connecting to: " + mHost + ":" + toString( mPortRemote ) );
		mClient->connect( mHost, (uint16_t)mPortRemote );
	}
}

CINDER_APP_BASIC( UdpClientMultiApp, RendererGl )
 