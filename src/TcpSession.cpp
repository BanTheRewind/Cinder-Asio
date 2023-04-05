#include "TcpSession.h"

using namespace ci;
using namespace std;
using asio::ip::tcp;

TcpSessionRef TcpSession::create( asio::io_service& io )
{
	return TcpSessionRef { new TcpSession { io } }->shared_from_this();
}

TcpSession::TcpSession( asio::io_service& io )
	: SessionInterface( io )
{
	mSocket = TcpSocketRef { new tcp::socket { io } };
}

TcpSession::~TcpSession()
{
	mCloseEventHandler = nullptr;
	close();
}

void TcpSession::close()
{
	if ( mSocket != nullptr && mSocket->is_open() ) {
		asio::error_code err;
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

void TcpSession::read()
{
	asio::async_read( *mSocket, mResponse,
		asio::transfer_at_least( 1 ),
		mStrand.wrap( std::bind( &TcpSession::onRead, shared_from_this(),
			std::placeholders::_1,
			std::placeholders::_2 ) ) );
	mSocket->set_option( asio::socket_base::reuse_address( true ) );
}

void TcpSession::read( const std::string& delim )
{
	asio::async_read_until( *mSocket, mResponse, delim,
		mStrand.wrap( std::bind( &TcpSession::onRead, shared_from_this(),
			std::placeholders::_1,
			std::placeholders::_2 ) ) );
}

void TcpSession::read( size_t bufferSize )
{
	mSocket->async_read_some( mResponse.prepare( bufferSize ),
		mStrand.wrap( std::bind( &TcpSession::onRead, shared_from_this(),
			std::placeholders::_1,
			std::placeholders::_2 ) ) );
}

void TcpSession::write( const BufferRef& buffer )
{
	ostream stream( &mRequest );
	if ( buffer && buffer->getSize() > 0 ) {
		stream.write( (const char*)buffer->getData(), buffer->getSize() );
	}
	asio::async_write( *mSocket, mRequest,
		mStrand.wrap( std::bind( &TcpSession::onWrite, shared_from_this(),
			std::placeholders::_1,
			std::placeholders::_2 ) ) );
	stream.clear();
	mRequest.consume( mRequest.size() );
}

void TcpSession::write( const string& str )
{
	ostream stream( &mRequest );
	if ( !str.empty() ) {
		stream.write( (const char*)&str[ 0 ], str.size() );
	}
	asio::async_write( *mSocket, mRequest,
		mStrand.wrap( std::bind( &TcpSession::onWrite, shared_from_this(),
			std::placeholders::_1,
			std::placeholders::_2 ) ) );
	stream.clear();
	mRequest.consume( mRequest.size() );
}

const TcpSocketRef& TcpSession::getSocket() const
{
	return mSocket;
}

void TcpSession::connectCloseEventHandler( const std::function<void()>& eventHandler )
{
	mCloseEventHandler = eventHandler;
}

void TcpSession::onClose( const asio::error_code& err )
{
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
