#include "SessionInterface.h"

#include "cinder/Log.h"

using namespace ci;
using namespace std;

string SessionInterface::bufferToString( const BufferRef& buffer )
{
	string s { static_cast<const char*>( buffer->getData() ) };
	if ( s.length() > buffer->getSize() ) {
		s.resize( buffer->getSize() );
	}
	return s;
}

BufferRef SessionInterface::stringToBuffer( string value )
{
	return Buffer::create( &value[ 0 ], value.size() );
}

SessionInterface::SessionInterface( asio::io_service& io )
	: DispatcherInterface( io )
{
}

SessionInterface::~SessionInterface()
{
	mReadCompleteEventHandler = nullptr;
	mReadEventHandler = nullptr;
	mWriteEventHandler = nullptr;
	mRequest.consume( mRequest.size() );
	mResponse.consume( mResponse.size() );
}

void SessionInterface::onRead( const asio::error_code& err, size_t bytesTransferred )
{
	if ( err ) {
		if ( err == asio::error::eof ) {
			if ( mReadCompleteEventHandler != nullptr ) {
				mReadCompleteEventHandler();
			}
		} else {
			if ( mErrorEventHandler != nullptr ) {
				mErrorEventHandler( err.message(), bytesTransferred );
			}
		}
	} else {
		if ( mReadEventHandler != nullptr ) {
			BufferRef buffer { Buffer::create( const_cast<void*>( mResponse.data().data() ), bytesTransferred ) };
			mReadEventHandler( buffer );
		}
		if ( mReadCompleteEventHandler != nullptr &&
			mBufferSize > 0 && bytesTransferred < mBufferSize ) {
			mReadCompleteEventHandler();
		}
	}
}

void SessionInterface::onWrite( const asio::error_code& err, size_t bytesTransferred )
{
	if ( err ) {
		if ( mErrorEventHandler != nullptr ) {
			mErrorEventHandler( err.message(), bytesTransferred );
		}
	} else {
		if ( mWriteEventHandler != nullptr ) {
			mWriteEventHandler( bytesTransferred );
		}
	}
}

void SessionInterface::connectReadEventHandler( const std::function<void( ci::BufferRef )>& eventHandler )
{
	mReadEventHandler = eventHandler;
}

void SessionInterface::connectReadCompleteEventHandler( const std::function<void()>& eventHandler )
{
	mReadCompleteEventHandler = eventHandler;
}

void SessionInterface::connectWriteEventHandler( const std::function<void( size_t )>& eventHandler )
{
	mWriteEventHandler = eventHandler;
}