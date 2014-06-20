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

#include "TcpClient.h"

#include "cinder/Utilities.h"

using namespace ci;
using namespace std;
using boost::asio::ip::tcp;

TcpClientRef TcpClient::create( boost::asio::io_service& io )
{
	return TcpClientRef( new TcpClient( io ) )->shared_from_this();
}

TcpClient::TcpClient( boost::asio::io_service& io )
	: ClientInterface( io ), mConnectEventHandler( nullptr )
{
}

TcpClient::~TcpClient()
{
	mConnectEventHandler = nullptr;
}

void TcpClient::connect( const string& host, uint16_t port )
{
	connect( host, toString( port ) );
}

void TcpClient::connect( const string& host, const string& protocol )
{
	tcp::resolver::query query( host, protocol );
	mResolver = TcpResolverRef( new tcp::resolver( mStrand.get_io_service() ) );
	mResolver->async_resolve( query, 
		mStrand.wrap( boost::bind( &TcpClient::onResolve, shared_from_this(), 
			boost::asio::placeholders::error, boost::asio::placeholders::iterator ) ) );
}

TcpResolverRef TcpClient::getResolver() const
{
	return mResolver;
}

void TcpClient::onConnect( TcpSessionRef session, const boost::system::error_code& err )
{
	if ( err ) {
		if ( mErrorEventHandler != nullptr ) {
			mErrorEventHandler( err.message(), 0 );
		}
	} else {
		if ( mConnectEventHandler != nullptr ) {
			mConnectEventHandler( session );
		}
	}
}

void TcpClient::onResolve( const boost::system::error_code& err,
						  tcp::resolver::iterator iter )
{
	if ( err ) {
		if ( mErrorEventHandler != nullptr ) {
			mErrorEventHandler( err.message(), 0 );
		}
	} else {
		if ( mResolveEventHandler != nullptr ) {
			mResolveEventHandler();
		}
		TcpSessionRef session( new TcpSession( mIoService ) );
		boost::asio::async_connect( *session->mSocket, iter, mStrand.wrap( boost::bind( &TcpClient::onConnect, 
			shared_from_this(), session, boost::asio::placeholders::error ) ) );
	}
}

void TcpClient::connectConnectEventHandler( const std::function<void( TcpSessionRef )>& eventHandler )
{
	mConnectEventHandler = eventHandler;
}