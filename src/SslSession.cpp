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

#include "SslSession.h"

using namespace ci;
using namespace std;
using asio::ip::tcp;

SslSessionRef SslSession::create( asio::io_service& io, SslContextRef ctx )
{
	return SslSessionRef( new SslSession( io, ctx ) )->shared_from_this();
}

SslSession::SslSession( asio::io_service& io, SslContextRef ctx )
	: SessionInterface( io ), mCloseEventHandler( nullptr ), 
	mHandshakeEventHandler( nullptr ), mHandshakeType( SslHandshakeType::client )
{
	
	mStream = SslStreamRef( new asio::ssl::stream<tcp::socket>( io, *ctx ) );
}

SslSession::~SslSession()
{
	mCloseEventHandler = nullptr;
	close();
}

void SslSession::close()
{
	if ( mStream && mStream->lowest_layer().is_open() ) {
		asio::error_code err;
		mStream->lowest_layer().close( err );
		onClose( err );
	}
}

void SslSession::handshake()
{
	if ( mStream ) {
		asio::error_code err;
		mStream->async_handshake( mHandshakeType, 
			mStrand.wrap( boost::bind( &SslSession::onHandshake, shared_from_this(), 
			asio::placeholders::error ) ) );
	}
}

void SslSession::read()
{
	asio::async_read( *mStream, mResponse, 
		asio::transfer_at_least( 1 ), 
		mStrand.wrap( boost::bind( &SslSession::onRead, shared_from_this(), 
			asio::placeholders::error, 
			asio::placeholders::bytes_transferred ) ) );
}

void SslSession::read( const std::string& delim )
{
	asio::async_read_until( *mStream, mResponse, delim, 
		mStrand.wrap( boost::bind( &SslSession::onRead, shared_from_this(), 
			asio::placeholders::error, 
			asio::placeholders::bytes_transferred ) ) );
}

void SslSession::read( size_t bufferSize )
{
	mStream->async_read_some( mResponse.prepare( bufferSize ), 
		mStrand.wrap( boost::bind( &SslSession::onRead, shared_from_this(), 
			asio::placeholders::error, 
			asio::placeholders::bytes_transferred ) ) );
}

void SslSession::write( const BufferRef& buffer )
{
	ostream stream( &mRequest );
	if ( buffer && buffer->getSize() > 0 ) {
		stream.write( (const char*)buffer->getData(), buffer->getSize() );
	}
	asio::async_write( *mStream, mRequest, 
		mStrand.wrap( boost::bind( &SslSession::onWrite, shared_from_this(), 
			asio::placeholders::error, 
			asio::placeholders::bytes_transferred ) ) );
	mRequest.consume( mRequest.size() );
}

const SslStreamRef& SslSession::getStream() const
{
	return mStream;
}

void SslSession::connectCloseEventHandler( const std::function<void ()>& eventHandler )
{
	mCloseEventHandler = eventHandler;
}

void SslSession::connectHandshakeEventHandler( const std::function<void ()>& eventHandler )
{
	mHandshakeEventHandler = eventHandler;
}

void SslSession::onClose( const asio::error_code& err )
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

void SslSession::onHandshake( const asio::error_code& err )
{
	if ( err ) {
		if ( mErrorEventHandler != nullptr ) {
			mErrorEventHandler( err.message(), 0 );
		}
	} else {
		if ( mHandshakeEventHandler != nullptr ) {
			mHandshakeEventHandler();
		}
	}
}
 