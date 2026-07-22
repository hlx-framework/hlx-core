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
include_directories(vendor/hashlink)

add_library(hlx-boot SHARED
    src/boot.c
    src/config.c
    src/hlcode_shim.c
    src/hlx_common.c
    src/log.c
    src/module.c
    src/patching.c
    src/reflection.c
    src/trampoline.c
    vendor/hashlink/code.c
)

set_target_properties(hlx-boot PROPERTIES
    OUTPUT_NAME "libhl64"
    PREFIX ""
)

# code.c/hlcode_shim.c include the vendored hashlink/hl.h - LIBHL_STATIC makes its HL_API
# macro expand to plain `extern` rather than `__declspec(dllimport)`, so hlcode_shim.c's own
# definitions of hl_malloc/hl_zalloc/hl_alloc_init/hl_free/etc. don't conflict with importing
# them from a (nonexistent, for these symbols) libhl.dll import lib. See
# vendor/hashlink/README.md.
target_compile_definitions(hlx-boot PRIVATE LIBHL_STATIC)

target_link_libraries(hlx-boot PRIVATE user32)
