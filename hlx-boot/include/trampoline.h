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
 * into a fixed handler (log.c) are the same 12 bytes. Clobbers RAX - fine when
 * `at` is reached as the START of a fresh call (nothing live yet to clobber),
 * but NOT fine as a resume-jump appended after relocated bytes that may have
 * just loaded a value INTO rax for the resumed code to read - see
 * WriteRegisterSafeJumpStub below, which is what patching.c's BuildTrampoline
 * uses for exactly that case. */
void WriteAbsoluteJumpStub(unsigned char *at, const void *target);

/* Writes "jmp qword ptr [rip+0]" followed immediately by the 8-byte absolute
 * target (6 + 8 = 14 bytes total) at `at` - caller ensures room. Unlike
 * WriteAbsoluteJumpStub, this touches NO general-purpose register: the jump
 * target is read straight out of the instruction stream via a RIP-relative
 * memory operand, never staged into rax first. Required for BuildTrampoline's
 * resume-jump (patching.c) because the bytes copied ahead of it were vetted by
 * DecodeSafeInstruction only for "safe to relocate" (no RIP-relative/call/jmp
 * operands of their OWN), not for "doesn't leave a value live in some register
 * that the ORIGINAL code immediately after the cut point depends on" - e.g. a
 * whitelisted `mov r64,imm` loading a global's address into rax right before
 * the original body's next instruction dereferences that same rax. Plugging a
 * rax-clobbering jump stub in between silently corrupts that value instead of
 * raising anything at patch-install time; the trampoline install "succeeds"
 * and the corruption only surfaces the first time the patched function runs
 * (see the access-violation-inside-the-patched-function bug this replaced). */
void WriteRegisterSafeJumpStub(unsigned char *at, const void *target);

#endif /* HLX_TRAMPOLINE_H */
