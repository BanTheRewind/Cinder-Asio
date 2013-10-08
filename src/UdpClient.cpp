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
	: ClientInterface( io ), mConnectEventHandler( nullptr )
{
}

void UdpClient::connect( const string& host, uint16_t port )
{
	connect( host, toString( port ) );
}

void UdpClient::connect( const string& host, const string& protocol )
{
	udp::resolver::query query( host, protocol );
	mResolver = UdpResolverRef( new udp::resolver( mStrand.get_io_service() ) );
	mResolver->async_resolve( query, 
		mStrand.wrap( boost::bind( &UdpClient::onResolve, shared_from_this(), 
			boost::asio::placeholders::error, boost::asio::placeholders::iterator ) ) );
}

void UdpClient::onConnect( UdpSessionRef session, const boost::system::error_code& err )
{
	if ( err ) {
		if ( mErrorEventHandler != nullptr ) {
			mErrorEventHandler( err.message(), 0 );
		}
	} else {
		if ( mConnectEventHandler != nullptr ) {
			mConnectEventHandler( session );
		}
	}
}

void UdpClient::onResolve( const boost::system::error_code& err,
						  udp::resolver::iterator iter )
{
	if ( err ) {
		if ( mErrorEventHandler != nullptr ) {
			mErrorEventHandler( err.message(), 0 );
		}
	} else {
		if ( mResolveEventHandler != nullptr ) {
			mResolveEventHandler();
		}
		UdpSessionRef session( new UdpSession( mIoService ) );
		boost::asio::async_connect( *session->mSocket, iter, mStrand.wrap( boost::bind( &UdpClient::onConnect, 
			shared_from_this(), session, boost::asio::placeholders::error ) ) );
	}
}

void UdpClient::connectConnectEventHandler( const std::function< void( UdpSessionRef ) >& eventHandler )
{
	mConnectEventHandler = eventHandler;
}
