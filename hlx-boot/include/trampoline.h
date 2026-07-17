#ifndef HLX_TRAMPOLINE_H
#define HLX_TRAMPOLINE_H

#include <stddef.h>

/* mov r64,imm64 (REX.W + (0xB8+reg) + imm64) only needs to reach these two -
 * add more as new trampoline shapes need them. */
typedef enum { JIT_REG_RAX = 0, JIT_REG_RDX = 2 } JitReg;

/* VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE); NULL on failure. */
unsigned char *JitAlloc(size_t size);

/* Emits "mov reg, imm64" (10 bytes) at buf+pos. Returns pos advanced past it. */
int JitEmitMovImm64(unsigned char *buf, int pos, JitReg reg, unsigned long long imm64);

/* Emits "jmp rax" (2 bytes) at buf+pos. Returns pos advanced past it. */
int JitEmitJmpRax(unsigned char *buf, int pos);

/* Emits "ret" (1 byte) at buf+pos. Returns pos advanced past it. */
int JitEmitRet(unsigned char *buf, int pos);

/* Writes "mov rax, target; jmp rax" (12 bytes) at `at` - caller ensures room.
 * Shared shape: a hook redirect (patching.c) and a trampoline's own tail-jump
 * into a fixed handler (log.c) are the same 12 bytes. */
void WriteAbsoluteJumpStub(unsigned char *at, const void *target);

#endif /* HLX_TRAMPOLINE_H */
