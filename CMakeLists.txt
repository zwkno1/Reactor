cmake_minimum_required(VERSION 3.4)

project(Reactor)

set(CMAKE_CXX_FLAGS "-std=c++14")
add_definitions("-Wall -Werror")

find_package(Threads REQUIRED)
set(CMAKE_BUILD_TYPE Debug)

include_directories(.)
aux_source_directory(. SRCS)
aux_source_directory(tcp SRCS)
aux_source_directory(udp SRCS)
#message("SRCS=${SRCS}")

add_executable(
	echo_server_udp
	test/echo_server_udp.cpp
	${SRCS}
	)
	
add_executable(
	echo_server_tcp
	test/echo_server_tcp.cpp
	${SRCS}
	)
