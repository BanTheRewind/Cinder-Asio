#include "UdpSession.h"

using namespace ci;
using namespace std;
using asio::ip::udp;

UdpSessionRef UdpSession::create( asio::io_service& io )
{
	return UdpSessionRef { new UdpSession { io } }->shared_from_this();
}

UdpSession::UdpSession( asio::io_service& io )
	: SessionInterface( io )
{
	mSocket = UdpSocketRef { new udp::socket { io } };
}

UdpSession::~UdpSession()
{
}

void UdpSession::read()
{
	read( 512 );
}

void UdpSession::read( size_t bufferSize )
{
	mBufferSize = bufferSize;
	mSocket->async_receive_from( mResponse.prepare( bufferSize ), mEndpointRemote,
		mStrand.wrap( std::bind( &UdpSession::onRead, shared_from_this(),
			std::placeholders::_1,
			std::placeholders::_2 ) ) );
}

void UdpSession::write( const BufferRef& buffer )
{
	ostream stream( &mRequest );
	if ( buffer && buffer->getSize() > 0 ) {
		stream.write( (const char*)buffer->getData(), buffer->getSize() );
	}
	mSocket->async_send( mRequest.data(),
		mStrand.wrap( std::bind( &UdpSession::onWrite, shared_from_this(),
			std::placeholders::_1,
			std::placeholders::_2 ) ) );
	mSocket->set_option( asio::socket_base::broadcast( true ) );
	mEndpointLocal = mSocket->local_endpoint();
	stream.clear();
	mRequest.consume( mRequest.size() );
}

void UdpSession::write( const string& str )
{
	ostream stream( &mRequest );
	if ( !str.empty() ) {
		stream.write( (const char*)&str[ 0 ], str.size() );
	}
	mSocket->async_send( mRequest.data(),
		mStrand.wrap( std::bind( &UdpSession::onWrite, shared_from_this(),
			std::placeholders::_1,
			std::placeholders::_2 ) ) );
	mSocket->set_option( asio::socket_base::broadcast( true ) );
	mEndpointLocal = mSocket->local_endpoint();
	stream.clear();
	mRequest.consume( mRequest.size() );
}


const asio::ip::udp::endpoint& UdpSession::getLocalEndpoint() const
{
	return mEndpointLocal;
}

const asio::ip::udp::endpoint& UdpSession::getRemoteEndpoint() const
{
	return mEndpointRemote;
}

const UdpSocketRef& UdpSession::getSocket() const
{
	return mSocket;
}
