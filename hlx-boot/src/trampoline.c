#include "trampoline.h"
#include <windows.h>
#include <stdint.h>

unsigned char *JitAlloc(size_t size)
{
    return (unsigned char *)VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
}

int JitEmitMovImm64(unsigned char *buf, int pos, JitReg reg, unsigned long long imm64)
{
    buf[pos++] = 0x48;
    buf[pos++] = (unsigned char)(0xB8 + (int)reg);
    for (int i = 0; i < 8; i++) buf[pos++] = (unsigned char)(imm64 >> (8 * i));
    return pos;
}

int JitEmitJmpRax(unsigned char *buf, int pos)
{
    buf[pos++] = 0xFF;
    buf[pos++] = 0xE0;
    return pos;
}

int JitEmitRet(unsigned char *buf, int pos)
{
    buf[pos++] = 0xC3;
    return pos;
}

void WriteAbsoluteJumpStub(unsigned char *at, const void *target)
{
    int pos = JitEmitMovImm64(at, 0, JIT_REG_RAX, (unsigned long long)(uintptr_t)target);
    JitEmitJmpRax(at, pos);
}
