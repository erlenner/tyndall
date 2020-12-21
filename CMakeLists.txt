project(tyndall)
cmake_minimum_required(VERSION 2.8.3)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -Wall")

include_directories(${CMAKE_SOURCE_DIR})
set(LD_FLAGS "-lpthread -lzmq -lprotobuf -lfmt")

find_package(roscpp)

if ("${roscpp_FOUND}")
  if ("${roscpp_LINK_DIRECTORIES}" STREQUAL "")
    # quickfix using the path of the first library in roscpp_LIBRARIES
    execute_process(COMMAND bash "-c" "dirname -z $(echo \"${roscpp_LIBRARIES}\" | cut -f1 -d ';')" OUTPUT_VARIABLE roscpp_LINK_DIRECTORIES)
  endif()

  include_directories(${roscpp_INCLUDE_DIRS})
  link_directories(${roscpp_LINK_DIRECTORIES})
  set(LD_FLAGS "${LD_FLAGS} ${roscpp_LIBRARIES}")
else()
  add_definitions("-DNO_ROS")
endif()

file(GLOB CFILES "tyndall/*/*.c*")
if ("${CFILES}" STREQUAL "")
  set(TYNDAL_TARGET "${LD_FLAGS}")
else()
  add_library(tyndall SHARED ${CFILES})
  target_link_libraries(tyndall ${LD_FLAGS})
  install(TARGETS tyndall DESTINATION lib)
  set(TYNDAL_TARGET "tyndall")
endif()

file(GLOB HEADERS "tyndall/*.h*" "tyndall/*/*.h*")
foreach(HEADER ${HEADERS})
  execute_process(COMMAND bash "-c" "dirname -z $(realpath --relative-to=${CMAKE_SOURCE_DIR} ${HEADER})" OUTPUT_VARIABLE HEADER_DIR)
  install(FILES ${HEADER} DESTINATION include/${HEADER_DIR})
endforeach(HEADER)

set(EXAMPLE_PREFIX "${CMAKE_PROJECT_NAME}_ex_")

file(GLOB EXAMPLES "examples/*.c*")
foreach(EXAMPLE ${EXAMPLES})
  execute_process(COMMAND bash "-c" "printf ${EXAMPLE_PREFIX}$(basename ${EXAMPLE} | cut -f 1 -d '.')" OUTPUT_VARIABLE EXAMPLE_TARGET)
  add_executable(${EXAMPLE_TARGET} EXCLUDE_FROM_ALL ${EXAMPLE})
  target_link_libraries(${EXAMPLE_TARGET} ${TYNDAL_TARGET})
  install(TARGETS ${EXAMPLE_TARGET} DESTINATION bin OPTIONAL)
  set(EXAMPLE_TARGETS "${EXAMPLE_TARGETS}${EXAMPLE_TARGET};")
endforeach(EXAMPLE)


file(GLOB SUBEXAMPLES "examples/*/*.c*")
foreach(EXAMPLE ${SUBEXAMPLES})
  execute_process(COMMAND bash "-c" "printf ${EXAMPLE_PREFIX}$(dirname ${EXAMPLE} | xargs basename)_$(basename ${EXAMPLE} | cut -f 1 -d '.')" OUTPUT_VARIABLE EXAMPLE_TARGET)
  add_executable(${EXAMPLE_TARGET} EXCLUDE_FROM_ALL ${EXAMPLE})
  target_link_libraries(${EXAMPLE_TARGET} ${TYNDAL_TARGET})
  install(TARGETS ${EXAMPLE_TARGET} DESTINATION bin OPTIONAL)
  set(EXAMPLE_TARGETS "${EXAMPLE_TARGETS}${EXAMPLE_TARGET};")
endforeach(EXAMPLE)

add_custom_target(examples)
add_dependencies(examples ${EXAMPLE_TARGETS})
