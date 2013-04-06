#pragma once

#include "Client.h"

typedef std::shared_ptr<class HttpClient>	HttpClientRef;

class HttpClient : public Client
{
public:
	static HttpClientRef	create();
	~HttpClient();
	
	void					connect( const std::string& host = "localhost", uint16_t port = 21 );

	void					send( uint_fast8_t* buffer, size_t count );
protected:
	HttpClient();
};
