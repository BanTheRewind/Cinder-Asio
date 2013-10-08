#include "UdpServer.h"
#include "cinder/app/App.h"

using namespace ci;
using namespace std;
using boost::asio::ip::udp;

UdpServerRef UdpServer::create( boost::asio::io_service& io )
{
	return UdpServerRef( new UdpServer( io ) )->shared_from_this();
}

UdpServer::UdpServer( boost::asio::io_service& io )
: ServerInterface( io )
{
}

UdpServer::~UdpServer()
{
	cancel();
}

void UdpServer::accept( uint16_t port )
{
	app::console() << "UdpServer accept: " << port << endl;

	UdpSessionRef session = UdpSession::create( mIoService );

	boost::system::error_code errCode;
	session->mSocket->open( boost::asio::ip::udp::v4(), errCode );
	
	if ( errCode ) {
		mErrorEventHandler( errCode.message(), 0 );
	} else {
		session->mSocket->bind( udp::endpoint( udp::v4(), port ), errCode );
		if ( errCode ) {
			mErrorEventHandler( errCode.message(), 0 );
		} else {
			mAcceptEventHandler( session );
		}
	}
}

void UdpServer::cancel()
{
	mCancelEventHandler();
}

void UdpServer::connectAcceptEventHandler( const std::function<void( UdpSessionRef )>& eventHandler )
{
	mAcceptEventHandler = eventHandler;
}
	