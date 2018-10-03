if( NOT TARGET Cinder-Asio)
	get_filename_component( Cinder-Asio_SOURCE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../src" ABSOLUTE )
	get_filename_component( CINDER_PATH "${CMAKE_CURRENT_LIST_DIR}/../../../.." ABSOLUTE )

	list( APPEND Cinder-Asio_SOURCES
		${Cinder-Asio_SOURCE_PATH}/CinderAsio.h
		${Cinder-Asio_SOURCE_PATH}/ClientEventHandlerInterface.h
		${Cinder-Asio_SOURCE_PATH}/ClientInterface.cpp
		${Cinder-Asio_SOURCE_PATH}/ClientInterface.h
		${Cinder-Asio_SOURCE_PATH}/DispatcherEventHandlerInterface.h
		${Cinder-Asio_SOURCE_PATH}/DispatcherInterface.cpp
		${Cinder-Asio_SOURCE_PATH}/DispatcherInterface.h
		${Cinder-Asio_SOURCE_PATH}/ServerEventHandlerInterface.h
		${Cinder-Asio_SOURCE_PATH}/ServerInterface.cpp
		${Cinder-Asio_SOURCE_PATH}/ServerInterface.h
		${Cinder-Asio_SOURCE_PATH}/SessionEventHandlerInterface.h
		${Cinder-Asio_SOURCE_PATH}/SessionInterface.cpp
		${Cinder-Asio_SOURCE_PATH}/SessionInterface.h
		${Cinder-Asio_SOURCE_PATH}/TcpClient.cpp
		${Cinder-Asio_SOURCE_PATH}/TcpClient.h
		${Cinder-Asio_SOURCE_PATH}/TcpClientEventHandlerInterface.h
		${Cinder-Asio_SOURCE_PATH}/TcpServer.cpp
		${Cinder-Asio_SOURCE_PATH}/TcpServer.h
		${Cinder-Asio_SOURCE_PATH}/TcpServerEventHandlerInterface.h
		${Cinder-Asio_SOURCE_PATH}/TcpSession.cpp
		${Cinder-Asio_SOURCE_PATH}/TcpSession.h
		${Cinder-Asio_SOURCE_PATH}/TcpSessionEventHandlerInterface.h
		${Cinder-Asio_SOURCE_PATH}/UdpClient.cpp
		${Cinder-Asio_SOURCE_PATH}/UdpClient.h
		${Cinder-Asio_SOURCE_PATH}/UdpClientEventHandlerInterface.h
		${Cinder-Asio_SOURCE_PATH}/UdpServer.cpp
		${Cinder-Asio_SOURCE_PATH}/UdpServer.h
		${Cinder-Asio_SOURCE_PATH}/UdpServerEventHandlerInterface.h
		${Cinder-Asio_SOURCE_PATH}/UdpSession.cpp
		${Cinder-Asio_SOURCE_PATH}/UdpSession.h
		${Cinder-Asio_SOURCE_PATH}/UdpSessionEventHandlerInterface.h
		${Cinder-Asio_SOURCE_PATH}/WaitTimer.cpp
		${Cinder-Asio_SOURCE_PATH}/WaitTimer.h
		${Cinder-Asio_SOURCE_PATH}/WaitTimerEventHandlerInterface.h
	)
	
	add_library( Cinder-Asio ${Cinder-Asio_SOURCES} )

	target_include_directories( Cinder-Asio PUBLIC "${Cinder-Asio_SOURCE_PATH}" )
	target_include_directories( Cinder-Asio SYSTEM BEFORE PUBLIC "${CINDER_PATH}/include" )

	if( NOT TARGET cinder )
		    include( "${CINDER_PATH}/proj/cmake/configure.cmake" )
		    find_package( cinder REQUIRED PATHS
		        "${CINDER_PATH}/${CINDER_LIB_DIRECTORY}"
		        "$ENV{CINDER_PATH}/${CINDER_LIB_DIRECTORY}" )
	endif()
	target_link_libraries( Cinder-Asio PRIVATE cinder )
	
endif()



