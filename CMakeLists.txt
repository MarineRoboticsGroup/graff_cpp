cmake_minimum_required(VERSION 3.5.0)
set (CMAKE_CXX_STANDARD 11)

# project(proj VERSION 0.0.1 DESCRIPTION "GraphSDK wrapper to Julia system.")

# include_directories(include)
include_directories(src/test)

add_subdirectory(src)
