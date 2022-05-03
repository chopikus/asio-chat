cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_FLAGS -pthread)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# set the project name
project(AsioChat VERSION 1.0)

# add the executable
find_package( Boost REQUIRED COMPONENTS system coroutine regex)
include_directories( ${Boost_INCLUDE_DIRS} )
add_executable( run main.cpp ) # Example application based on main.cpp

# Alternatively you could use ${Boost_LIBRARIES} here.
target_link_libraries( run ${Boost_SYSTEM_LIBRARY} ${Boost_REGEX_LIBRARY} ${Boost_COROUTINE_LIBRARY})
