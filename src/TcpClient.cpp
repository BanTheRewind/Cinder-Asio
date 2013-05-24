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
: Client( io )
{
	mSocket = TcpSocketRef( new tcp::socket( io ) );
}

TcpClient::~TcpClient()
{
	disconnect();
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

void TcpClient::disconnect()
{
	if ( mSocket && mSocket->is_open() ) {
		mSocket->close();
	}
}

void TcpClient::read()
{
	boost::asio::async_read( *mSocket, mResponse, 
		boost::asio::transfer_at_least( 1 ), 
		boost::bind( &TcpClient::onRead, shared_from_this(), 
			boost::asio::placeholders::error, 
			boost::asio::placeholders::bytes_transferred ) );
}

void TcpClient::read( const std::string& delim )
{
	boost::asio::async_read_until( *mSocket, mResponse, delim, 
		mStrand.wrap( boost::bind( &TcpClient::onRead, shared_from_this(), 
		boost::asio::placeholders::error, 
		boost::asio::placeholders::bytes_transferred ) ) );
}

void TcpClient::read( size_t bufferSize )
{
	mSocket->async_read_some( mResponse.prepare( bufferSize ), 
		mStrand.wrap( boost::bind( &TcpClient::onRead, shared_from_this(), 
			boost::asio::placeholders::error, 
			boost::asio::placeholders::bytes_transferred ) ) );
}

void TcpClient::wait( size_t millis, bool repeat )
{
	Connection::wait( millis, repeat );
	if ( mTimerInterval > 0 ) {
		mTimer.async_wait( 
			mStrand.wrap( boost::bind( &TcpClient::onWait, shared_from_this(), 
			boost::asio::placeholders::error ) ) );
	}
}

void TcpClient::write( const Buffer& buffer )
{
	ostream stream( &mRequest );
	stream.write( (const char*)buffer.getData(), buffer.getDataSize() );
	boost::asio::async_write( *mSocket, mRequest, 
		mStrand.wrap( boost::bind( &TcpClient::onWrite, shared_from_this(), 
			boost::asio::placeholders::error, 
			boost::asio::placeholders::bytes_transferred ) ) );
	mRequest.consume( mRequest.size() );
}

void TcpClient::onConnect( const boost::system::error_code& err )
{
	if ( err ) {
		mSignalError( err.message(), 0 );
	} else {
		mSignalConnect();
	}
}

void TcpClient::onResolve( const boost::system::error_code& err,
						  tcp::resolver::iterator iter )
{
	if ( err ) {
		mSignalError( err.message(), 0 );
	} else {
		mSignalResolve();
		boost::asio::async_connect( *mSocket, iter, mStrand.wrap( boost::bind( &TcpClient::onConnect, shared_from_this(),
																boost::asio::placeholders::error ) ) );
	}
}
