project(tyndal)
cmake_minimum_required(VERSION 2.8.3)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -Wall")

file(GLOB ROS_INCLUDE_DIRS "${CMAKE_SYSROOT}/opt/ros/*/include")
include_directories(${CMAKE_SOURCE_DIR} ${ROS_INCLUDE_DIRS})
file(GLOB ROS_LD_DIRS "${CMAKE_SYSROOT}/opt/ros/*/lib")
link_directories(${ROS_LD_DIRS})

set(LD_FLAGS "-lpthread -lrosconsole -lroscpp -lroscpp_serialization -lzmq -lprotobuf") #  -lfmt

file(GLOB CFILES "tyndal/*/*.c*")
if (${CFILES})
add_library(tyndal SHARED ${CFILES})
target_link_libraries(tyndal ${LD_FLAGS})
install(TARGETS tyndal DESTINATION lib)
set(TYNDAL_TARGET "tyndal")
else()
set(TYNDAL_TARGET "${LD_FLAGS}")
endif()

file(GLOB HEADERS "tyndal/*.h*" "tyndal/*/*.h*")
foreach(HEADER ${HEADERS})
execute_process(COMMAND bash "-c" "dirname -z $(realpath --relative-to=${CMAKE_SOURCE_DIR} ${HEADER})" OUTPUT_VARIABLE HEADER_DIR)
install(FILES ${HEADER} DESTINATION include/${HEADER_DIR})
endforeach(HEADER)

file(GLOB EXAMPLES "examples/*.c*")
foreach(EXAMPLE ${EXAMPLES})
execute_process(COMMAND bash "-c" "basename -z $(echo ${EXAMPLE} | cut -f 1 -d '.')" OUTPUT_VARIABLE EXAMPLE_TARGET)
add_executable(${EXAMPLE_TARGET} ${EXAMPLE})
target_link_libraries(${EXAMPLE_TARGET} ${TYNDAL_TARGET})
install(TARGETS ${EXAMPLE_TARGET} DESTINATION bin)
endforeach(EXAMPLE)
