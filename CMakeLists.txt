cmake_minimum_required(VERSION 3.18)
enable_testing()
project(evqueue)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-Warray-bounds)
endif()

set(EVQ_CONFIG_LIST "")
macro(evq_set_default opt value)
    if(NOT ${opt})
        set(${opt} ${value})
    endif()
    list(APPEND EVQ_CONFIG_LIST "${opt}")
endmacro()

# Set default values for config
evq_set_default(EVQ_RTOS_SUPPORT ON)
evq_set_default(EVQ_BUILD_EXAMPLES ÖN)
evq_set_default(EVQ_BUILD_DOCS ÖN)
evq_set_default(EVQ_BUILD_TESTS ÖN)
evq_set_default(EVQ_LOGGING ÖN)
evq_set_default(EVQ_ASSERT_ENABLE ÖN)
evq_set_default(EVQ_LOG_LEVEL 4)
evq_set_default(EVQ_LOG_LEVEL_MAX 4)
evq_set_default(EVQ_LOG_SOURCE ON)
evq_set_default(EVQ_LOG_LEVEL_SYM ON)
evq_set_default(EVQ_MAX_HANDLES 16)
evq_set_default(EVQ_MAX_EVENT_SUBSCRIBE_COUNT 16)
evq_set_default(EVQ_CORE_QUEUE_SIZE 8)
evq_set_default(EVQ_CORE_TIMEOUT 5)
configure_file(evq_config.h.in evq/evq_config.h @ONLY)

add_library(evq
    ${CMAKE_CURRENT_LIST_DIR}/src/evq_core.c
    ${CMAKE_CURRENT_LIST_DIR}/src/evq_log.c
    ${CMAKE_CURRENT_LIST_DIR}/src/evq_stream.c
    ${CMAKE_CURRENT_LIST_DIR}/src/evq_event.c
    ${CMAKE_CURRENT_LIST_DIR}/src/evq_message.c)

target_include_directories(evq PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/src/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_include_directories(evq PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/src/include/evq>)

# Print options
message("EVQ Options")
foreach(opt ${EVQ_CONFIG_LIST})
    message("${opt}: ${${opt}}")
endforeach()

if(EVQ_BUILD_TESTS)
    target_link_libraries(evq PRIVATE gcov)
    target_compile_options(evq PRIVATE --coverage -fprofile-arcs -ftest-coverage)
    add_subdirectory(tests)

    # Add target to generate reports
    find_program(LCOV_PATH NAMES lcov)
    find_program(GCOV_PATH NAMES gcov)
    find_program(GENHTML_PATH NAMES genhtml)
    add_custom_target(evq_coverage
        COMMAND ${LCOV_PATH} --gcov-tool ${GCOV_PATH} -c -i -d . -b ${PROJECT_SOURCE_DIR} -o evq_coverage.base
        COMMAND ${LCOV_PATH} --gcov-tool ${GCOV_PATH} --directory . -b ${PROJECT_SOURCE_DIR} --capture --output-file evq_coverage.capture
        COMMAND ${LCOV_PATH} --gcov-tool ${GCOV_PATH} -a evq_coverage.base -a evq_coverage.capture --output-file evq_coverage.total
        COMMAND ${LCOV_PATH} --gcov-tool ${GCOV_PATH} --extract evq_coverage.total ${PROJECT_SOURCE_DIR}/src/* --output-file evq_coverage.info
        COMMAND ${GENHTML_PATH} -o evq_coverage evq_coverage.info
        BYPRODUCTS 
            evq_coverage.base
            evq_coverage.coverage
            evq_coverage.total
            evq_coverage.info
            evq_coverage/index.html
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()

find_package(Doxygen QUIET REQUIRED dot)
if(EVQ_BUILD_DOCS AND DOXYGEN_FOUND)
    set(DOXYGEN_EXTRACT_ALL ON)

    doxygen_add_docs(evq_docs
        ${PROJECT_SOURCE_DIR}/src/
        ${PROJECT_SOURCE_DIR}/include/)
endif()

if(EVQ_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()