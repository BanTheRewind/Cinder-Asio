#pragma once

#include "HttpClient.h"

typedef std::shared_ptr<class WebSocketClient>	WebSocketClientRef;

class WebSocketClient : public HttpClient
{
public:
	static WebSocketClientRef	create();
	~WebSocketClient();
	
	void						connect( const std::string& host = "localhost", uint16_t port = 80 );
	
	const std::string&					getKey() const;
	const std::vector<std::string>&		getProtocols() const;
	const std::string&					getVersion() const;
protected:
	WebSocketClient();
	
	std::string					mKey;
	std::vector<std::string>	mProtocols;
	std::string					mVersion;
};
