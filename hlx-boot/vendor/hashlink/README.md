# Vendored HashLink sources

`code.c`, `hlmodule.h`, `hl.h`, `hlsystem.h`, `opcodes.h` are copied **verbatim** (byte-for-byte,
verified via `diff`) from HashLink's own `src/` directory (MIT licensed), so that hlx-boot can call
`hl_code_read`/`hl_code_free` as ordinary in-process function calls.

**Why vendor instead of linking against the real DLL**: `hl_code_read` (`code.c:424`, declared in
`hlmodule.h:145`) is not exported by the real `libhl.dll` any shipped HashLink game ships (verified
via `objdump -p` against Farever's own `libhl.dll`: 431 `hl_*` exports, zero `hl_code_*`/
`hl_module_*`). This is not an oversight - HashLink's own `CMakeLists.txt` compiles `code.c`/
`module.c`/`main.c`/`jit.c` only into the `hl` executable target, never into the `libhl` DLL target,
so none of it carries an `HL_API` export marker. This logic was never meant to cross a DLL boundary.

**Do not hand-edit `code.c`/`hlmodule.h`/`hl.h`/`hlsystem.h`/`opcodes.h` here** - if HashLink's
upstream source changes, re-copy the new versions from a matching `hashlink/src/` checkout instead.
hlx-boot's own glue code (allocator shim, the constructor-table builder) lives in
`../../src/hlcode_shim.c` and `../../src/reflection.c`, not in this directory.

**Build note**: `LIBHL_STATIC` must be defined when compiling anything that includes `hl.h` from
this directory (see `hlx-boot/CMakeLists.txt`) - without it, `HL_API` expands to
`__declspec(dllimport)` (`hl.h`'s default, assuming you're linking against the real `libhl.dll`),
which conflicts with hlx-boot supplying its own local definitions of the allocator primitives
(`hl_malloc`/`hl_zalloc`/`hl_alloc_init`/`hl_free`) and the couple of other externs `code.c` needs
(`hl_hash_gen`/`hl_utf8_length`/`hl_from_utf8`/`hl_detect_debugger`/`hlt_void` - see
`hlcode_shim.c` for why each is needed and why a private shim was chosen over
`GetProcAddress`-resolving them from the real DLL).
