project( "micasa" )

cmake_minimum_required( VERSION 2.8.7 )

INCLUDE( ExternalProject )
INCLUDE( FindPackageHandleStandardArgs )

#
# Set sources directory.
# NOTE: for optional hardware or plugin support sources are added together later on.
#
set( SRC
	src/micasa.cpp
	src/Arguments.cpp
	src/Worker.cpp
	src/Controller.cpp
	src/Event.cpp
	src/Logger.cpp
	src/WebServer.cpp
	src/Hardware.cpp
	src/Device.cpp
	src/Curl.cpp
	src/Database.cpp

	# hardware
	src/hardware/Internal.cpp
	src/hardware/OpenZWave.cpp
	src/hardware/OpenZWaveNode.cpp
	src/hardware/RFXCom.cpp
	src/hardware/PiFace.cpp
	src/hardware/PiFaceBoard.cpp
	src/hardware/P1Meter.cpp
	src/hardware/SolarEdge.cpp
	src/hardware/WeatherUnderground.cpp
	src/hardware/HarmonyHub.cpp
	src/hardware/Domoticz.cpp

	# devices
	src/device/Switch.cpp
	src/device/Counter.cpp
	src/device/Level.cpp
	src/device/Text.cpp
)

#
# Make sure thread support is available.
#
find_package( Threads REQUIRED )

#
# Set debug mode.
#
option( DEBUG "Build debug release" NO )
if ( DEBUG )
	set(CMAKE_BUILD_TYPE Debug)
	add_definitions( -D_DEBUG )
endif()

#
# Add and compile open-zwave project.
#
option( WITH_OPENZWAVE "Build with OpenZWave libraries" YES )
if ( WITH_OPENZWAVE )
	MESSAGE( STATUS "Build with static OpenZWave libraries." )
	ExternalProject_Add(
		OpenZWave

		GIT_REPOSITORY "https://github.com/OpenZWave/open-zwave.git"
		GIT_TAG "v1.4"

		UPDATE_COMMAND ""
		PATCH_COMMAND ""
		TEST_COMMAND ""

		SOURCE_DIR "${CMAKE_SOURCE_DIR}/lib/open-zwave"
		PREFIX "${CMAKE_SOURCE_DIR}/lib/open-zwave-prefix"

		CONFIGURE_COMMAND ""
		BUILD_COMMAND make
		BUILD_IN_SOURCE 1
		INSTALL_COMMAND ""
	)
	SET( OPENZWAVE_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/lib/open-zwave/cpp/src )
	SET( OPENZWAVE_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/lib/open-zwave )

	add_library( OpenZWaveLib STATIC IMPORTED )
	set_target_properties( OpenZWaveLib PROPERTIES IMPORTED_LOCATION ${OPENZWAVE_LIB_DIR}/libopenzwave.a )

	add_definitions( -DWITH_OPENZWAVE )
endif()

#
# Add and compile json project.
#
ExternalProject_Add(
	JSON

	GIT_REPOSITORY "https://github.com/nlohmann/json.git"
	GIT_TAG "v2.0.7"

	UPDATE_COMMAND ""
	PATCH_COMMAND ""
	TEST_COMMAND ""

	SOURCE_DIR "${CMAKE_SOURCE_DIR}/lib/json"
	PREFIX "${CMAKE_SOURCE_DIR}/lib/json-prefix"

	CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
	INSTALL_COMMAND ""
)
SET( JSON_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/lib/json/src )
include_directories( ${JSON_INCLUDE_DIR} )

#
# Set the target directory.
#
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )

#
# Set compiler options.
#
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread" )

#
# Configure executable.
#
add_executable( micasa ${SRC} )

#
# Configure link libraries.
#
target_link_libraries( micasa ${PThreadLib})

#
# OpenZWave needs libudev.
#
if ( WITH_OPENZWAVE )
	IF ( CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" )
		MESSAGE( STATUS "Building on FreeBSD, libudev not needed." )
	else()
		find_library( UDEV NAMES libudev.a )
		IF( UDEV )
			message( STATUS ${UDEV} )
			target_link_libraries( micasa ${UDEV} -lrt )
		else()
			find_library( UDEV NAMES libudev.so )
			IF( UDEV )
				message( STATUS ${UDEV} )
				target_link_libraries( micasa ${UDEV} -lrt )
			else()
				MESSAGE( FATAL_ERROR "Library libudev not found on your system. Try:\nsudo apt-get install libudev-dev" )
			ENDIF(UDEV)
		ENDIF(UDEV)
	ENDIF()
	include_directories( ${OPENZWAVE_INCLUDE_DIR} )
	add_dependencies( micasa OpenZWave )
	target_link_libraries( micasa OpenZWaveLib )
endif()

#
# Check for cURL.
#
FIND_PACKAGE( CURL )
IF( CURL_FOUND )
	MESSAGE( STATUS "cURL libraries found at: ${CURL_LIBRARIES}" )
	MESSAGE( STATUS "cURL includes found at: ${CURL_INCLUDE_DIRS}" )
	INCLUDE_DIRECTORIES( ${CURL_INCLUDE_DIRS} )
	target_link_libraries( micasa ${CURL_LIBRARIES} )
else()
	MESSAGE( FATAL_ERROR "cURL not found on your system. Try:\nsudo apt-get install curl libcurl4-openssl-dev" )
ENDIF( CURL_FOUND )

#
# Check for SQLite3
#
FIND_PATH( SQLITE3_INCLUDE_DIR NAMES sqlite3.h )
FIND_LIBRARY( SQLITE3_LIBRARY NAMES sqlite3 )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( SQLITE3 DEFAULT_MSG SQLITE3_LIBRARY SQLITE3_INCLUDE_DIR )
IF( SQLITE3_FOUND )
	MESSAGE( STATUS "SQLite3 libraries found at: ${SQLITE3_LIBRARY}" )
	MESSAGE( STATUS "SQLite3 includes found at: ${SQLITE3_INCLUDE_DIR}" )
	INCLUDE_DIRECTORIES( ${SQLITE3_INCLUDE_DIR} )
	target_link_libraries( micasa ${SQLITE3_LIBRARY} )
ELSE()
	MESSAGE( FATAL_ERROR "SQLite3 not found on your system. Try:\nsudo apt-get install sqlite3 libsqlite3-dev" )
ENDIF( SQLITE3_FOUND )

#
# Add the install targets.
#
install( TARGETS micasa DESTINATION bin )