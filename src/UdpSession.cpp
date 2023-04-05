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
	mRequest.prepare( buffer->getSize() );
	ostream stream { &mRequest };
	if ( buffer && buffer->getSize() > 0 ) {
		stream << (const char*)buffer->getData() << flush;
	}
	mRequest.commit( buffer->getSize() );
	mSocket->async_send( mRequest.data(),
		mStrand.wrap( std::bind( &UdpSession::onWrite, shared_from_this(),
			std::placeholders::_1,
			std::placeholders::_2 ) ) );
	mRequest.consume( buffer->getSize() );
}

void UdpSession::write( const string& str )
{
	size_t bufferSize { str.size() * sizeof( char ) };
	mRequest.prepare( bufferSize );
	ostream stream { &mRequest };
	if ( !str.empty() ) {
		stream << str.c_str() << flush;
	}
	mRequest.commit( bufferSize );
	mSocket->async_send( mRequest.data(),
		mStrand.wrap( std::bind( &UdpSession::onWrite, shared_from_this(),
			std::placeholders::_1,
			std::placeholders::_2 ) ) );
	mSocket->set_option( asio::socket_base::broadcast( true ) );
	mEndpointLocal = mSocket->local_endpoint();
	mRequest.consume( bufferSize );
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
