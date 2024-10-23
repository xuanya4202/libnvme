project(mynvme)
cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

file(GLOB_RECURSE SOURCES "lib/nvme/*"
                          "lib/common/*")

include_directories(
  "${PROJECT_SOURCE_DIR}/include"
  "${PROJECT_SOURCE_DIR}/lib/common"
  "${PROJECT_SOURCE_DIR}/lib/nvme"
  "${PROJECT_SOURCE_DIR}/lib"
)

set(INCLUDE_PATHS "include"
                  "lib")

add_library(mynvme STATIC ${SOURCES})
target_compile_options(mynvme PRIVATE -g -O0)

add_executable(nvme_info ${PROJECT_SOURCE_DIR}/tools/info/nvme_info.c)
add_dependencies(nvme_info mynvme)
target_link_libraries(nvme_info 
    pciaccess 
    mynvme
    numa)

add_executable(nvme_perf ${PROJECT_SOURCE_DIR}/tools/perf/nvme_perf.c)
add_dependencies(nvme_perf mynvme)
target_link_libraries(nvme_perf 
    pciaccess 
    mynvme
    numa
    pthread)

add_executable(nvme_example ${PROJECT_SOURCE_DIR}/tools/example/example.cpp)
target_compile_options(nvme_example PRIVATE -std=c++11 -g -O0)
add_dependencies(nvme_example mynvme)
target_link_libraries(nvme_example 
    pciaccess 
    mynvme
    numa)