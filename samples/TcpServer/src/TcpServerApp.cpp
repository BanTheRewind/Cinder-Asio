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

#include "TcpServer.h"

/*
 * This application demonstrates how to create a basic TCP
 * echo server. This is meant to communicate with the TcpClient
 * sample running on the same machine. This sample only handles a
 * single session.
 *
 * To communicate with a client, you must create a server and add a
 * connect event callback. Once connected, the server will pass a 
 * session object through the callback. Use the session to communicate
 * by setting callbacks on it. Close the session when you no longer 
 * need it.
 */ 
class TcpServerApp : public ci::app::App
{
public:
	void						draw() override;
	void						setup() override;
	void						update() override;
private:
	void						accept();
	TcpServerRef				mServer;
	TcpSessionRef				mSession;
	int32_t						mPort;
	int32_t						mPortPrev;
	
	void						onAccept( TcpSessionRef session );
	void						onCancel();
	void						onClose();
	void						onError( std::string err, size_t bytesTransferred );
	void						onRead( ci::Buffer buffer );
	void						onReadComplete();
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

void TcpServerApp::accept()
{
	if ( mServer ) {
		// This is how you start listening for a connection. Once
		// a connection occurs, a session will be created and passed
		// in through the onAccept method.
		mServer->accept( (uint16_t)mPort );
	
		mText.push_back( "Listening on port: " + toString( mPort ) );
	}
}

void TcpServerApp::draw()
{
	gl::clear( Colorf::black() );
	gl::setMatricesWindow( getWindowSize() );
	
	if ( mTexture ) {
		gl::draw( mTexture, ivec2( 250, 20 ) );
	}

	mParams->draw();
}

void TcpServerApp::onAccept( TcpSessionRef session )
{
	mText.push_back( "Connected" );
	
	// Get the session from the argument and set callbacks.
	// Note that you can use lambdas.
	mSession = session;
	mSession->connectCloseEventHandler( [ & ]() {
		mText.push_back(  "Session closed" );
	} );
	mSession->connectErrorEventHandler( &TcpServerApp::onError, this );
	mSession->connectReadCompleteEventHandler( &TcpServerApp::onReadComplete, this );
	mSession->connectReadEventHandler( &TcpServerApp::onRead, this );
	mSession->connectWriteEventHandler( &TcpServerApp::onWrite, this );
	
	// Start reading data from the client. 
	mSession->read();
}

void TcpServerApp::onCancel()
{
	mText.push_back( "Canceled" );

	// Automatically listen on cancel.
	accept();
}

void TcpServerApp::onClose()
{
	mText.push_back( "Disconnected" );
	
	// Automatically listen on close.
	accept();
}

void TcpServerApp::onError( string err, size_t bytesTransferred )
{
	string text = "Error";
	if ( !err.empty() ) {
		text += ": " + err;
	}
	mText.push_back( text );
}

void TcpServerApp::onRead( ci::Buffer buffer )
{
	mText.push_back( toString( buffer.getDataSize() ) + " bytes read" );

	// Data is packaged as a ci::Buffer. This allows 
	// you to send any kind of data. Because it's more common to
	// work with strings, the session object has static convenience 
	// methods for converting between std::string and ci::Buffer.
	string response	= TcpSession::bufferToString( buffer );
	mText.push_back( response );
	
	// We're simply echoing what the client sent us. Write the response
	// back to them to acknowledge the succesful communication.
	mSession->write( TcpSession::stringToBuffer( response ) );
}

// This event is triggered when the connection is closed
// remotely. The TcpClient sample is written to close the session
// after it reads the echo, meaning that this will be fired when
// a round of communication is complete.
void TcpServerApp::onReadComplete()
{
	mText.push_back( "Read complete" );

	// Close the session.
	mSession->close();

	// Cancel any remaining IO operations (there shouldn't be any).
	// This will tell the server to start listening for the next 
	// connection. See the onCancel method.
	mServer->cancel();
}

void TcpServerApp::onWrite( size_t bytesTransferred )
{
	mText.push_back( toString( bytesTransferred ) + " bytes written" );
	
	// Read after writing to look for an EOF, or disconnect signal
	// from the client.
	mSession->read();
}

void TcpServerApp::setup()
{	
	gl::enableAlphaBlending();
	gl::enable( GL_TEXTURE_2D );

	mFont		= Font( "Georgia", 24 );
	mFrameRate	= 0.0f;
	mFullScreen	= false;
	mPort		= 2000;
	mPortPrev	= mPort;
	
	mParams = params::InterfaceGl::create( "Params", ivec2( 200, 110 ) );
	mParams->addParam( "Frame rate",	&mFrameRate,					"", true );
	mParams->addParam( "Full screen",	&mFullScreen,					"key=f" );
	mParams->addParam( "Port",			&mPort,
					  "min=0 max=65535 step=1 keyDecr=p keyIncr=P" );
	mParams->addButton( "Quit", bind(	&TcpServerApp::quit, this ),	"key=q" );
	
	// Initialize a server by passing a boost::asio::io_service to it.
	// ci::App already has one that it polls on update, so we'll use that.
	// You can use your own io_service, but you will have to manage it 
	// manually (i.e., call poll(), poll_one(), run(), etc).
	mServer = TcpServer::create( io_service() );

	// Add callbacks to work with the server asynchronously.
	mServer->connectAcceptEventHandler( &TcpServerApp::onAccept, this );
	mServer->connectCancelEventHandler( &TcpServerApp::onCancel, this );
	mServer->connectErrorEventHandler( &TcpServerApp::onError, this );
	
	// Start listening.
	accept();
}

void TcpServerApp::update()
{
	mFrameRate = getFrameRate();
	
	// Toggle full screen.
	if ( mFullScreen != isFullScreen() ) {
		setFullScreen( mFullScreen );
		mFullScreen = isFullScreen();
	}
	
	// Close the connection when the port changes. This will 
	// automatically reset the server to listen on the new port.
	// See the onCancel method.
	if ( mPortPrev != mPort ) {
		mPortPrev = mPort;
		mServer->cancel();
		// NOTE: IIS (Windows) returns an error message during this operation.
		//		 It can be ignored. Your server will still work after 
		//		 switching ports.
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

CINDER_APP( TcpServerApp, RendererGl )
