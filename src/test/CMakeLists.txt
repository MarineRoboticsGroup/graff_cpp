
set (CMAKE_CXX_STANDARD 11)
# https://stackoverflow.com/questions/41251474/how-to-import-zeromq-libraries-in-cmake#41252437
find_package(PkgConfig)
## use pkg-config to get hints for 0mq locations
pkg_check_modules(PC_ZeroMQ QUIET zmq)
find_path(ZeroMQ_INCLUDE_DIR
        NAMES zmq.hpp
        PATHS ${PC_ZeroMQ_INCLUDE_DIRS}
        )

## use the hint from about to find the location of libzmq
find_library(ZeroMQ_LIBRARY
        NAMES zmq
        PATHS ${PC_ZeroMQ_LIBRARY_DIRS}
        )

add_executable(feitor-test main.cpp)
## add the include directory to our compile directives
target_include_directories(feitor-test PUBLIC ${ZeroMQ_INCLUDE_DIR})
## at the 0mq library to our link directive
target_link_libraries(feitor-test PUBLIC ${ZeroMQ_LIBRARY})
