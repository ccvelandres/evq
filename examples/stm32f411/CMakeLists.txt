
cmake_minimum_required(VERSION 3.18)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(evqueue-stm32f411)
include(FetchContent)

#########################################################################
# libopencm3
FetchContent_Declare(libopencm3
    SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/external/libopencm3"
    GIT_REPOSITORY "https://github.com/libopencm3/libopencm3.git"
    GIT_TAG cb0661f81de5b1cae52ca99c7b5985b678176db7
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    )
FetchContent_MakeAvailable(libopencm3)
FetchContent_GetProperties(libopencm3)

add_custom_command(OUTPUT "${libopencm3_SOURCE_DIR}/lib/libopencm3_stm32f4.a"
    COMMAND make -j TARGETS=stm32/f4
    WORKING_DIRECTORY ${libopencm3_SOURCE_DIR})
add_custom_target(libopencm3
    DEPENDS "${libopencm3_SOURCE_DIR}/lib/libopencm3_stm32f4.a")

# Setup library and compile flags for stm32f4
add_library(libopencm3_stm32f4 STATIC IMPORTED)
set_target_properties(libopencm3_stm32f4 PROPERTIES 
    IMPORTED_LOCATION "${libopencm3_SOURCE_DIR}/lib/libopencm3_stm32f4.a")
target_include_directories(libopencm3_stm32f4 INTERFACE "${libopencm3_SOURCE_DIR}/include")
target_link_directories(libopencm3_stm32f4 INTERFACE "${libopencm3_SOURCE_DIR}/lib")
target_compile_options(libopencm3_stm32f4 INTERFACE
    -mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -MD -DSTM32F4 -nostartfiles)
target_link_options(libopencm3_stm32f4 INTERFACE
    -mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -MD -DSTM32F4 -nostartfiles)
add_dependencies(libopencm3_stm32f4 libopencm3)

#########################################################################
# FreeRTOS-Kernel
FetchContent_Declare(freertos_kernel
    SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/external/freertos_kernel"
    GIT_REPOSITORY "https://github.com/FreeRTOS/FreeRTOS-Kernel"
    GIT_TAG V10.4.4
    GIT_SHALLOW ON
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND "")
FetchContent_MakeAvailable(freertos_kernel)
FetchContent_GetProperties(freertos_kernel)

# FreeRTOS doesnt have cmake scripts yet
add_library(freertos_kernel_cm4 STATIC
    ${freertos_kernel_SOURCE_DIR}/croutine.c
    ${freertos_kernel_SOURCE_DIR}/tasks.c
    ${freertos_kernel_SOURCE_DIR}/list.c
    ${freertos_kernel_SOURCE_DIR}/queue.c
    ${freertos_kernel_SOURCE_DIR}/event_groups.c
    ${freertos_kernel_SOURCE_DIR}/stream_buffer.c
    ${freertos_kernel_SOURCE_DIR}/timers.c
    ${freertos_kernel_SOURCE_DIR}/portable/GCC/ARM_CM4F/port.c
    ${freertos_kernel_SOURCE_DIR}/portable/MemMang/heap_4.c)
target_include_directories(freertos_kernel_cm4 PUBLIC
    ${freertos_kernel_SOURCE_DIR}/include
    ${freertos_kernel_SOURCE_DIR}/portable/GCC/ARM_CM4F)
target_include_directories(freertos_kernel_cm4 PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src)
target_link_libraries(freertos_kernel_cm4 PRIVATE libopencm3_stm32f4)

#########################################################################
# evqueue_stm32f411
add_executable(evqueue_stm32f411
    ${CMAKE_CURRENT_LIST_DIR}/src/main.c
    ${CMAKE_CURRENT_LIST_DIR}/src/usb.c
    ${CMAKE_CURRENT_LIST_DIR}/src/log.c
    ${CMAKE_CURRENT_LIST_DIR}/src/freertos_hooks.c
    ${CMAKE_CURRENT_LIST_DIR}/src/retarget.c
    ${CMAKE_CURRENT_LIST_DIR}/src/board.c)
target_link_libraries(evqueue_stm32f411 PRIVATE 
    libopencm3_stm32f4
    freertos_kernel_cm4)
    target_include_directories(evqueue_stm32f411 PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src)
target_link_options(evqueue_stm32f411 PRIVATE 
    -T${CMAKE_CURRENT_LIST_DIR}/stm32f411ce.ld
    -Xlinker -Map=$<TARGET_FILE_DIR:evqueue_stm32f411>/$<TARGET_FILE_BASE_NAME:evqueue_stm32f411>.map)

# Generate binary files
add_custom_command(TARGET evqueue_stm32f411 POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:evqueue_stm32f411> $<TARGET_FILE_DIR:evqueue_stm32f411>/$<TARGET_FILE_BASE_NAME:evqueue_stm32f411>.bin)

# Generate disassembly of executable
add_custom_command(TARGET evqueue_stm32f411 POST_BUILD
    COMMAND ${CMAKE_OBJDUMP} -D $<TARGET_FILE:evqueue_stm32f411> > $<TARGET_FILE_DIR:evqueue_stm32f411>/$<TARGET_FILE_BASE_NAME:evqueue_stm32f411>.asm)
