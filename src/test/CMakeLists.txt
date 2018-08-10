
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

message("CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR}")

add_library(graffsdk SHARED GraffSDK.cpp)
# set_target_properties(graffsdk PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(graffsdk PROPERTIES PUBLIC_HEADER GraffSDK.h)
target_include_directories(graffsdk PRIVATE .)
include(GNUInstallDirs)
install(TARGETS graffsdk
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

add_executable(feitor-test main.cpp)
## add the include directory to our compile directives
target_include_directories(feitor-test PUBLIC ${ZeroMQ_INCLUDE_DIR})
## at the 0mq library to our link directive
target_link_libraries(feitor-test PUBLIC ${ZeroMQ_LIBRARY})
# target_link_libraries(feitor-test PUBLIC graffsdk)

# the api testing program
add_executable(api-test apitest.cpp)
## add the include directory to our compile directives
target_include_directories(api-test PUBLIC ${ZeroMQ_INCLUDE_DIR})
## at the 0mq library to our link directive
target_link_libraries(api-test PUBLIC ${ZeroMQ_LIBRARY})
target_link_libraries(api-test PUBLIC graffsdk)
