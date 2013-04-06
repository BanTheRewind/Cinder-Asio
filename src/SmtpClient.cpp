#include "SmtpClient.h"

using namespace ci;
using namespace std;

SmtpClientRef SmtpClient::create()
{
	return SmtpClientRef( new SmtpClient() );
}

SmtpClient::SmtpClient()
	: Client()
{
}

SmtpClient::~SmtpClient()
{
}

void SmtpClient::connect( const string& host, uint16_t port )
{
}

void SmtpClient::send( uint_fast8_t *buffer, size_t count ) 
{
}
