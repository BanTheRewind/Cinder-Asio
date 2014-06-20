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

#pragma once

#include "DispatcherInterface.h"
#include "cinder/Buffer.h"

class SessionInterface : public DispatcherInterface
{
public:
	static std::string		bufferToString( const ci::Buffer& buffer );
	static ci::Buffer		stringToBuffer( std::string& value );

	~SessionInterface();

	virtual void			read() = 0;
	virtual void			write( const ci::Buffer& buffer ) = 0;	

	template< typename T, typename Y >
	inline void				connectReadEventHandler( T eventHandler, Y* eventHandlerObject )
	{
		connectReadEventHandler( std::bind( eventHandler, eventHandlerObject, std::placeholders::_1 ) );
	}
	void					connectReadEventHandler( const std::function<void( ci::Buffer )>& eventHandler );

	template< typename T, typename Y >
	inline void				connectReadCompleteEventHandler( T eventHandler, Y* eventHandlerObject )
	{
		connectReadCompleteEventHandler( std::bind( eventHandler, eventHandlerObject ) );
	}
	void					connectReadCompleteEventHandler( const std::function<void ()>& eventHandler );

	template< typename T, typename Y >
	inline void				connectWriteEventHandler( T eventHandler, Y* eventHandlerObject )
	{
		connectWriteEventHandler( std::bind( eventHandler, eventHandlerObject, std::placeholders::_1 ) );
	}
	void					connectWriteEventHandler( const std::function<void( size_t )>& eventHandler );
protected:
	SessionInterface( boost::asio::io_service& io );

	virtual void			onRead( const boost::system::error_code& err,
								   size_t bytesTransferred );
	virtual void			onWrite( const boost::system::error_code& err,
									size_t bytesTransferred );
	
	size_t					mBufferSize;
	boost::asio::streambuf	mRequest;
	boost::asio::streambuf	mResponse;
	
	std::function<void()>				mReadCompleteEventHandler;
	std::function<void( ci::Buffer )>	mReadEventHandler;
	std::function<void( size_t )>		mWriteEventHandler;
};
 