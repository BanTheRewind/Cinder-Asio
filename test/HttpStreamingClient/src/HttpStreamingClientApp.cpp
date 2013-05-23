#include <boost/algorithm/string.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/ostream_iterator.hpp>

#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/params/Params.h"
#include "cinder/Buffer.h"
#include "cinder/Thread.h"

#include "ConcurrentQueue.h"
#include "TcpClient.h"

class HttpStreamingClientApp : public ci::app::AppNative {
public:
	typedef std::map<std::string, std::string> HttpHeaderMap;
	typedef std::pair<std::string, std::string> HttpHeaderEntry;
	typedef std::map<std::string, std::string>::iterator HttpHeaderMapIter;
	typedef std::map<std::string, std::string>::const_iterator HttpHeaderMapConstIter;
	
public:
	HttpStreamingClientApp();
	~HttpStreamingClientApp();
	void setup();
	void mouseDown( ci::app::MouseEvent event );
	void update();
	void draw();
	
	friend std::string encode( const std::string& username, const std::string& password );
	
private:
	TcpClientRef				mClient;
	std::string					mHost;
	uint16_t					mPort;
	std::string					mPath;
	std::string					mRequest;
	std::string					mResponse;
	
	bool						mFullScreen;
	float						mFrameRate;
	ci::params::InterfaceGlRef	mParams;
	
	void						send();
	
	//----
	void						streamData();
	bool						mConnected;
	bool						mNewImageIsReady;
	std::string					mBufferRemainder;
	std::string					mMimeBoundary;
	std::string					mContentType;
	std::string					mJpegBuffer;
	ci::Buffer					mImageBuffer;
	uint16_t					mHttpStatus;
	std::thread*				mStreamThread;
	ConcurrentQueue<std::string> mImageDataQueue;
	ci::Surface8u				mSurface_internal;
	ci::Surface8u				mSurface_external;
	ci::gl::Texture				mTemporayTexture;
	std::mutex					mSurfaceMutex;
//	void						parseHeaders( const std::string& response );
	void						parseHeaders( std::stringstream& response_stream );
	HttpHeaderMap				mHttpHeaders;
	//----
	
	void						onConnect();
	void						onError( std::string error, size_t bytesTransferred );
	void						onRead( ci::Buffer buffer );
	void						onReadComplete();
	void						onResolve();
	void						onWrite( size_t bytesTransferred );
	
};

std::string encode( const std::string& username, const std::string& password )
{
	std::string http_credentials = username + ":" + password;
	std::stringstream os;
	// Base64 encode username and password for HTTP access authorization
	typedef	boost::archive::iterators::base64_from_binary<boost::archive::iterators::transform_width<const char*, 6, 8> > Base64Text;
	std::copy(Base64Text(http_credentials.c_str()), Base64Text(http_credentials.c_str() + http_credentials.size()), boost::archive::iterators::ostream_iterator<char>(os));
	return os.str();
}

#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

HttpStreamingClientApp::HttpStreamingClientApp()
:	mStreamThread(NULL), mConnected(false), mNewImageIsReady(false), mFullScreen(false), mFrameRate(0.0), mPort(0)
{
}

HttpStreamingClientApp::~HttpStreamingClientApp()
{
	if (mStreamThread && mStreamThread->joinable()) {
		mClient->disconnect();
		mConnected = false;
		mStreamThread->join();
	}
}

void HttpStreamingClientApp::onConnect()
{
	mConnected = true;
	
	mHttpHeaders.clear();
	mJpegBuffer.clear();
	
	console() << "Connected." << endl;
	mResponse.clear();
	mClient->write( Buffer( &mRequest[ 0 ], mRequest.size() ) );
}

void HttpStreamingClientApp::onError( string error, size_t bytesTransferred )
{
	console() << "Error: " << error << "." << endl;
}

void HttpStreamingClientApp::onReadComplete()
{
	console() << "Read complete." << endl;
	console() << mResponse << endl;
	
	mClient->disconnect();
}

void HttpStreamingClientApp::parseHeaders( std::stringstream& response_stream )
{
	using namespace boost::algorithm;
	
	size_t header_delim, mime_delim;
	string http_version, http_status, http_header;
	string header_field, header_attribute;
	
	response_stream >> http_version;
	response_stream >> mHttpStatus;
	response_stream >> http_status;
	
	if (mHttpStatus == 404) return;
	
	// collect header data
	while (std::getline(response_stream, http_header)) {
		if (http_header == "\r") continue;
		
		header_delim = http_header.find(":");
		if (header_delim != string::npos) {
			header_field = http_header.substr(0, header_delim);
			header_attribute = http_header.substr(header_delim + 1);
			trim(header_field);
			trim(header_attribute);
			console() << header_field << ": " << header_attribute << std::endl;
			mHttpHeaders.insert(HttpHeaderEntry(header_field, header_attribute));
		}
	}
	
	// handle header data
	for (HttpHeaderMapConstIter iter = mHttpHeaders.begin(); iter != mHttpHeaders.end(); ++iter) {
		if (iter->first == "Content-Type") {
			mime_delim = iter->second.find(";");
			if (mime_delim != string::npos) {
				mContentType = iter->second.substr(0, mime_delim);
				trim(mContentType);
				mime_delim = iter->second.find("boundary=");
				if (mime_delim != string::npos) {
					mMimeBoundary = iter->second.substr(mime_delim + 9);
					trim(mMimeBoundary);
				}
				
				if (mContentType == "image/jpeg") {
					// we may continue...
				}
			}
		}
		else {
			console() << "Unparsed field: " << iter->first << std::endl;
		}
	}
}

