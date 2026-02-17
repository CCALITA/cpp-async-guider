cmake_minimum_required(VERSION 3.25)
project(cpp-async-examples)

# Get stdexec
include(CPM.cmake)
CPMAddPackage(
  NAME stdexec
  GITHUB_REPOSITORY NVIDIA/stdexec
  GIT_TAG main
)

# Example 1: Parallel Data Processing
add_executable(parallel_processing scripts/parallel_processing.cpp)
target_link_libraries(parallel_processing PRIVATE STDEXEC::stdexec)

# Example 2: Reliable API with Retry
add_executable(reliable_api_call scripts/reliable_api_call.cpp)
target_link_libraries(reliable_api_call PRIVATE STDEXEC::stdexec)

# Example 3: Data Pipeline
add_executable(data_pipeline scripts/data_pipeline.cpp)
target_link_libraries(data_pipeline PRIVATE STDEXEC::stdexec)
