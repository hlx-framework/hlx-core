cmake_minimum_required(VERSION 3.12)
project(hlx-boot C)

if(NOT WIN32)
    message(FATAL_ERROR "hlx-boot is Windows-only - no proven POSIX equivalent exists for its IAT hooking/VirtualAlloc/SEH mechanisms.")
endif()
if(NOT MSVC)
    message(FATAL_ERROR "hlx-boot requires MSVC - it uses __try/__except, proven only when built from an \"x64 Native Tools Command Prompt for VS\".")
endif()

set(CMAKE_C_STANDARD 11)

include_directories(include)

add_library(hlx-boot SHARED
    src/boot.c
    src/config.c
    src/hlx_common.c
    src/log.c
    src/module.c
    src/patching.c
    src/reflection.c
    src/trampoline.c
)

set_target_properties(hlx-boot PROPERTIES
    OUTPUT_NAME "libhl64"
    PREFIX ""
)

target_link_libraries(hlx-boot PRIVATE user32)