void HttpStreamingClientApp::onRead( ci::Buffer buffer )
{
	console() << buffer.getDataSize() << " bytes read." << endl;
	
	// Stringify buffer
	string response( static_cast<const char*>( buffer.getData() ) );
	if (mHttpHeaders.empty()) {
		size_t header_end = response.rfind("\r\n\r\n");
		if (header_end != string::npos) {
			stringstream response_stream( response.substr(0,header_end) );
			this->parseHeaders(response_stream);
			response = response.substr(header_end + 4);
		}
	}
	
	mResponse += response;
	
	mClient->read();
	
	//begin processing loop....
	mStreamThread = new std::thread(std::bind(&HttpStreamingClientApp::streamData, this));
	if (!mStreamThread) {
		console() << "could not spawn worker thread." << std::endl;
	}
}

void HttpStreamingClientApp::streamData()
{
	while (mConnected) {
		// collect all the data into a string buffer for processing
		string buffer = mResponse;
		buffer = mBufferRemainder + buffer;	// prepend any extra data from the previous cycle
		
		// scrape out the first chunk of data, up until the next file boundary
		size_t first_index = buffer.rfind(mMimeBoundary);
		// If we can't find the next segment, then we need to start over
		if (first_index == std::string::npos) continue;
		mJpegBuffer = buffer.substr(0,first_index); // here's our JPG data
		
		// Now, we need to find the next file starting point by skipping past the meta data
		mBufferRemainder = buffer.substr(first_index);
		if (boost::starts_with(mBufferRemainder, mMimeBoundary)) {
			size_t index = mBufferRemainder.rfind("\r\n\r\n");
			
			if (index == std::string::npos) continue;	// not sure if we should return just yet...
			
			mBufferRemainder = mBufferRemainder.substr(index + 4);
		}
		
		mImageDataQueue.push(mJpegBuffer);
	}
}

void HttpStreamingClientApp::onResolve()
{
	console() << "Endpoint resolved." << endl;
}

void HttpStreamingClientApp::onWrite( size_t bytesTransferred )
{
	console() << bytesTransferred << " bytes written." << endl;
	
	mClient->read();
}

void HttpStreamingClientApp::send()
{
	mClient->connect( mHost, mPort );
}

void HttpStreamingClientApp::setup()
{
	mHost = "10.0.1.108";
	mPath = "/axis-cgi/mjpg/video.cgi";
	mPort = 80;
	
	mRequest = "GET " + mPath + " HTTP/1.0\r\n";
	mRequest += "Host: " + mHost + "\r\n";
	mRequest += "Accept: */*\r\n";
	mRequest += "Authorization: Basic " + encode("root", "controlgroup") + "=\r\n";
	mRequest += "Connection: close\r\n\r\n";
	
	mParams = params::InterfaceGl::create( "Params", Vec2i( 200, 150 ) );
	mParams->addParam( "Frame rate",	&mFrameRate,					"", true );
	mParams->addParam( "Full screen",	&mFullScreen,					"key=f" );
	mParams->addButton( "Send", bind(	&HttpStreamingClientApp::send, this ),	"key=s" );
	mParams->addButton( "Quit", bind(	&HttpStreamingClientApp::quit, this ),	"key=q" );
	
	mClient = TcpClient::create( io_service() );
	mClient->addConnectCallback( &HttpStreamingClientApp::onConnect, this );
	mClient->addErrorCallback( &HttpStreamingClientApp::onError, this );
	mClient->addReadCallback( &HttpStreamingClientApp::onRead, this );
	mClient->addReadCompleteCallback( &HttpStreamingClientApp::onReadComplete, this );
	mClient->addResolveCallback( &HttpStreamingClientApp::onResolve, this );
	mClient->addWriteCallback( &HttpStreamingClientApp::onWrite, this );
	
	send();
}

void HttpStreamingClientApp::mouseDown( MouseEvent event )
{
}

void HttpStreamingClientApp::update()
{
	mFrameRate = getFrameRate();
	
	if ( mFullScreen != isFullScreen() ) {
		setFullScreen( mFullScreen );
		mFullScreen = isFullScreen();
	}
	
	if (mConnected && !mImageDataQueue.empty()) {
		string* temp_img_data = new (std::nothrow) string();
		if (!temp_img_data) return;
		
		if (mImageDataQueue.try_pop(*temp_img_data) && !temp_img_data->empty()) {
			mImageBuffer.resize(temp_img_data->size());	// can we avoid this??
			mImageBuffer.copyFrom((char*) temp_img_data->c_str(), temp_img_data->size());
			DataSourceBufferRef source = DataSourceBuffer::create(mImageBuffer);
			mSurfaceMutex.lock();
			try {
				mSurface_internal = loadImage(source, ImageSource::Options(), "jpg");
				mSurface_external = mSurface_internal.clone();
				mNewImageIsReady = true;
			}
			catch (...) {
				console() << "AxisCamera: Error loading the image with size " << temp_img_data->size();
			}
			mSurfaceMutex.unlock();
		}
		
		// If we get behind on processing the feed, then we'll force it to catch up a bit
		while(mImageDataQueue.size() > 2) {
			mImageDataQueue.try_pop(*temp_img_data);
		}
		
		delete temp_img_data;
	}
	
	if (mNewImageIsReady) {
		mTemporayTexture = gl::Texture(mSurface_internal);
		gl::draw(mTemporayTexture);
	}
}

void HttpStreamingClientApp::draw()
{
	gl::clear( Colorf::black() );
	
	mParams->draw();
}

CINDER_APP_NATIVE( HttpStreamingClientApp, RendererGl )
