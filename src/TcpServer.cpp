#include "TcpServer.h"

#include "cinder/Utilities.h"

using namespace ci;
using namespace std;
using boost::asio::ip::tcp;

TcpServerRef TcpServer::create( boost::asio::io_service& io )
{
	return TcpServerRef( new TcpServer( io ) )->shared_from_this();
}

TcpServer::TcpServer( boost::asio::io_service& io )
: ServerInterface( io )
{
}

TcpServer::~TcpServer()
{
	cancel();
}

void TcpServer::accept( uint16_t port )
{
	mAcceptor = TcpAcceptorRef( new tcp::acceptor( mIoService, tcp::endpoint( tcp::v4(), port) ) );
	TcpSessionRef session( new TcpSession( mIoService ) );
	mAcceptor->async_accept( *session->mSocket, 
		mStrand.wrap( boost::bind( &TcpServer::onAccept, shared_from_this(), 
			session, boost::asio::placeholders::error ) ) );
}

void TcpServer::cancel()
{
	boost::system::error_code err;
	mAcceptor->cancel( err );
	if ( err ) {
		mErrorEventHandler( err.message(), 0 );
	} else {
		mCancelEventHandler();
	}
}

void TcpServer::onAccept( TcpSessionRef session, const boost::system::error_code& err )
{
	if ( err ) {
		mErrorEventHandler( err.message(), 0 );
	} else {
		mAcceptEventHandler( session );
	}
}
