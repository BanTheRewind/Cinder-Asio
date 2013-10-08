#include "TcpSession.h"

using namespace ci;
using namespace std;
using boost::asio::ip::tcp;

TcpSessionRef TcpSession::create( boost::asio::io_service& io )
{
	return TcpSessionRef( new TcpSession( io ) )->shared_from_this();
}

TcpSession::TcpSession( boost::asio::io_service& io )
	: SessionInterface( io )
{
	mSocket = TcpSocketRef( new tcp::socket( io ) );
}

TcpSession::~TcpSession()
{
	close();
}

void TcpSession::close()
{
	if ( mSocket && mSocket->is_open() ) {
		boost::system::error_code err;
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
	boost::asio::async_read( *mSocket, mResponse, 
		boost::asio::transfer_at_least( 1 ), 
		boost::bind( &TcpSession::onRead, shared_from_this(), 
			boost::asio::placeholders::error, 
			boost::asio::placeholders::bytes_transferred ) );
}

void TcpSession::read( const std::string& delim )
{
	boost::asio::async_read_until( *mSocket, mResponse, delim, 
		mStrand.wrap( boost::bind( &TcpSession::onRead, shared_from_this(), 
		boost::asio::placeholders::error, 
		boost::asio::placeholders::bytes_transferred ) ) );
}

void TcpSession::read( size_t bufferSize )
{
	mSocket->async_read_some( mResponse.prepare( bufferSize ), 
		mStrand.wrap( boost::bind( &TcpSession::onRead, shared_from_this(), 
			boost::asio::placeholders::error, 
			boost::asio::placeholders::bytes_transferred ) ) );
}

void TcpSession::write( const Buffer& buffer )
{
	ostream stream( &mRequest );
	stream.write( (const char*)buffer.getData(), buffer.getDataSize() );
	boost::asio::async_write( *mSocket, mRequest, 
		mStrand.wrap( boost::bind( &TcpSession::onWrite, shared_from_this(), 
			boost::asio::placeholders::error, 
			boost::asio::placeholders::bytes_transferred ) ) );
	mRequest.consume( mRequest.size() );
}

const TcpSocketRef& TcpSession::getSocket() const
{
	return mSocket;
}
