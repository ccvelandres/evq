include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.13.0.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(test_evq
    ${CMAKE_CURRENT_LIST_DIR}/test_evq.cpp
    ${CMAKE_CURRENT_LIST_DIR}/test_evq_message.cpp
    ${CMAKE_CURRENT_LIST_DIR}/test_evq_stream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mocks/mock_evq_port.cpp)

target_link_libraries(test_evq
    evq
    GTest::gtest_main
    GTest::gmock)

add_executable(test_evq_log
    ${CMAKE_CURRENT_LIST_DIR}/test_evq_log.cpp)

target_link_libraries(test_evq_log
    evq
    GTest::gtest_main
    GTest::gmock)

include(GoogleTest)
gtest_discover_tests(test_evq)
gtest_discover_tests(test_evq_log)
# add_test(NAME test_evq
#   COMMAND test_evq)
