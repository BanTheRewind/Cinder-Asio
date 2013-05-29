#include "DispatcherInterface.h"

DispatcherInterface::DispatcherInterface( boost::asio::io_service& io )
	: mIoService( io ), mStrand( io )
{
}

DispatcherInterface::~DispatcherInterface()
{
	for ( CallbackList::iterator iter = mCallbacks.begin(); iter != mCallbacks.end(); ++iter ) {
		iter->second->disconnect();
	}
	mCallbacks.clear();
}

void DispatcherInterface::removeCallback( uint32_t id )
{
	if ( mCallbacks.find( id ) != mCallbacks.end() ) {
		mCallbacks.find( id )->second->disconnect();
		mCallbacks.erase( id );
	}
}
