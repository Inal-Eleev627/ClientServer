cmake_minimum_required (VERSION 3.12)

set(TARGET server)
project(${TARGET})

set(SOURCES
    ./connection.cpp
    ./connection.hpp
    ./server.cpp
    ./server.hpp
    ./main.cpp
)

add_executable (${TARGET} ${SOURCES})
include_directories(${TARGET} ${CMAKE_SOURCE_DIR}/../include)

find_package(Boost 1.68 COMPONENTS date_time system REQUIRED)
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_directories(${TARGET} PUBLIC ${Boost_LIBRARIES})
endif()

if (CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 20)
    set_property(TARGET ${TARGET} PROPERTY CMAKE_CXX_STANDARD_REQUIRED 20)
endif()

add_subdirectory(tests)