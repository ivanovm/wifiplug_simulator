cmake_minimum_required (VERSION 2.8.8)

project (WiFiPlugSimulator)

add_compile_options(-std=c++11 -Wall -Wextra -Wsign-conversion)

add_subdirectory(Simple-Web-Server)

add_executable(simulator simulator.cpp)
target_link_libraries(simulator simple-web-server)
find_package(Boost 1.53.0 COMPONENTS system thread filesystem REQUIRED)
target_link_libraries(simulator ${Boost_LIBRARIES})
target_include_directories(simulator PRIVATE ${Boost_INCLUDE_DIR})