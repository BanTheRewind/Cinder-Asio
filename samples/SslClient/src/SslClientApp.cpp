/*
* 
* Copyright (c) 2016, Wieden+Kennedy, 
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
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"

#include "SslClient.h"

/*
 * Dedsc here.
 */ 
class SslClientApp : public ci::app::App 
{
public:
	void						draw() override;
	void						setup() override;
	void						update() override;
private:
	SslClientRef				mClient;
	SslSessionRef				mSession;
	std::string					mHost;
	int32_t						mPort;
	std::string					mRequest;
	void						write();
	
	void						onConnect( SslSessionRef session );
	void						onHandshake();
	void						onError( std::string err, size_t bytesTransferred );
	void						onRead( ci::BufferRef buffer );
	void						onWrite( size_t bytesTransferred );
	bool						onVerify( bool preVerified, asio::ssl::verify_context& ctx );
	
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

void SslClientApp::draw()
{
	gl::clear( Colorf::black() );
	gl::setMatricesWindow( getWindowSize() );
	
	if ( mTexture ) {
		gl::draw( mTexture, ivec2( 250, 20 ) );
	}

	mParams->draw();
}

void SslClientApp::onConnect( SslSessionRef session )
{
	mText.push_back( "Connected" );

	// Get the session from the argument and set callbacks.
	// Note that you can use lambdas.
	mSession = session;
	mSession->connectCloseEventHandler( [ & ]()
	{
		mText.push_back( "Disconnected" );
	} );
	mSession->connectErrorEventHandler( &SslClientApp::onError, this );
	mSession->connectHandshakeEventHandler( &SslClientApp::onHandshake, this );
	mSession->connectReadCompleteEventHandler( [ & ]()
	{
		mText.push_back( "Read complete" );
	} );
	mSession->connectReadEventHandler( &SslClientApp::onRead, this );
	mSession->connectWriteEventHandler( &SslClientApp::onWrite, this );

	// Unlike the TCP client, we need to perform a handshake with 
	// the server before writing.
	mSession->handshake();
}

void SslClientApp::onError( string err, size_t bytesTransferred )
{
	string text = "Error";
	if ( !err.empty() ) {
		text += ": " + err;
	}
	mText.push_back( text );
}

void SslClientApp::onHandshake()
{
	mText.push_back( "Handshake successful" );

	// Write data is packaged as a ci::Buffer. This allows 
	// you to send any kind of data. Because it's more common to
	// work with strings, the session object has static convenience 
	// methods for converting between std::string and ci::Buffer.
	mSession->write( SslSession::stringToBuffer( mRequest ) );
}

void SslClientApp::onRead( ci::BufferRef buffer )
{
	mText.push_back( toString( buffer->getSize() ) + " bytes read" );
	
	// Responses are passed in the read callback as ci::Buffer. Use
	// a convenience method on the session object to convert it to
	// a std::string.
	string response	= SslSession::bufferToString( buffer );
	mText.push_back( response );
	
	// Closing the connection after reading mimics the behavior
	// of a HTTP client. To keep the connection open and continue
	// communicating with the server, comment the line below.
	mSession->close();
}

bool SslClientApp::onVerify( bool preVerified, asio::ssl::verify_context& ctx )
{
	// This is your opportunity to modify the verification flag
	// if you don't like what you find in the context.
	return preVerified;
}

void SslClientApp::onWrite( size_t bytesTransferred )
{
	mText.push_back( toString( bytesTransferred ) + " bytes written" );
	
	// After successfully sending your request to the server, you 
	// should expect a response. Start reading immediately.
	mSession->read();
}

void SslClientApp::setup()
{	
	gl::enableAlphaBlending();
	gl::enable( GL_TEXTURE_2D );
	
	mFont		= Font( "Georgia", 24 );
	mFrameRate	= 0.0f;
	mFullScreen	= false;
	mHost		= "localhost";
	mPort		= 443;
	mRequest	= "echo";
		
	mParams = params::InterfaceGl::create( "Params", ivec2( 200, 150 ) );
	mParams->addParam( "Frame rate",	&mFrameRate,					"", true );
	mParams->addParam( "Full screen",	&mFullScreen,					"key=f" );
	mParams->addParam( "Host",			&mHost );
	mParams->addParam( "Port",			&mPort,
					  "min=0 max=65535 step=1 keyDecr=p keyIncr=P" );
	mParams->addParam( "Request",		&mRequest );
	mParams->addButton( "Write", bind(	&SslClientApp::write, this ),	"key=w" );
	mParams->addButton( "Quit", bind(	&SslClientApp::quit, this ),	"key=q" );
	
	// Initialize a client by passing a asio::io_service to it.
	// ci::App already has one that it polls on update, so we'll use that.
	// You can use your own io_service, but you will have to manage it 
	// manually (i.e., call poll(), poll_one(), run(), etc).
	mClient = SslClient::create( io_service() );

	// Add callbacks to work with the client asynchronously.
	// Note that you can use lambdas.
	mClient->connectConnectEventHandler( &SslClientApp::onConnect, this );
	mClient->connectErrorEventHandler( &SslClientApp::onError, this );
	mClient->connectResolveEventHandler( [ & ]()
	{
		mText.push_back( "Endpoint resolved" );
	} );
	mClient->connectVerifyEventHandler( &SslClientApp::onVerify, this );
}

void SslClientApp::update()
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

void SslClientApp::write()
{
	// This sample is meant to work with only one session at a time.
	if ( !mSession || !mSession->getStream() ) {
		// Before we can write, we need to establish a connection 
		// and create a session. Check out the onConnect method.
		mText.push_back( "Connecting to: " + mHost + ":" + toString( mPort ) );
		mClient->connect( mHost, (uint16_t)mPort );
	}
}

CINDER_APP( SslClientApp, RendererGl )
