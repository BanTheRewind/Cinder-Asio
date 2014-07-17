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

#include "TcpServer.h"

#include "cinder/Utilities.h"

using namespace ci;
using namespace std;
using boost::asio::ip::tcp;

TcpServerRef TcpServer::create( boost::asio::io_service& io )
{
	return TcpServerRef( new TcpServer( io ) )->shared_from_this();
}

TcpServer::TcpServer( boost::asio::io_service& io )
	: ServerInterface( io ), mAcceptEventHandler( nullptr ), mCancelEventHandler( nullptr )
{
}

TcpServer::~TcpServer()
{
	mAcceptEventHandler = nullptr;
	mCancelEventHandler = nullptr;
	cancel();
}

void TcpServer::accept( uint16_t port )
{
	if ( mAcceptor ) {
		mAcceptor.reset();
	}
	mAcceptor				= TcpAcceptorRef( new tcp::acceptor( mIoService, tcp::endpoint( tcp::v4(), port) ) );
	TcpSessionRef session	= TcpSession::create( mIoService );
	
	mAcceptor->async_accept( *session->mSocket, 
		mStrand.wrap( boost::bind( &TcpServer::onAccept, shared_from_this(), 
			session, boost::asio::placeholders::error ) ) );
}

void TcpServer::cancel()
{
	if ( mAcceptor ) {
		boost::system::error_code err;
		mAcceptor->cancel( err );
		if ( err ) {
			if ( mErrorEventHandler != nullptr ) {
				mErrorEventHandler( err.message(), 0 );
			}
		} else {
			if ( mCancelEventHandler != nullptr ) {
				mCancelEventHandler();
			}
		}
	}
}

TcpAcceptorRef TcpServer::getAcceptor() const
{
	return mAcceptor;
}

void TcpServer::connectAcceptEventHandler( const std::function<void( TcpSessionRef )>& eventHandler )
{
	mAcceptEventHandler = eventHandler;
}

void TcpServer::connectCancelEventHandler( const std::function<void()>& eventHandler )
{
	mCancelEventHandler = eventHandler;
}

void TcpServer::onAccept( TcpSessionRef session, const boost::system::error_code& err )
{
	if ( err ) {
		if ( mErrorEventHandler != nullptr ) {
			mErrorEventHandler( err.message(), 0 );
		}
	} else {
		if ( mAcceptEventHandler != nullptr ) {
			mAcceptEventHandler( session );
		}
	}
}
