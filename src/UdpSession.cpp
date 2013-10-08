#include "UdpSession.h"

using namespace ci;
using namespace std;
using boost::asio::ip::udp;

UdpSessionRef UdpSession::create( boost::asio::io_service& io )
{
	return UdpSessionRef( new UdpSession( io ) )->shared_from_this();
}

UdpSession::UdpSession( boost::asio::io_service& io )
	: SessionInterface( io )
{
	mSocket = UdpSocketRef( new udp::socket( io ) );
}

UdpSession::~UdpSession()
{
	close();
}

void UdpSession::close()
{
	if ( mSocket && mSocket->is_open() ) {
		boost::system::error_code err;
		mSocket->shutdown( boost::asio::socket_base::shutdown_both, err );
		if ( err ) {
			if ( mErrorEventHandler != nullptr ) {
				mErrorEventHandler( err.message(), 0 );
			}
		} else { 
			mSocket->close( err );
			if ( err ) {
				if ( mErrorEventHandler != nullptr ) {
					mErrorEventHandler( err.message(), 0 );
				}
			} else {
				if ( mCloseEventHandler != nullptr ) {
					mCloseEventHandler();
				}
			}
		}
	}
}

void UdpSession::read()
{
	read( 512 );
}

void UdpSession::read( size_t bufferSize )
{
	mSocket->async_receive( mResponse.prepare( bufferSize ), 
		boost::bind( &UdpSession::onRead, shared_from_this(), 
			boost::asio::placeholders::error, 
			boost::asio::placeholders::bytes_transferred ) );
}

void UdpSession::write( const Buffer& buffer )
{
	ostream stream( &mRequest );
	stream.write( (const char*)buffer.getData(), buffer.getDataSize() );
	mSocket->async_send( mRequest.data(), 
		boost::bind( &UdpSession::onWrite, shared_from_this(), 
			boost::asio::placeholders::error, 
			boost::asio::placeholders::bytes_transferred ) );
	mRequest.consume( mRequest.size() );
}

const UdpSocketRef& UdpSession::getSocket() const
{
	return mSocket;
}
