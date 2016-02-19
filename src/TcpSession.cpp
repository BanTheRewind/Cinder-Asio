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

#include "TcpSession.h"

using namespace ci;
using namespace std;
using asio::ip::tcp;

TcpSessionRef TcpSession::create( asio::io_service& io )
{
	return TcpSessionRef( new TcpSession( io ) )->shared_from_this();
}

TcpSession::TcpSession( asio::io_service& io )
	: SessionInterface( io ), mCloseEventHandler( nullptr )
{
	mSocket = TcpSocketRef( new tcp::socket( io ) );
}

TcpSession::~TcpSession()
{
	mCloseEventHandler = nullptr;
	close();
}

void TcpSession::close()
{
	if ( mSocket && mSocket->is_open() ) {
		asio::error_code err;
		mSocket->close( err );
		if ( err ) {
			if ( mErrorEventHandler != nullptr ) {
				mErrorEventHandler( err.message(), 0 );
			}
		} else {
			if ( mCloseEventHandler != nullptr ) {
				mCloseEventHandler();
			}
		}
	}
}

void TcpSession::read()
{
	asio::async_read( *mSocket, mResponse, 
		asio::transfer_at_least( 1 ), 
		mStrand.wrap( boost::bind( &TcpSession::onRead, shared_from_this(), 
			asio::placeholders::error, 
			asio::placeholders::bytes_transferred ) ) );
	mSocket->set_option( asio::socket_base::reuse_address( true ) );
}

void TcpSession::read( const std::string& delim )
{
	asio::async_read_until( *mSocket, mResponse, delim, 
		mStrand.wrap( boost::bind( &TcpSession::onRead, shared_from_this(), 
			asio::placeholders::error, 
			asio::placeholders::bytes_transferred ) ) );
}

void TcpSession::read( size_t bufferSize )
{
	mSocket->async_read_some( mResponse.prepare( bufferSize ), 
		mStrand.wrap( boost::bind( &TcpSession::onRead, shared_from_this(), 
			asio::placeholders::error, 
			asio::placeholders::bytes_transferred ) ) );
}

void TcpSession::write( const BufferRef& buffer )
{
	ostream stream( &mRequest );
	if ( buffer && buffer->getSize() > 0 ) {
		stream.write( (const char*)buffer->getData(), buffer->getSize() );
	}
	asio::async_write( *mSocket, mRequest, 
		mStrand.wrap( boost::bind( &TcpSession::onWrite, shared_from_this(), 
			asio::placeholders::error, 
			asio::placeholders::bytes_transferred ) ) );
	mRequest.consume( mRequest.size() );
}

const TcpSocketRef& TcpSession::getSocket() const
{
	return mSocket;
}

void TcpSession::connectCloseEventHandler( const std::function<void ()>& eventHandler )
{
	mCloseEventHandler = eventHandler;
}

void TcpSession::onClose( const asio::error_code& err )
{
	if ( err ) {
		if ( mErrorEventHandler != nullptr ) {
			mErrorEventHandler( err.message(), 0 );
		}
	} else {
		if ( mCloseEventHandler != nullptr ) {
			mCloseEventHandler();
		}
	}
}
