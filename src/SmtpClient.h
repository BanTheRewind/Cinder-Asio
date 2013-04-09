#pragma once

#include "Client.h"

typedef std::shared_ptr<class SmtpClient>	SmtpClientRef;

class SmtpClient : public Client
{
public:
	static SmtpClientRef	create();
	~SmtpClient();
	
	void					connect( const std::string& host = "localhost", uint16_t port = 25 );
protected:
	SmtpClient();
	
	void					sendImpl( uint_fast8_t* buffer, size_t count );
};
