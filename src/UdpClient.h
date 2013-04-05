#pragma once

#include "Client.h"

typedef std::shared_ptr<class UdpClient>	UdpClientRef;

class UdpClient : public Client
{
public:
	static UdpClientRef								create();
	~UdpClient();
	
	void											connect( const std::string& host = "localhost", uint16_t port = 2000 );
	
	void											send( uint_fast8_t* buffer, size_t count );
private:
	UdpClient();
	void											onSend( const std::string& message, 
		const boost::system::error_code& error, std::size_t bytesTransferred );

	std::string										mBuffer;
	boost::asio::ip::udp::endpoint					mEndpoint;
	boost::asio::io_service							mIoService;
	std::shared_ptr<boost::asio::ip::udp::socket>	mSocket;

};
