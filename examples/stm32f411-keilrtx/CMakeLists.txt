
cmake_minimum_required(VERSION 3.18)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_language(C ASM)
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

# link_libraries(libopencm3_stm32f4)

#########################################################################
# Keil-RTX
FetchContent_Declare(cmsis_5
    SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/external/cmsis_5"
    GIT_REPOSITORY "https://github.com/ARM-software/CMSIS_5"
    GIT_TAG 5.8.0
    GIT_SHALLOW ON
    UPDATE_COMMAND ""
)
FetchContent_MakeAvailable(cmsis_5)
FetchContent_GetProperties(cmsis_5)

add_library(keil_rtx STATIC
    ${cmsis_5_SOURCE_DIR}/CMSIS/RTOS2/Source/os_systick.c
    ${cmsis_5_SOURCE_DIR}/CMSIS/RTOS2/RTX/Config/RTX_Config.c
    ${cmsis_5_SOURCE_DIR}/CMSIS/RTOS2/RTX/Source/rtx_delay.c
    ${cmsis_5_SOURCE_DIR}/CMSIS/RTOS2/RTX/Source/rtx_evflags.c
    ${cmsis_5_SOURCE_DIR}/CMSIS/RTOS2/RTX/Source/rtx_evr.c
    ${cmsis_5_SOURCE_DIR}/CMSIS/RTOS2/RTX/Source/rtx_kernel.c
    ${cmsis_5_SOURCE_DIR}/CMSIS/RTOS2/RTX/Source/rtx_lib.c
    ${cmsis_5_SOURCE_DIR}/CMSIS/RTOS2/RTX/Source/rtx_memory.c
    ${cmsis_5_SOURCE_DIR}/CMSIS/RTOS2/RTX/Source/rtx_mempool.c
    ${cmsis_5_SOURCE_DIR}/CMSIS/RTOS2/RTX/Source/rtx_msgqueue.c
    ${cmsis_5_SOURCE_DIR}/CMSIS/RTOS2/RTX/Source/rtx_mutex.c
    ${cmsis_5_SOURCE_DIR}/CMSIS/RTOS2/RTX/Source/rtx_semaphore.c
    ${cmsis_5_SOURCE_DIR}/CMSIS/RTOS2/RTX/Source/rtx_system.c
    ${cmsis_5_SOURCE_DIR}/CMSIS/RTOS2/RTX/Source/rtx_thread.c
    ${cmsis_5_SOURCE_DIR}/CMSIS/RTOS2/RTX/Source/rtx_timer.c
    ${cmsis_5_SOURCE_DIR}/CMSIS/RTOS2/RTX/Source/GCC/irq_armv7m.S)
target_include_directories(keil_rtx PUBLIC
    )
target_include_directories(keil_rtx PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/src/include
    ${cmsis_5_SOURCE_DIR}/CMSIS/Core/Include
    ${cmsis_5_SOURCE_DIR}/CMSIS/RTOS2/Include
    ${cmsis_5_SOURCE_DIR}/CMSIS/RTOS2/RTX/Include
    ${cmsis_5_SOURCE_DIR}/CMSIS/RTOS2/RTX/Config
    ${cmsis_5_SOURCE_DIR}/Device/ARM/ARMCM4/Include
)
target_compile_definitions(keil_rtx PUBLIC
    -DARMCM4_FP
    -DCMSIS_device_header=\"ARMCM4_FP.h\"
    D__ARM_ARCH_7EM__=1)
target_link_libraries(keil_rtx PRIVATE libopencm3_stm32f4)

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

# #########################################################################
# # evq_example_stm32f411
add_executable(evq_example_stm32f411
    ${CMAKE_CURRENT_LIST_DIR}/src/main.c
    ${CMAKE_CURRENT_LIST_DIR}/src/board.c
    ${CMAKE_CURRENT_LIST_DIR}/src/usb.c
    ${CMAKE_CURRENT_LIST_DIR}/src/irq_handlers.c
    # ${CMAKE_CURRENT_LIST_DIR}/../../port/freertos/evq_port_egroup.c
    # ${CMAKE_CURRENT_LIST_DIR}/../../port/freertos/evq_port_memory.c
    # ${CMAKE_CURRENT_LIST_DIR}/../../port/freertos/evq_port_mutex.c
        )
target_link_libraries(evq_example_stm32f411 PRIVATE 
    libopencm3_stm32f4
    keil_rtx)
#     target_include_directories(evq_example_stm32f411 PRIVATE
#         ${CMAKE_CURRENT_LIST_DIR}/src)
target_link_options(evq_example_stm32f411 PRIVATE 
    -T${CMAKE_CURRENT_LIST_DIR}/stm32f411ce.ld
    -Xlinker -Map=$<TARGET_FILE_DIR:evq_example_stm32f411>/$<TARGET_FILE_BASE_NAME:evq_example_stm32f411>.map)

# # Generate binary files
# add_custom_command(TARGET evq_example_stm32f411 POST_BUILD
#     COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:evq_example_stm32f411> $<TARGET_FILE_DIR:evq_example_stm32f411>/$<TARGET_FILE_BASE_NAME:evq_example_stm32f411>.bin)

# # Generate disassembly of executable
# add_custom_command(TARGET evq_example_stm32f411 POST_BUILD
#     COMMAND ${CMAKE_OBJDUMP} -D $<TARGET_FILE:evq_example_stm32f411> > $<TARGET_FILE_DIR:evq_example_stm32f411>/$<TARGET_FILE_BASE_NAME:evq_example_stm32f411>.asm)
