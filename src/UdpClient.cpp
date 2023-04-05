#include "UdpClient.h"

#include "cinder/Utilities.h"

using namespace ci;
using namespace std;
using asio::ip::udp;

UdpClientRef UdpClient::create( asio::io_service& io )
{
	return UdpClientRef { new UdpClient { io } }->shared_from_this();
}

UdpClient::UdpClient( asio::io_service& io )
	: ClientInterface( io )
{
}

UdpClient::~UdpClient()
{
	mConnectEventHandler = nullptr;
}

void UdpClient::connect( const string& host, uint16_t port )
{
	connect( host, toString( port ) );
}

void UdpClient::connect( const string& host, const string& protocol )
{
	udp::resolver::query query { host, protocol };
	mResolver = UdpResolverRef { new udp::resolver { mStrand.context() } };
	mResolver->async_resolve( query, 
		mStrand.wrap( std::bind( &UdpClient::onResolve, shared_from_this(),
			std::placeholders::_1, std::placeholders::_2 ) ) );
}

UdpResolverRef UdpClient::getResolver() const
{
	return mResolver;
}

void UdpClient::onConnect( UdpSessionRef session, const asio::error_code& err )
{
	if ( err ) {
		if ( mErrorEventHandler != nullptr ) {
			mErrorEventHandler( err.message(), 0 );
		}
	} else {
		if ( mConnectEventHandler != nullptr ) {
			session->mSocket->set_option( asio::socket_base::reuse_address( true ) );
			mConnectEventHandler( session );
		}
	}
}

void UdpClient::onResolve( const asio::error_code& err,
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
		UdpSessionRef session { UdpSession::create( mIoService ) };
		asio::async_connect( *session->mSocket, iter, mStrand.wrap( std::bind( &UdpClient::onConnect,
			shared_from_this(), session, std::placeholders::_1 ) ) );
	}
}

void UdpClient::connectConnectEventHandler( const std::function< void( UdpSessionRef ) >& eventHandler )
{
	mConnectEventHandler = eventHandler;
}
