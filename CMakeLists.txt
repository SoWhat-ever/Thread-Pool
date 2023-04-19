cmake_minimum_required(VERSION 3.0)
project(ThreadPool)



include_directories(.)
include_directories(/usr/local/include)
link_directories(/usr/local/lib)


    
add_executable(test test/main.cc)
target_link_libraries(test pthread)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
