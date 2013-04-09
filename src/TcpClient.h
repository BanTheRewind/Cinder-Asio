#pragma once

#include "Client.h"

typedef std::shared_ptr<class TcpClient>	TcpClientRef;

class TcpClient : public Client
{
public:
	static TcpClientRef				create();
	~TcpClient();
	
	virtual void					connect( const std::string& host = "localhost", uint16_t port = 2000 );
protected:
	typedef std::shared_ptr<boost::asio::ip::tcp::socket>	TcpSocketRef;
	
	TcpClient();
	void							onSend( const std::string& message,
										   const boost::system::error_code& error,
										   std::size_t bytesTransferred );

	void							sendImpl( uint_fast8_t* buffer, size_t count );
	
	std::string						mBuffer;
	boost::asio::ip::tcp::endpoint	mEndpoint;
	boost::asio::io_service			mIoService;
	TcpSocketRef					mSocket;
};
