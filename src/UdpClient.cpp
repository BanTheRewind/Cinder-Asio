#include "UdpClient.h"

#include "cinder/Utilities.h"

using namespace ci;
using namespace std;
using boost::asio::ip::udp;

UdpClientRef UdpClient::create( boost::asio::io_service& io )
{
	return UdpClientRef( new UdpClient( io ) )->shared_from_this();
}

UdpClient::UdpClient( boost::asio::io_service& io )
: Client( io )
{
	mSocket = UdpSocketRef( new udp::socket( io ) );
}

void UdpClient::connect( const string& host, uint16_t port )
{
	connect( host, toString( port ) );
}

void UdpClient::connect( const string& host, const string& protocol )
{
	udp::resolver::query query( host, protocol );
	mResolver = UdpResolverRef( new udp::resolver( mStrand.get_io_service() ) );
	mResolver->async_resolve( query, mStrand.wrap( boost::bind( &UdpClient::onResolve, shared_from_this(),
											   boost::asio::placeholders::error,
											   boost::asio::placeholders::iterator ) ) );
}

void UdpClient::disconnect()
{
	if ( mSocket && mSocket->is_open() ) {
		mSocket->close();
	}
}

void UdpClient::read()
{
	/*mSocket->async_receive( mResponse, boost::asio::transfer_at_least( 1 ),
							boost::bind( &UdpClient::onRead, shared_from_this(),
										boost::asio::placeholders::error,
										boost::asio::placeholders::bytes_transferred ) );*/
}

void UdpClient::wait( size_t millis, bool repeat )
{
	Connection::wait( millis, repeat );
	if ( mTimerInterval > 0 ) {
		mTimer.async_wait( 
			mStrand.wrap( boost::bind( &UdpClient::onWait, shared_from_this(), 
			boost::asio::placeholders::error ) ) );
	}
}

void UdpClient::write( const Buffer& buffer )
{
	ostream stream( &mRequest );
	stream.write( (const char*)buffer.getData(), buffer.getDataSize() );
	/*mSocket->async_send( mRequest, boost::bind( &UdpClient::onWrite, shared_from_this(),
	 boost::asio::placeholders::error,
	 boost::asio::placeholders::bytes_transferred ) );
	*/
	mRequest.consume( mRequest.size() );
}

void UdpClient::onConnect( const boost::system::error_code& err )
{
	if ( err ) {
		mSignalError( err.message(), 0 );
	} else {
		mSignalConnect();
	}
}

void UdpClient::onResolve( const boost::system::error_code& err,
						  udp::resolver::iterator iter )
{
	if ( err ) {
		mSignalError( err.message(), 0 );
	} else {
		mSignalResolve();
		boost::asio::async_connect( *mSocket, iter,
								  mStrand.wrap( boost::bind( &UdpClient::onConnect, shared_from_this(),
											   boost::asio::placeholders::error ) ) );
	}
}
