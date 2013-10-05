#include "UdpServer.h"

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
	mSocket		= UdpSocketRef( new udp::socket( mIoService, udp::endpoint( udp::v4(), port ) ) );
	
	read();
}

void UdpServer::cancel()
{
	if ( mSocket && mSocket->is_open() ) {
		boost::system::error_code err;
		mSocket->shutdown( boost::asio::socket_base::shutdown_both, err );
		if ( err ) {
			mErrorEventHandler( err.message(), 0 );
		} else {
			mSocket->close( err );
			if ( err ) {
				cout << "ERROR: " << err.message() << endl;
				mErrorEventHandler( err.message(), 0 );
			} else {
				cout << "Socket closed" << endl;
			}
		}
	}
}

void UdpServer::read()
{
	mSocket->async_receive_from(
		boost::asio::buffer( mData, kMaxLength ),
		mSenderEndPoint,
		boost::bind(
			&UdpServer::onReceiveFrom,
			shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred
		)
	);
}

void UdpServer::process()
{
	cout << "process() " << endl;
}

void UdpServer::onReceiveFrom( const boost::system::error_code& err, size_t bytesReceived )
{
	if ( err == boost::asio::error::eof ) {
		cout << "onReceiveFrom: EOF" << endl;
	} else if ( !err && bytesReceived > 0 ) {
		process();
		read();
	} else {
		cout << "ERROR: " << err.message() << endl;
	}
}