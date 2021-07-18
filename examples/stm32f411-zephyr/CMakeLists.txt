
cmake_minimum_required(VERSION 3.18)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(FetchContent)
include(ExternalProject)
find_package(Git REQUIRED)

# set(ZEPHYR_TOOLCHAIN_VARIANT gnuarmemb)
# set(GNUARMEMB_TOOLCHAIN_PATH /usr)

set(BOARD blackpill_f411ce)
find_package(Zephyr REQUIRED HINTS "$ENV{ZEPHYR_BASE}")

project(evqueue-stm32f411)

# #########################################################################
# # evq_example_stm32f411
# set(EVQ_BUILD_TESTS OFF)
# set(EVQ_BUILD_DOCS OFF)
# set(EVQ_BUILD_EXAMPLES OFF)
# set(EVQ_LOG_LEVEL 5)
# set(EVQ_LOG_LEVEL_MAX 5)
# set(EVQ_CORE_TIMEOUT EVQ_TIMEOUT_MAX)
# add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../ ${CMAKE_CURRENT_BINARY_DIR}/_deps/evq)
# target_link_libraries(evq PRIVATE libopencm3_stm32f4)

target_sources(app PRIVATE src/main.c)