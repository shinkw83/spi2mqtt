cmake_minimum_required(VERSION 3.8)

IF (POLICY CMP0077)
	cmake_policy(SET CMP0077 OLD)
ENDIF (POLICY CMP0077)

project(spi2mqtt C CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(Threads REQUIRED)
find_package(cppzmq REQUIRED)

include_directories(
	common/include
	source/App
)

link_directories(
	common/lib
)

file(GLOB _cpp_files "source/App/*.cpp")

add_executable(spi2mqtt
	${_cpp_files}
)

target_link_libraries(spi2mqtt
	cppzmq
	xthread
	xpacket
	json
	boost_system
	log4cplus
)

set_target_properties(spi2mqtt PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/install/bin)
