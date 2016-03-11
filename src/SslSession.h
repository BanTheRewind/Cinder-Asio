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

#pragma once

#include "SessionInterface.h"
#include "cinder/FileSystem.h"
#include "asio/ssl.hpp"

typedef std::shared_ptr<asio::ssl::context>							SslContextRef;
typedef asio::ssl::stream_base::handshake_type						SslHandshakeType;
typedef asio::ssl::context_base::method								SslMethod;
typedef std::shared_ptr<class SslSession>							SslSessionRef;
typedef std::shared_ptr<asio::ssl::stream<asio::ip::tcp::socket>>	SslStreamRef;

class SslClient;
// class SslServer;

class SslSession : public SessionInterface, public std::enable_shared_from_this<SslSession>
{
public:
	static SslSessionRef	create( asio::io_service& io, SslContextRef ctx );
	~SslSession();
	
	void					close();
	
	virtual void			read();
	virtual void			read( const std::string& delim );
	virtual void			read( size_t bufferSize );
	
	virtual void			handshake();
	virtual void			write( const ci::BufferRef& buffer );

	const SslStreamRef&		getStream() const;

	template< typename T, typename Y >
	inline void				connectCloseEventHandler( T eventHandler, Y* eventHandlerObject )
	{
		connectCloseEventHandler( std::bind( eventHandler, eventHandlerObject ) );
	}
	void					connectCloseEventHandler( const std::function<void ()>& eventHandler );

	template< typename T, typename Y >
	inline void				connectHandshakeEventHandler( T eventHandler, Y* eventHandlerObject )
	{
		connectHandshakeEventHandler( std::bind( eventHandler, eventHandlerObject ) );
	}
	void					connectHandshakeEventHandler( const std::function<void ()>& eventHandler );
protected:
	SslSession( asio::io_service& io, SslContextRef ctx );

	virtual void			onHandshake( const asio::error_code& err );
	virtual void			onClose( const asio::error_code& err );

	SslHandshakeType		mHandshakeType;
	SslStreamRef			mStream;

	std::function<void ()>	mCloseEventHandler;
	std::function<void ()>	mHandshakeEventHandler;

	friend class			SslClient;
	//friend class			SslServer;
};
