
# evq port for linux
# TODO: do we need windows? who tf uses windows anyways
add_library(evq_port_linux OBJECT
    ${PROJECT_SOURCE_DIR}/port/linux/evq_port_egroup.cpp
    ${PROJECT_SOURCE_DIR}/port/linux/evq_port_memory.cpp
    ${PROJECT_SOURCE_DIR}/port/linux/evq_port_mutex.cpp
    ${PROJECT_SOURCE_DIR}/port/linux/evq_port_time.cpp
    ${PROJECT_SOURCE_DIR}/port/linux/evq_port_log.cpp)
target_link_libraries(evq_port_linux PUBLIC evq)

# Sample application for direct messaging
add_executable(evq_sample_direct 
    ${CMAKE_CURRENT_LIST_DIR}/evq_sample_direct.cpp)
target_link_libraries(evq_sample_direct PRIVATE evq_port_linux)

# Sample application for event system
add_executable(evq_sample_event
    ${CMAKE_CURRENT_LIST_DIR}/evq_sample_event.cpp)
target_link_libraries(evq_sample_event PRIVATE evq_port_linux)

