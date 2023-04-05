#pragma once

#include "SessionInterface.h"

typedef std::shared_ptr<class TcpSession>		TcpSessionRef;
typedef std::shared_ptr<asio::ip::tcp::socket>	TcpSocketRef;

class TcpClient;
class TcpServer;

class TcpSession : public SessionInterface, public std::enable_shared_from_this<TcpSession>
{
public:
	static TcpSessionRef	create( asio::io_service& io );
	~TcpSession();
	
	void					close();
	
	virtual void			read();
	virtual void			read( const std::string& delim );
	virtual void			read( size_t bufferSize );
	
	virtual void			write( const ci::BufferRef& buffer );
	virtual void			write( const std::string& str );

	const TcpSocketRef&		getSocket() const;

	template< typename T, typename Y >
	inline void				connectCloseEventHandler( T eventHandler, Y* eventHandlerObject )
	{
		connectCloseEventHandler( std::bind( eventHandler, eventHandlerObject ) );
	}
	void					connectCloseEventHandler( const std::function<void ()>& eventHandler );
protected:
	TcpSession( asio::io_service& io );

	virtual void			onClose( const asio::error_code& err );

	TcpSocketRef			mSocket { nullptr };

	std::function<void()>	mCloseEventHandler { nullptr };

	friend class			TcpClient;
	friend class			TcpServer;
};
