#include "TcpClient.h"

#include "cinder/Utilities.h"

using namespace ci;
using namespace std;
using asio::ip::tcp;

TcpClientRef TcpClient::create( asio::io_service& io )
{
	return TcpClientRef { new TcpClient { io } }->shared_from_this();
}

TcpClient::TcpClient( asio::io_service& io )
	: ClientInterface( io )
{
}

TcpClient::~TcpClient()
{
	mConnectEventHandler = nullptr;
}

void TcpClient::connect( const string& host, uint16_t port )
{
	connect( host, toString( port ) );
}

void TcpClient::connect( const string& host, const string& protocol )
{
	tcp::resolver::query query { host, protocol };
	mResolver = TcpResolverRef( new tcp::resolver( mStrand.context() ) );
	mResolver->async_resolve( query, 
		mStrand.wrap( std::bind( &TcpClient::onResolve, shared_from_this(),
		std::placeholders::_1, std::placeholders::_2 ) ) );
}

TcpResolverRef TcpClient::getResolver() const
{
	return mResolver;
}

void TcpClient::onConnect( TcpSessionRef session, const asio::error_code& err )
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

void TcpClient::onResolve( const asio::error_code& err,
						  tcp::resolver::iterator iter )
{
	if ( err ) {
		if ( mErrorEventHandler != nullptr ) {
			mErrorEventHandler( err.message(), 0 );
		}
	} else {
		if ( mResolveEventHandler != nullptr ) {
			mResolveEventHandler();
		}
		TcpSessionRef session { new TcpSession { mIoService } };
		asio::async_connect( *session->mSocket, iter, 
							 mStrand.wrap( std::bind( &TcpClient::onConnect,
							 shared_from_this(), session, std::placeholders::_1 ) ) );
	}
}

void TcpClient::connectConnectEventHandler( const std::function<void( TcpSessionRef )>& eventHandler )
{
	mConnectEventHandler = eventHandler;
}
