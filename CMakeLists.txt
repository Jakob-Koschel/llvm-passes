cmake_minimum_required(VERSION 3.4.3)
project(llvm-passes)

set(CMAKE_CXX_STANDARD 11)

if(NOT DEFINED ENV{LLVM_HOME})
  message(FATAL_ERROR "$LLVM_HOME is not defined")
else ()
  set(ENV{LLVM_DIR} $ENV{LLVM_HOME}/lib/cmake/llvm)
endif()

find_package(LLVM REQUIRED CONFIG)
add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

if (${LLVM_VERSION_MAJOR} VERSION_GREATER_EQUAL 10)
    set(CMAKE_CXX_STANDARD 14)
endif ()
message(STATUS "Using LLVM version ${LLVM_PACKAGE_VERSION}")

add_subdirectory(HelloPass)
add_subdirectory(HelloLTOPass)
add_subdirectory(test)
