#include "TcpClient.h"

#include "cinder/app/App.h"
#include "boost/bind.hpp"
#include "cinder/Utilities.h"

using boost::asio::ip::tcp;
using namespace ci;
using namespace std;

TcpClientRef TcpClient::create()
{
	return TcpClientRef( new TcpClient() );
}

TcpClient::TcpClient()
: Client()
{
}

TcpClient::~TcpClient()
{
	if ( mSocket && mSocket->is_open() ) {
		mSocket->close();
	}
}

void TcpClient::connect( const string& host, uint16_t port )
{
	try {
		// Resolve host
		boost::asio::ip::tcp::resolver::query query( host, toString( port ) );
		boost::asio::ip::tcp::resolver resolver( mIoService );
		boost::asio::ip::tcp::resolver::iterator destination = resolver.resolve( query );
		while ( destination != boost::asio::ip::tcp::resolver::iterator() ) {
			mEndpoint = *destination++;
		}
		mSocket = std::shared_ptr<tcp::socket>( new tcp::socket( mIoService ) );
		
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

void TcpClient::onSend( const string& message, const boost::system::error_code& error, 
	std::size_t bytesTransferred )
{
	OutputDebugStringA( ( error.message() + "\n" ).c_str() );
}

void TcpClient::send( uint_fast8_t *buffer, size_t count ) 
{
	if ( mSocket ) {
		mSocket->async_send( boost::asio::buffer( buffer, count ), 
			boost::bind(& TcpClient::onSend, this, "", boost::asio::placeholders::error, count ) 
			);
	}
}
