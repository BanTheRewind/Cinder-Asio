#include "HttpClient.h"

using namespace ci;
using namespace std;

HttpClientRef HttpClient::create()
{
	return HttpClientRef( new HttpClient() );
}

HttpClient::HttpClient()
	: Client()
{
}

HttpClient::~HttpClient()
{
}

void HttpClient::connect( const string& host, uint16_t port )
{
}

void HttpClient::send( uint_fast8_t *buffer, size_t count ) 
{
}
