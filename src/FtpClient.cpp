#include "FtpClient.h"

using namespace ci;
using namespace std;

FtpClientRef FtpClient::create()
{
	return FtpClientRef( new FtpClient() );
}

FtpClient::FtpClient()
	: Client()
{
}

FtpClient::~FtpClient()
{
}

void FtpClient::connect( const string& host, uint16_t port )
{
}

void FtpClient::send( uint_fast8_t *buffer, size_t count ) 
{
}
