cmake_minimum_required(VERSION 3.0.2)

project(graff_cpp VERSION 0.0.1 LANGUAGES CXX)

include(cmake/pods.cmake)

add_definitions(-std=c++11 -Wall)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(include)

add_subdirectory(include)

pods_install_pkg_config_file(${LIBRARY_NAME}
  LIBS -l${LIBRARY_NAME}
VERSION 0.0.1)

# add_subdirectory(src)
