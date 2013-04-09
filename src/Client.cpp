#include "Client.h"

using namespace ci;
using namespace std;

Client::Client()
: mConnected( false ), mHost( "" ), mPort( 0 )
{
}

bool Client::isConnected() const
{
	return mConnected;
}

const string& Client::getHost() const
{
	return mHost;
}

uint16_t Client::getPort() const
{
	return mPort;
}

void Client::send( uint_fast8_t* buffer, size_t count )
{
	sendImpl( buffer, count );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

Client::ExcConnection::ExcConnection( const string& msg ) throw()
{
	sprintf( mMessage, "Unable to connect: %s", msg.c_str() );
}
