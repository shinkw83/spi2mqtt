cmake_minimum_required(VERSION 3.8)

project(mqttcpp C CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

file(GLOB srcs "*.cpp")
file(GLOB hdrs "*.h")

add_library(${PROJECT_NAME} ${srcs})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../sioclog ${CMAKE_CURRENT_SOURCE_DIR}/../../include ${CMAKE_CURRENT_SOURCE_DIR}/../json)

install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(FILES ${hdrs} DESTINATION include)