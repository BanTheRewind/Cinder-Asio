/*
* 
* Copyright (c) 2015, Wieden+Kennedy,
* Stephen Schieberl
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

#include "UdpClient.h"
#include "UdpClientEventHandlerInterface.h"
#include "UdpServer.h"
#include "UdpServerEventHandlerInterface.h"
#include "UdpSessionEventHandler.h"

/*
 * This application demonstrates how to extend event handlers
 * to create a UDP server which can speak to multiple clients. 
 * This is meant to communicate with the UdpClientMulti sample running
 * on the same machine.
 */
class MultiUdpServerApp :
public ci::app::App,
public UdpClientEventHandlerInterface,
public UdpServerEventHandlerInterface
{
public:
	void						draw() override;
	void						setup() override;
	void						update() override;
private:
	typedef std::map<UdpSessionRef, UdpSessionEventHandler> UdpSessionEventHandlerMap;

	void						accept();
	int32_t						mPort;
	int32_t						mPortPrev;
	UdpClientRef				mClient;
	UdpServerRef				mServer;
	UdpSessionEventHandlerMap	mUdpSessionEventHandlerMap;
	
	// These methods implement event handler interfaces
	void						onAccept( UdpSessionRef session );
	void						onConnect( UdpSessionRef session );
	void						onError( std::string err, size_t bytesTransferred );
	void						onResolve();
	
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

void MultiUdpServerApp::accept()
{
	if ( mServer ) {
		mServer->accept( (uint16_t)mPort );
		mText.push_back( "Listening on port: " + toString( mPort ) );
	}
}

void MultiUdpServerApp::draw()
{
	gl::clear( Colorf::black() );
	gl::setMatricesWindow( getWindowSize() );
	
	if ( mTexture ) {
		gl::draw( mTexture, ivec2( 250, 20 ) );
	}

	mParams->draw();
}

void MultiUdpServerApp::onAccept( UdpSessionRef session ) 
{
	mUdpSessionEventHandlerMap[ session ]	= UdpSessionEventHandler();
	UdpSessionEventHandler& eventHandler	= mUdpSessionEventHandlerMap.at( session );
	
	session->connectErrorEventHandler( &UdpSessionEventHandler::onError, &eventHandler );
	session->connectReadCompleteEventHandler( &UdpSessionEventHandler::onReadComplete, &eventHandler );
	session->connectReadEventHandler( &UdpSessionEventHandler::onRead, &eventHandler );

	session->read();
}

void MultiUdpServerApp::onConnect( UdpSessionRef session )
{
	string response = "OK";
	BufferRef buffer = SessionInterface::stringToBuffer( response );
	session->connectWriteEventHandler( [ & ]( size_t bytesTransferred )
	{
		accept();
	} );
	session->write( buffer );
	
	mText.push_back( "Response sent" );
	console() << mText.back() << endl;
}

void MultiUdpServerApp::onError( string err, size_t bytesTransferred )
{
	string text = "Error";
	if ( !err.empty() ) {
		text += ": " + err;
	}
	mText.push_back( text );
	console() << mText.back() << endl;
}

void MultiUdpServerApp::onResolve()
{
}

void MultiUdpServerApp::setup()
{
	gl::enableAlphaBlending();
	gl::enable( GL_TEXTURE_2D );

	mFont		= Font( "Georgia", 24 );
	mFrameRate	= 0.0f;
	mFullScreen	= false;
	mPort		= 2000;
	mPortPrev	= mPort;
	
	mParams = params::InterfaceGl::create( "Params", ivec2( 200, 110 ) );
	mParams->addParam( "Frame rate",	&mFrameRate,			"", true );
	mParams->addParam( "Full screen",	&mFullScreen,			"key=f" );
	mParams->addParam( "Port",			&mPort,					"min=0 max=65535 step=1 keyDecr=p keyIncr=P" );
	mParams->addButton( "Quit",			[ & ]() { quit(); },	"key=q" );
	
	mClient = UdpClient::create( io_service() );
	mServer = UdpServer::create( io_service() );

	mClient->connectConnectEventHandler( &MultiUdpServerApp::onConnect, this );
	
	mServer->connectAcceptEventHandler( &MultiUdpServerApp::onAccept, this );
	mServer->connectErrorEventHandler( &MultiUdpServerApp::onError, this );
	
	accept();
}

void MultiUdpServerApp::update()
{
	mFrameRate = getFrameRate();
	
	if ( mFullScreen != isFullScreen() ) {
		setFullScreen( mFullScreen );
		mFullScreen = isFullScreen();
	}
	
	if ( mPortPrev != mPort ) {
		mPortPrev = mPort;
		accept();
	}

	for ( UdpSessionEventHandlerMap::iterator iter = mUdpSessionEventHandlerMap.begin();
		 iter != mUdpSessionEventHandlerMap.end(); ) {
		UdpSessionEventHandler eventHandler = iter->second;
		UdpSessionRef session				= iter->first;
		if ( eventHandler.isReadComplete() ) {
			if ( session ) {
				asio::error_code err;
				string host = session->getRemoteEndpoint().address().to_string( err );
				if ( err ) {
					mText.push_back( "Unable to read remote endpoint's address ( " + toString( err ) + " )" );
					console() << mText.back() << endl;
				}
				uint16_t port = session->getRemoteEndpoint().port();
				
				string request	= eventHandler.getResponse();
				mText.push_back( "Received request from " + host + ":" + toString( port ) + ": " + request );
				console() << mText.back() << endl;
				
				mClient->connect( host, port );
			}
			
			iter = mUdpSessionEventHandlerMap.erase( iter );
		} else {
			++iter;
		}
	}
	
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

CINDER_APP( MultiUdpServerApp, RendererGl )
