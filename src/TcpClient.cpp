#include "TcpClient.h"

#include "cinder/Utilities.h"

using namespace ci;
using namespace std;
using boost::asio::ip::tcp;

TcpClientRef TcpClient::create( boost::asio::io_service& io )
{
	return TcpClientRef( new TcpClient( io ) )->shared_from_this();
}

TcpClient::TcpClient( boost::asio::io_service& io )
	: ClientInterface( io )
{
}

void TcpClient::connect( const string& host, uint16_t port )
{
	connect( host, toString( port ) );
}

void TcpClient::connect( const string& host, const string& protocol )
{
	tcp::resolver::query query( host, protocol );
	mResolver = TcpResolverRef( new tcp::resolver( mStrand.get_io_service() ) );
	mResolver->async_resolve( query, 
		mStrand.wrap( boost::bind( &TcpClient::onResolve, shared_from_this(), 
			boost::asio::placeholders::error, boost::asio::placeholders::iterator ) ) );
}

void TcpClient::onConnect( TcpSessionRef session, const boost::system::error_code& err )
{
	if ( err ) {
		mErrorEventHandler( err.message(), 0 );
	} else {
		mConnectEventHandler( session );
	}
}

void TcpClient::onResolve( const boost::system::error_code& err,
						  tcp::resolver::iterator iter )
{
	if ( err ) {
		mErrorEventHandler( err.message(), 0 );
	} else {
		mResolveEventHandler();
		TcpSessionRef session( new TcpSession( mIoService ) );
		boost::asio::async_connect( *session->mSocket, iter, mStrand.wrap( boost::bind( &TcpClient::onConnect, 
			shared_from_this(), session, boost::asio::placeholders::error ) ) );
	}
}

void TcpClient::connectConnectEventHandler( const std::function< void( TcpSessionRef ) >& eventHandler )
{
	mConnectEventHandler		= eventHandler;
}