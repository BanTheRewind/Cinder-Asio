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

#include "UdpSession.h"

using namespace ci;
using namespace std;
using boost::asio::ip::udp;

UdpSessionRef UdpSession::create( boost::asio::io_service& io )
{
	return UdpSessionRef( new UdpSession( io ) )->shared_from_this();
}

UdpSession::UdpSession( boost::asio::io_service& io )
	: SessionInterface( io )
{
	mSocket = UdpSocketRef( new udp::socket( io ) );
}

UdpSession::~UdpSession()
{
}

void UdpSession::read()
{
	read( 512 );
}

void UdpSession::read( size_t bufferSize )
{
	mBufferSize = bufferSize;
	mSocket->async_receive_from( mResponse.prepare( bufferSize ), mEndpointRemote,
		boost::bind( &UdpSession::onRead, shared_from_this(), 
			boost::asio::placeholders::error, 
			boost::asio::placeholders::bytes_transferred ) );
}

void UdpSession::write( const Buffer& buffer )
{
	ostream stream( &mRequest );
	if ( buffer && buffer.getDataSize() > 0 ) {
		stream.write( (const char*)buffer.getData(), buffer.getDataSize() );
	}
	mSocket->async_send( mRequest.data(), 
		boost::bind( &UdpSession::onWrite, shared_from_this(), 
			boost::asio::placeholders::error, 
			boost::asio::placeholders::bytes_transferred ) );
	mSocket->set_option( boost::asio::socket_base::broadcast( true ) );
	mEndpointLocal = mSocket->local_endpoint();
	mRequest.consume( mRequest.size() );
}


const boost::asio::ip::udp::endpoint& UdpSession::getLocalEndpoint() const
{
	return mEndpointLocal;
}

const boost::asio::ip::udp::endpoint& UdpSession::getRemoteEndpoint() const
{
	return mEndpointRemote;
}

const UdpSocketRef& UdpSession::getSocket() const
{
	return mSocket;
}
