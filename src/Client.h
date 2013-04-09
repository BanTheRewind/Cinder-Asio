#pragma once

#ifdef WIN32
    #include "sdkddkver.h"
#endif
#include "boost/asio.hpp"
#include "boost/signals2.hpp"
#include "cinder/Buffer.h"
#include "cinder/Exception.h"

class Client
{
private:
	typedef boost::signals2::connection		Callback;
	typedef std::shared_ptr<Callback>		CallbackRef;
	typedef std::map<uint32_t, CallbackRef>	CallbackList;
public:
	explicit Client();
	
	//! Connects to \a host on port number \a port.
	virtual void		connect( const std::string &host, uint16_t port ) = 0;
	//! Returns true if connected
	bool				isConnected() const;

	//! Sends \a buffer of length \a count.
	void				send( uint_fast8_t* buffer, size_t count );
	
	//! Returns host as string.
	const std::string&	getHost() const;
	//! Returns port number.
	uint16_t			getPort() const;
	
	//! Adds callback with signature void( ci::Buffer ) and returns ID.
	template<typename T, typename Y> 
	inline uint32_t		addCallback( T callback, Y *callbackObject )
	{
		uint32_t id = mCallbacks.empty() ? 0 : mCallbacks.rbegin()->first + 1;
		mCallbacks.insert( std::make_pair( id, CallbackRef( new Callback( mSignal.connect( std::bind( callback, callbackObject, std::_1 ) ) ) ) ) );
		return id;
	}
protected:
	CallbackList									mCallbacks;
	boost::signals2::signal<void ( ci::Buffer )>	mSignal;

	bool				mConnected;
	
	std::string			mHost;
	uint16_t			mPort;
	
	virtual void		sendImpl( uint_fast8_t* buffer, size_t count ) = 0;
public:

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	//! Base class for Client exceptions.
	class Exception : public cinder::Exception
	{
	};
	
	//! Exception expressing connection error.
	class ExcConnection : public Client::Exception {
	public:
		ExcConnection( const std::string &msg ) throw();
		virtual const char* what() const throw()
		{
			return mMessage;
		}
		
	private:
		char mMessage[ 2048 ];
	};
};
