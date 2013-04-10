#pragma once

#include "HttpClient.h"

typedef std::shared_ptr<class WebSocketClient>	WebSocketClientRef;

class WebSocketClient : public HttpClient
{
public:
	static WebSocketClientRef		create();
	~WebSocketClient();
	
	void							connect( const std::string& host = "localhost", uint16_t port = 80 );
	
	const std::string&				getKey() const;
	void							setKey( const std::string& value );
	
	const std::vector<std::string>&	getProtocols() const;
	void							setProtocols( const std::vector<std::string>& values );
	
	const std::string&				getVersion() const;
	void							setVersion( const std::string& value );
protected:
	WebSocketClient();
	
	std::vector<std::string>		mProtocols;
};
