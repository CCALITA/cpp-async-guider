cmake_minimum_required(VERSION 3.25)
project(stdexec-examples)

# Get stdexec
include(CPM.cmake)
CPMAddPackage(
  NAME stdexec
  GITHUB_REPOSITORY NVIDIA/stdexec
  GIT_TAG main
)

# Example: hello_world
add_executable(hello_world hello_world.cpp)
target_link_libraries(hello_world PRIVATE STDEXEC::stdexec)

# Example: basic_pipeline
add_executable(basic_pipeline basic_pipeline.cpp)
target_link_libraries(basic_pipeline PRIVATE STDEXEC::stdexec)

# Example: concurrent_when_all
add_executable(concurrent_when_all concurrent_when_all.cpp)
target_link_libraries(concurrent_when_all PRIVATE STDEXEC::stdexec)
