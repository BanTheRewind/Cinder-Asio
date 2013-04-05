#include "UdpClient.h"

#include "cinder/app/App.h"
#include "boost/bind.hpp"
#include "cinder/Utilities.h"

using boost::asio::ip::udp;
using namespace ci;
using namespace std;

UdpClientRef UdpClient::create()
{
	return UdpClientRef( new UdpClient() );
}

UdpClient::UdpClient()
: Client()
{
}

UdpClient::~UdpClient()
{
	if ( mSocket && mSocket->is_open() ) {
		mSocket->close();
	}
}

void UdpClient::connect( const string& host, uint16_t port )
{
	try {
		// Resolve host
		boost::asio::ip::udp::resolver::query query( host, toString( port ) );
		boost::asio::ip::udp::resolver resolver( mIoService );
		boost::asio::ip::udp::resolver::iterator destination = resolver.resolve( query );
		while ( destination != boost::asio::ip::udp::resolver::iterator() ) {
			mEndpoint = *destination++;
		}
		mSocket = std::shared_ptr<udp::socket>( new udp::socket( mIoService ) );
		
		// Convert address to V4 IP
		boost::asio::ip::address_v4 ip;
		ip = boost::asio::ip::address_v4::from_string( mEndpoint.address().to_string() );
		mEndpoint.address( ip );

		// Open socket
		mSocket->open( mEndpoint.protocol() );
		mSocket->connect( mEndpoint );
		mIoService.run();
		mConnected = true;

	} catch ( const std::exception& ex ) {
		OutputDebugStringA( ex.what() );
		OutputDebugStringA( "\n" );
	}
}

void UdpClient::onSend( const string& message, const boost::system::error_code& error, 
	std::size_t bytesTransferred )
{
	OutputDebugStringA( ( error.message() + "\n" ).c_str() );
}

void UdpClient::send( uint_fast8_t *buffer, size_t count ) 
{
	if ( mSocket ) {
		mSocket->async_send( boost::asio::buffer( buffer, count ), 
			boost::bind(& UdpClient::onSend, this, "", boost::asio::placeholders::error, count ) 
			);
	}
}
