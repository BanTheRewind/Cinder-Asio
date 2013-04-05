#include "Client.h"

Client::Client()
: mConnected( false )
{
}

bool Client::isConnected() const
{
	return mConnected;
}
