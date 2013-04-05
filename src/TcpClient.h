#pragma once

#include "Client.h"

typedef std::shared_ptr<class TcpClient>			TcpClientRef;

class TcpClient : public Client
{
public:
	static TcpClientRef								create();
	~TcpClient();
	
	void											connect( const std::string& host = "localhost", uint16_t port = 2000 );

	void											send( uint_fast8_t* buffer, size_t count );
protected:
	TcpClient();
	void											onSend( const std::string& message, 
		const boost::system::error_code& error, std::size_t bytesTransferred );

	std::string										mBuffer;
	boost::asio::ip::tcp::endpoint					mEndpoint;
	boost::asio::io_service							mIoService;
	std::shared_ptr<boost::asio::ip::tcp::socket>	mSocket;

};
