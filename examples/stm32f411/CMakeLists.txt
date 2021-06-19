
cmake_minimum_required(VERSION 3.18)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(evqueue-stm32f411)
include(FetchContent)

FetchContent_Declare(libopencm3
    GIT_REPOSITORY "https://github.com/libopencm3/libopencm3.git"
    GIT_TAG cb0661f81de5b1cae52ca99c7b5985b678176db7
    )
FetchContent_MakeAvailable(libopencm3)
FetchContent_GetProperties(libopencm3)

add_custom_target(libopencm3
    COMMAND make -j TARGETS=stm32/f4
    WORKING_DIRECTORY ${libopencm3_SOURCE_DIR})

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

# Setup executable
add_executable(evqueue_stm32f411
    ${CMAKE_CURRENT_LIST_DIR}/src/main.c)
target_link_libraries(evqueue_stm32f411 PRIVATE libopencm3_stm32f4)
target_link_options(evqueue_stm32f411 PRIVATE -T${CMAKE_CURRENT_LIST_DIR}/src/blackpill-bootloader.ld)


# Generate binary files
add_custom_command(TARGET evqueue_stm32f411 POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:evqueue_stm32f411> $<TARGET_FILE_DIR:evqueue_stm32f411>/$<TARGET_FILE_BASE_NAME:evqueue_stm32f411>.bin)