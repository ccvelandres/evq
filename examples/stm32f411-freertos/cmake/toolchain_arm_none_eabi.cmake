
# Append current directory to CMAKE_MODULE_PATH for making device specific cmake modules visible
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

# Target definition
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)
set(CMAKE_CROSSCOMPILING 1)

# Set system dependent extensions
if(WIN32)
  set(TOOLCHAIN_EXT ".exe" )
else()
  set(TOOLCHAIN_EXT "" )
endif()

# Toolchain name
set(TOOLCHAIN arm-none-eabi)

# Check environment variable for toolchain directory
if($ENV{ARMGCC_DIR})
    set(TOOLCHAIN_PREFIX $ENV{ARMGCC_DIR})
endif()

# Locate toolchain binaries
find_program(CMAKE_C_COMPILER NAMES ${TOOLCHAIN}-gcc${TOOLCHAIN_EXT} PATHS ${TOOLCHAIN_PREFIX}/ REQUIRED)
find_program(CMAKE_CXX_COMPILER ${TOOLCHAIN}-g++${TOOLCHAIN_EXT} PATHS ${TOOLCHAIN_PREFIX}/ REQUIRED)
find_program(CMAKE_ASM_COMPILER ${TOOLCHAIN}-gcc${TOOLCHAIN_EXT} PATHS ${TOOLCHAIN_PREFIX}/ REQUIRED)
find_program(CMAKE_OBJCOPY ${TOOLCHAIN}-objcopy${TOOLCHAIN_EXT} PATHS ${TOOLCHAIN_PREFIX}/ REQUIRED)
find_program(CMAKE_OBJDUMP ${TOOLCHAIN}-objdump${TOOLCHAIN_EXT} PATHS ${TOOLCHAIN_PREFIX}/ REQUIRED)
find_program(CMAKE_SIZE ${TOOLCHAIN}-size${TOOLCHAIN_EXT} PATHS ${TOOLCHAIN_PREFIX}/ REQUIRED)

# Derive toolchain path from binaries
get_filename_component(TOOLCHAIN_PATH "${CMAKE_C_COMPILER}" DIRECTORY)
get_filename_component(TOOLCHAIN_PATH "${TOOLCHAIN_PATH}/.." ABSOLUTE)

# Set toolchain paths
set(TOOLCHAIN_BIN_DIR "${TOOLCHAIN_PATH}/bin")
set(TOOLCHAIN_INC_DIR "${TOOLCHAIN_PATH}/${TOOLCHAIN}/include")
set(TOOLCHAIN_LIB_DIR "${TOOLCHAIN_PATH}/${TOOLCHAIN}/lib")

# Perform compiler test with static library
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Set compiler linker flags
set(OBJECT_CXX_GEN_GLAGS "${OBJECT_GEN_FLAGS} -fdata-sections -ffunction-sections -fno-builtin -fno-strict-aliasing -fshort-enums -Wall")

set(CMAKE_C_FLAGS   "${OBJECT_CXX_GEN_GLAGS} -std=c99 " CACHE INTERNAL "C Compiler options")
set(CMAKE_CXX_FLAGS "${OBJECT_CXX_GEN_GLAGS} -fno-exceptions -fno-rtti -Wno-register -std=c++17 " CACHE INTERNAL "C++ Compiler options")
set(CMAKE_ASM_FLAGS "${OBJECT_GEN_FLAGS} -g3" CACHE INTERNAL "ASM Compiler options")

set(CMAKE_EXE_LINKER_FLAGS "${OBJECT_GEN_FLAGS} -Wl,--gc-sections --specs=nano.specs --specs=nosys.specs -lc -lnosys -lm" CACHE INTERNAL "Linker options")

# Debug/Release configuration
set(CMAKE_C_FLAGS_DEBUG "-g -Og -g3 -DDEBUG" CACHE INTERNAL "C Compiler options for debug build type")
set(CMAKE_CXX_FLAGS_DEBUG "-g -Og -g3 -DDEBUG" CACHE INTERNAL "C++ Compiler options for debug build type")
set(CMAKE_ASM_FLAGS_DEBUG "" CACHE INTERNAL "ASM Compiler options for debug build type")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "" CACHE INTERNAL "Linker options for debug build type")

set(CMAKE_C_FLAGS_RELEASE "-O2 -flto" CACHE INTERNAL "C Compiler options for release build type")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -flto" CACHE INTERNAL "C++ Compiler options for release build type")
set(CMAKE_ASM_FLAGS_RELEASE "" CACHE INTERNAL "ASM Compiler options for release build type")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-flto" CACHE INTERNAL "Linker options for release build type")

set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g3 -flto" CACHE INTERNAL "C Compiler options for release + dbg info build type")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g3 -flto" CACHE INTERNAL "C++ Compiler options for release + dbg info build type")
set(CMAKE_ASM_FLAGS_RELWITHDEBINFO "" CACHE INTERNAL "ASM Compiler options for release + dbg info build type")
set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "-flto" CACHE INTERNAL "Linker options for release + dbg info build type")

set(CMAKE_C_FLAGS_MINSIZEREL "-Os -flto" CACHE INTERNAL "C Compiler options for minsize release build type")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -flto" CACHE INTERNAL "C++ Compiler options for minsize release build type")
set(CMAKE_ASM_FLAGS_MINSIZEREL "" CACHE INTERNAL "ASM Compiler options for minsize release build type")
set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "-flto" CACHE INTERNAL "Linker options for minsize release build type")

set(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_PREFIX}/${${TOOLCHAIN}} ${CMAKE_PREFIX_PATH})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)