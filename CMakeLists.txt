cmake_minimum_required(VERSION 3.13)
project(FlowRa)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 17)

set(VERSION_MAJOR 0)
set(VERSION_MINOR 9)
configure_file(Version.in Version.h)

include_directories(${CMAKE_BINARY_DIR})
include_directories(..)

add_executable(FlowRa main.cpp FlowRa.h)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)

find_package(Boost 1.62.0 COMPONENTS system filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(FlowRa ${Boost_LIBRARIES})

find_package(Threads)
target_link_libraries(FlowRa ${CMAKE_THREAD_LIBS_INIT})
target_compile_definitions(FlowRa PRIVATE LOG_BOOST_DLL)

if (UNIX)
    target_link_libraries(FlowRa dl)
    install(TARGETS FlowRa
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION lib
            )
endif ()

if (${CMAKE_JS_INC})
    include_directories(${CMAKE_JS_INC})
    add_library(FlowRaNode SHARED main_node.cpp ${PROTO_SRCS} ${PROTO_HDRS} ${CMAKE_JS_SRC})
    set_target_properties(FlowRaNode PROPERTIES PREFIX "" SUFFIX ".node")
    target_link_libraries(FlowRaNode ${CMAKE_JS_LIB})
    target_link_libraries(FlowRaNode ${GRPC_LIBRARIES})
    target_link_libraries(FlowRaNode ${Boost_LIBRARIES})
else ()
#    include_directories(/home/accessory/.cmake-js/node-x64/v14.1.0/include/node/)
#    if (APPLE)
#        add_link_options(-undefined dynamic_lookup)
#    endif ()
endif ()


