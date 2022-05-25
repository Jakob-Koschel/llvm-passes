cmake_minimum_required(VERSION 3.4.3)

set(CMAKE_CXX_STANDARD 11)

if(NOT DEFINED ENV{LLVMPREFIX})
  message(FATAL_ERROR "$LLVMPREFIX is not defined")
else ()
  set(ENV{LLVM_DIR} $ENV{LLVMPREFIX}/lib/cmake/llvm)
endif()

SET (CMAKE_C_COMPILER             $ENV{LLVMPREFIX}/bin/clang)
SET (CMAKE_CXX_COMPILER           $ENV{LLVMPREFIX}/bin/clang++)

project(llvm-passes)

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
