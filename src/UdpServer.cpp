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
	: ServerInterface( io ), mReuseAddress ( false ), mAcceptEventHandler( nullptr )
{
}

UdpServer::~UdpServer()
{
	mAcceptEventHandler = nullptr;
}

void UdpServer::setReuseAddress( bool reuseAddress )
{
	mReuseAddress = reuseAddress;
}

void UdpServer::accept( uint16_t port )
{
	UdpSessionRef session = UdpSession::create( mIoService );

	boost::system::error_code errCode;
	session->mSocket->open( boost::asio::ip::udp::v4(), errCode );

	if ( errCode ) {
		if ( mErrorEventHandler != nullptr ) {
			mErrorEventHandler( errCode.message(), 0 );
		}
	} else {
		if ( mReuseAddress ) {
			boost::asio::socket_base::reuse_address addressOption( true );
			session->mSocket->set_option( addressOption, errCode );
			if ( errCode ) {
				mErrorEventHandler( errCode.message(), 0 );
				return;
			}
		}

		session->mSocket->bind( udp::endpoint( udp::v4(), port ), errCode );
		if ( errCode ) {
			if ( mErrorEventHandler != nullptr ) {
				mErrorEventHandler( errCode.message(), 0 );
			}
		} else {
			if ( mAcceptEventHandler != nullptr ) {
				mAcceptEventHandler( session );
			}
		}
	}
}

void UdpServer::connectAcceptEventHandler( const std::function<void( UdpSessionRef )>& eventHandler )
{
	mAcceptEventHandler = eventHandler;
}
	