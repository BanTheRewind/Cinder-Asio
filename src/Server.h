#pragma once

#include "Connection.h"

class Server : public Connection
{
public:
	virtual void	connect( uint16_t port ) = 0;
	virtual void	connect( const std::string& protocol ) = 0;
protected:
	Server(boost::asio::io_service& io )
	: Connection( io )
	{
	}
};
