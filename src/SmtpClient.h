#pragma once

#include "Client.h"

typedef std::shared_ptr<class SmtpClient>	SmtpClientRef;

class SmtpClient : public Client
{
public:
	static SmtpClientRef	create();
	~SmtpClient();
	
	void					connect( const std::string& host = "localhost", uint16_t port = 25 );

	void					send( uint_fast8_t* buffer, size_t count );
protected:
	SmtpClient();
};
