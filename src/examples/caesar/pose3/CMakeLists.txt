find_package(PkgConfig)
## use pkg-config to get hints for 0mq locations
pkg_check_modules(PC_ZeroMQ QUIET zmq)
find_path(ZeroMQ_INCLUDE_DIR
        NAMES zmq.hpp
        PATHS ${PC_ZeroMQ_INCLUDE_DIRS}
        )

add_executable(caesar_pose3 main.cpp)
## add the include directory to our compile directives
target_include_directories(caesar_pose3 PUBLIC ${ZeroMQ_INCLUDE_DIR})
## add the 0mq library to our link directive
target_link_libraries(caesar_pose3 PUBLIC ${ZeroMQ_LIBRARY})
