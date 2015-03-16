/*
* 
* Copyright (c) 2015, Wieden+Kennedy,
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

#include "CinderAsio.h"

#include "cinder/app/App.h"
#include "cinder/Font.h"
#include "cinder/gl/Texture.h"
#include "cinder/params/Params.h"

#include "UdpClient.h"

/*
 * This application demonstrates how to create a basic UDP
 * client. This is meant to communicate with the UdpServer
 * sample running on the same machine.
 *
 * To communicate with a host, you must create a client and add a
 * connect event callback. Once connected, the client will pass a 
 * session object through the callback. Use the session to communicate
 * by setting callbacks on it.
 */ 
class UdpClientApp : public ci::app::App
{
public:
	void						draw() override;
	void						setup() override;
	void						update() override;
private:
	UdpClientRef				mClient;
	std::string					mHost;
	int32_t						mPort;
	std::string					mRequest;
	UdpSessionRef				mSession;
	void						write();

	void						onConnect( UdpSessionRef session );
	void						onError( std::string err, size_t bytesTransferred );
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

void UdpClientApp::draw()
{
	gl::clear( Colorf::black() );
	gl::setMatricesWindow( getWindowSize() );

	if ( mTexture ) {
		gl::draw( mTexture, ivec2( 250, 20 ) );
	}

	mParams->draw();
}

void UdpClientApp::onConnect( UdpSessionRef session )
{
	mText.push_back( "Connected" );

	// Get the session from the argument and set callbacks.
	// Note that you can use lambdas.
	mSession = session;
	mSession->connectErrorEventHandler( &UdpClientApp::onError, this );
	mSession->connectWriteEventHandler( &UdpClientApp::onWrite, this );

	write();
}

void UdpClientApp::onError( string err, size_t bytesTransferred )
{
	string text = "Error";
	if ( !err.empty() ) {
		text += ": " + err;
	}
	 mText.push_back( text );
}

void UdpClientApp::onWrite( size_t bytesTransferred )
{
	mText.push_back( toString( bytesTransferred ) + " bytes written" );
}

void UdpClientApp::setup()
{
	gl::enableAlphaBlending();
	gl::enable( GL_TEXTURE_2D );
	
	mFont		= Font( "Georgia", 24 );
	mFrameRate	= 0.0f;
	mFullScreen	= false;
	mHost		= "127.0.0.1";
	mPort		= 2000;
	mRequest	= "Hello, server!";
		
	mParams = params::InterfaceGl::create( "Params", ivec2( 200, 150 ) );
	mParams->addParam( "Frame rate",	&mFrameRate,					"", true );
	mParams->addParam( "Full screen",	&mFullScreen,					"key=f" );
	mParams->addParam( "Host",			&mHost );
	mParams->addParam( "Port",			&mPort,
					  "min=0 max=65535 step=1 keyDecr=p keyIncr=P" );
	mParams->addParam( "Request",		&mRequest );
	mParams->addButton( "Write", bind(	&UdpClientApp::write, this ),	"key=w" );
	mParams->addButton( "Quit", bind(	&UdpClientApp::quit, this ),	"key=q" );

	// Initialize a client by passing a boost::asio::io_service to it.
	// ci::App already has one that it polls on update, so we'll use that.
	// You can use your own io_service, but you will have to manage it 
	// manually (i.e., call poll(), poll_one(), run(), etc).
	mClient = UdpClient::create( io_service() );

	// Add callbacks to work with the client asynchronously.
	// Note that you can use lambdas.
	mClient->connectConnectEventHandler( &UdpClientApp::onConnect, this );
	mClient->connectErrorEventHandler( &UdpClientApp::onError, this );
	mClient->connectResolveEventHandler( [ & ]() 
	{
		mText.push_back( "Endpoint resolved" );
	} );
}

void UdpClientApp::update()
{	
	mFrameRate = getFrameRate();
	
	// Toggle full screen.
	if ( mFullScreen != isFullScreen() ) {
		setFullScreen( mFullScreen );
		mFullScreen = isFullScreen();
	}

	// Render text.
	if ( !mText.empty() ) {
		TextBox tbox = TextBox().alignment( TextBox::LEFT ).font( mFont ).size( ivec2( getWindowWidth() - 250, TextBox::GROW ) ).text( "" );
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

void UdpClientApp::write()
{
	// This sample is meant to work with only one session at a time.
	if ( mSession && mSession->getSocket()->is_open() ) {
		// Write data is packaged as a ci::Buffer. This allows 
		// you to send any kind of data. Because it's more common to
		// work with strings, the session object has static convenience 
		// methods for converting between std::string and ci::Buffer.
		Buffer buffer = UdpSession::stringToBuffer( mRequest );
		mSession->write( buffer );
	} else {
		// Before we can write, we need to establish a connection 
		// and create a session. Check out the onConnect method.
		mText.push_back( "Connecting to: " + mHost + ":" + toString( mPort ) );
		mClient->connect( mHost, (uint16_t)mPort );
	}
}

CINDER_APP( UdpClientApp, RendererGl )
 