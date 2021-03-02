cmake_minimum_required(VERSION 2.6.0)

set(HELLO_PASS_PATH $<TARGET_FILE:LLVMHelloPass>)
set(HELLO_LTO_PASS_PATH $<TARGET_FILE:LLVMHelloLTOPass>)

set(LT_TEST_SHLIBEXT "${CMAKE_SHARED_LIBRARY_SUFFIX}")

set(LT_TEST_SITE_CFG_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.py.in")
set(LT_TEST_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

configure_file("${LT_TEST_SITE_CFG_INPUT}"
  "${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg.py.gen" @ONLY
)
file(
  GENERATE
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg.py"
  INPUT "${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg.py.gen"
)

find_package(Python REQUIRED)
add_custom_target(check
  COMMAND lit -v "${CMAKE_CURRENT_BINARY_DIR}"
)
