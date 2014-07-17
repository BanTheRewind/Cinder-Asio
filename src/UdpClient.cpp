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

#include "UdpClient.h"

#include "cinder/Utilities.h"

using namespace ci;
using namespace std;
using boost::asio::ip::udp;

UdpClientRef UdpClient::create( boost::asio::io_service& io )
{
	return UdpClientRef( new UdpClient( io ) )->shared_from_this();
}

UdpClient::UdpClient( boost::asio::io_service& io )
	: ClientInterface( io ), mConnectEventHandler( nullptr )
{
}

UdpClient::~UdpClient()
{
	mConnectEventHandler = nullptr;
}

void UdpClient::connect( const string& host, uint16_t port )
{
	connect( host, toString( port ) );
}

void UdpClient::connect( const string& host, const string& protocol )
{
	udp::resolver::query query( host, protocol );
	mResolver = UdpResolverRef( new udp::resolver( mStrand.get_io_service() ) );
	mResolver->async_resolve( query, 
		mStrand.wrap( boost::bind( &UdpClient::onResolve, shared_from_this(), 
			boost::asio::placeholders::error, boost::asio::placeholders::iterator ) ) );
}

UdpResolverRef UdpClient::getResolver() const
{
	return mResolver;
}

void UdpClient::onConnect( UdpSessionRef session, const boost::system::error_code& err )
{
	if ( err ) {
		if ( mErrorEventHandler != nullptr ) {
			mErrorEventHandler( err.message(), 0 );
		}
	} else {
		if ( mConnectEventHandler != nullptr ) {
			session->mSocket->set_option( boost::asio::socket_base::reuse_address( true ) );
			mConnectEventHandler( session );
		}
	}
}

void UdpClient::onResolve( const boost::system::error_code& err,
						  udp::resolver::iterator iter )
{
	if ( err ) {
		if ( mErrorEventHandler != nullptr ) {
			mErrorEventHandler( err.message(), 0 );
		}
	} else {
		if ( mResolveEventHandler != nullptr ) {
			mResolveEventHandler();
		}
		UdpSessionRef session = UdpSession::create( mIoService );
		boost::asio::async_connect( *session->mSocket, iter, mStrand.wrap( boost::bind( &UdpClient::onConnect, 
			shared_from_this(), session, boost::asio::placeholders::error ) ) );
	}
}

void UdpClient::connectConnectEventHandler( const std::function< void( UdpSessionRef ) >& eventHandler )
{
	mConnectEventHandler = eventHandler;
}
