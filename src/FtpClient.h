#pragma once

#include "Client.h"

typedef std::shared_ptr<class FtpClient>	FtpClientRef;

class FtpClient : public Client
{
public:
	static FtpClientRef	create();
	~FtpClient();
	
	void					connect( const std::string& host = "localhost", uint16_t port = 21 );

	void					send( uint_fast8_t* buffer, size_t count );
protected:
	FtpClient();
};
