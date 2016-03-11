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

#include "ClientInterface.h"
#include "SslSession.h"

typedef std::shared_ptr<class SslClient>			SslClientRef;
typedef std::shared_ptr<asio::ip::tcp::resolver>	SslResolverRef;

class SslClient : public ClientInterface, public std::enable_shared_from_this<SslClient>
{
public:
	static SslClientRef	create( asio::io_service& io );
	~SslClient();

	virtual void					connect( const std::string& host, uint16_t port, 
											asio::ssl::context_base::method version = asio::ssl::context_base::method::sslv23 );
	virtual void					connect( const std::string& host, const std::string& protocol, 
											asio::ssl::context_base::method version = asio::ssl::context_base::method::sslv23 );

	template< typename T, typename Y >
	inline void						connectConnectEventHandler( T eventHandler, Y* eventHandlerObject )
	{
		connectConnectEventHandler( std::bind( eventHandler, eventHandlerObject, std::placeholders::_1 ) );
	}

	void							connectConnectEventHandler( const std::function<void( SslSessionRef )>& eventHandler );

	SslResolverRef					getResolver() const;
protected:
	SslClient( asio::io_service& io );
	
	virtual void					onConnect( SslSessionRef session, const asio::error_code& err );
	virtual void					onResolve( const asio::error_code& err, asio::ip::tcp::resolver::iterator iter );

	SslResolverRef					mResolver;
	asio::ssl::context_base::method	mSslMethod;

	std::function<void( SslSessionRef )>	mConnectEventHandler;
};
