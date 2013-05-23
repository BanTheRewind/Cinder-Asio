#pragma once

#include "Connection.h"

class Client : public Connection
{
public:
	virtual void	connect( const std::string& host, uint16_t port ) = 0;
	virtual void	connect( const std::string& host, const std::string& protocol ) = 0;
protected:
	Client( boost::asio::io_service& io )
	: Connection( io ), mHost( "" )
	{
	}
	
	std::string		mHost;
};
