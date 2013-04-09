#include "HttpClient.h"

using namespace ci;
using namespace std;

HttpClientRef HttpClient::create()
{
	return HttpClientRef( new HttpClient() );
}

HttpClient::HttpClient()
: TcpClient(), mHeader( "" ), mHttpVersion( HttpVersion::HTTP_1_0 ),
mMethod( "GET" ), mPath( "/" )
{
	concatenateHeader();
}

HttpClient::~HttpClient()
{
	mHeaders.clear();
	mHeader.clear();
	mPath.clear();
	mMethod.clear();
}

void HttpClient::connect( const string& host, uint16_t port )
{
	TcpClient::connect( host, port );
}

void HttpClient::send( const string& path, const string& method,
					  uint_fast8_t* buffer, size_t count )
{
	bool concat = path != mPath || method != mMethod;
	mPath	= path;
	mMethod	= method;
	if ( concat ) {
		concatenateHeader();
	}
	sendImpl( buffer, count );
}

const string& HttpClient::getHeader() const
{
	return mHeader;
}

void HttpClient::eraseHeaderField( const string& key )
{
	if ( mHeaders.find( key ) != mHeaders.end() ) {
		mHeaders.erase( mHeaders.find( key ) );
		concatenateHeader();
	}
	throw ExcHeaderNotFound( key );
}

const string& HttpClient::getHeaderField( const string& key ) const
{
	if ( mHeaders.find( key ) != mHeaders.end() ) {
		return mHeaders.find( key )->second;
	}
	throw ExcHeaderNotFound( key );
}

void HttpClient::setHeaderField( const string& key, const string& value )
{
	mHeaders[ key ] = value;
	concatenateHeader();
}

void HttpClient::concatenateHeader()
{
	mHeader = mMethod + " " + mPath + " HTTP/";
	switch ( mHttpVersion ) {
		case HTTP_1_0:
			mHeader += "1.0";
			break;
		case HTTP_1_1:
			mHeader += "1.1";
			break;
		case HTTP_2_0:
			mHeader += "2.0";
			break;
	}
	mHeader += "\r\n";
	for ( HeaderMap::const_iterator iter = mHeaders.begin(); iter != mHeaders.end(); ++iter ) {
		mHeader += iter->first + ": " + iter->second + "\r\n";
	}
}

void HttpClient::sendImpl( uint_fast8_t* buffer, size_t count )
{
	// Append header to buffer
	size_t headerSize			= mHeader.size() * sizeof( uint_fast8_t );
	size_t total				= count + headerSize;
	uint_fast8_t* headerBuffer	= new uint_fast8_t[ total ];
	memcpy( headerBuffer, &mHeader[ 0 ], headerSize );
	
	// TODO separator
	
	memcpy( &headerBuffer[ headerSize ], buffer, count );
	
	if ( mSocket ) {
		mSocket->async_send( boost::asio::buffer( headerBuffer, total ),
							boost::bind(& HttpClient::onSend, this, "", boost::asio::placeholders::error, count )
							);
	}
	
	delete [] headerBuffer;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

HttpClient::ExcHeaderNotFound::ExcHeaderNotFound( const string& msg ) throw()
{
	sprintf( mMessage, "Header field not found: %s", msg.c_str() );
}
