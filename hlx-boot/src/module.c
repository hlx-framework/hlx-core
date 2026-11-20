#include "module.h"
#include "hl_compat.h"
#include "reflection.h"
#include "boot.h"
#include <windows.h>

static void *g_recoveredCode;
static void **g_functionsPtrsGlobal;
static void *g_globalsData;
static int *g_globalsIndexes;
static int *g_functionsIndexes;

static void *FindPrimaryModule(const void *targetFun, void ***outFunctionsPtrs, void **outGlobalsData, int **outGlobalsIndexes, int **outFunctionsIndexes)
{
    unsigned char *addr = NULL;
    MEMORY_BASIC_INFORMATION mbi;
    int regionsScanned = 0;
    unsigned long candidatesChecked = 0;
    int matchesFound = 0;
    void *foundCode = NULL;
    DWORD startTick = GetTickCount();

    hlx_log(HLX_LOG_DEBUG, "[hlx-boot] FindPrimaryModule: scanning for jit_code/codesize bounding %p...", targetFun);

    while (VirtualQuery(addr, &mbi, sizeof(mbi)) == sizeof(mbi)) {
        if (mbi.State == MEM_COMMIT && mbi.Type == MEM_PRIVATE &&
            (mbi.Protect == PAGE_READWRITE || mbi.Protect == PAGE_EXECUTE_READWRITE)) {
            regionsScanned++;
            unsigned char *base = (unsigned char *)mbi.BaseAddress;
            SIZE_T size = mbi.RegionSize;
            SIZE_T maxOffset = (size >= sizeof(hlx_module_mirror_t)) ? size - sizeof(hlx_module_mirror_t) : 0;

            for (SIZE_T off = 0; off <= maxOffset; off += sizeof(void *)) {
                /* volatile is load-bearing: these fields are read once each purely for their
                 * value (no other side effect), which is exactly the shape of access an
                 * optimizing compiler is free to hoist/reorder/eliminate relative to a __try
                 * block, since ISO C's abstract machine has no concept of a "faulting read" for
                 * it to preserve. Every repro so far crashed at a reproducible fixed address
                 * (0xA both times, different ASLR base) from INSIDE an already-__try-guarded
                 * read - consistent with the read actually executing outside the guarded region
                 * at the machine-code level under Release optimization. volatile forces the
                 * compiler to perform the read exactly where the source says to. */
                hlx_module_mirror_t volatile *cand = (hlx_module_mirror_t volatile *)(base + off);
                candidatesChecked++;

                /* base+off is only guaranteed committed as of the VirtualQuery snapshot above -
                 * another thread can decommit/free part of this region before these reads run (a
                 * real TOCTOU race, not hypothetical: this scan walks the ENTIRE process address
                 * space and can take a while, during which other threads - the renderer, a GC, an
                 * audio/asset streamer - keep allocating/freeing). Every field this loop needs off
                 * `cand` is snapshotted here, in one guarded block, rather than re-reading `cand`
                 * again later unguarded once a candidate looks promising. */
                bool candOk = true;
                void *jit_code = NULL;
                int codesize = 0;
                void *candCode = NULL;
                void **fps = NULL;
                void *globalsData = NULL;
                void *globalsIndexes = NULL;
                void *functionsIndexes = NULL;
                __try {
                    jit_code = (void *)cand->jit_code;
                    codesize = cand->codesize;
                    candCode = (void *)cand->code;
                    fps = (void **)cand->functions_ptrs;
                    globalsData = (void *)cand->globals_data;
                    globalsIndexes = (void *)cand->globals_indexes;
                    functionsIndexes = (void *)cand->functions_indexes;
                } __except (EXCEPTION_EXECUTE_HANDLER) {
                    candOk = false;
                }
                if (!candOk) continue;

                if (!jit_code) continue;
                if (codesize <= 0 || codesize > 0x10000000) continue;

                unsigned char *jitStart = (unsigned char *)jit_code;
                unsigned char *jitEnd = jitStart + codesize;
                if ((const unsigned char *)targetFun < jitStart || (const unsigned char *)targetFun >= jitEnd) continue;

                matchesFound++;

                void * volatile *volatileFps = (void * volatile *)fps;
                bool readOk = false;

                if (candCode && fps) {
                    /* entrypoint sits at a different offset in each supported hl_code layout, and
                     * which layout this candidate has is exactly what hl_compat has not decided
                     * yet - so probe every layout's value and let the fps[entrypoint]==targetFun
                     * identity below pick the one that is real. Every value still has to pass the
                     * same bounds gate as codesize above before it indexes volatileFps, since a
                     * wild one (this candidate is probably not a real hlx_module_mirror_t at all)
                     * would otherwise become an arbitrarily-offset read. */
                    int entryCandidates[4];
                    int nEntryCandidates = hl_compat_entrypoint_candidates(candCode, entryCandidates, 4);
                    for (int ei = 0; ei < nEntryCandidates && !readOk; ei++) {
                        if (entryCandidates[ei] < 0 || entryCandidates[ei] >= 10000000) continue;
                        __try {
                            void *p = (void *)volatileFps[entryCandidates[ei]];
                            if (p == targetFun) readOk = true;
                        } __except (EXCEPTION_EXECUTE_HANDLER) {
                            readOk = false;
                        }
                    }
                }

                if (candCode && fps && readOk && !foundCode) {
                    if (!hl_compat_detect_live(candCode, reflection_get_bytecode_version())) {
                        hlx_log(HLX_LOG_DEBUG, "[hlx-boot] FindPrimaryModule: candidate #%d at %p rejected by hl_compat - continuing scan", matchesFound, (void *)cand);
                        continue;
                    }
                    hlx_log(HLX_LOG_DEBUG, "[hlx-boot] FindPrimaryModule: candidate #%d at %p VALIDATED - functions_ptrs=%p", matchesFound, (void *)cand, fps);
                    foundCode = candCode;
                    if (outFunctionsPtrs) *outFunctionsPtrs = fps;
                    if (outGlobalsData) *outGlobalsData = globalsData;
                    if (outGlobalsIndexes) *outGlobalsIndexes = (int *)globalsIndexes;
                    if (outFunctionsIndexes) *outFunctionsIndexes = (int *)functionsIndexes;
                }
            }
        }
        unsigned char *next = (unsigned char *)mbi.BaseAddress + mbi.RegionSize;
        if (next <= addr) break;
        addr = next;
    }

    hlx_log(HLX_LOG_INFO, "[hlx-boot] FindPrimaryModule: done in %lums - %d regions, %lu candidates checked, %d match(es)", (unsigned long)(GetTickCount() - startTick), regionsScanned, candidatesChecked, matchesFound);
    return foundCode;
}

bool module_recover(const void *targetFun)
{
    void **functionsPtrs = NULL;
    void *globalsData = NULL;
    int *globalsIndexes = NULL;
    int *functionsIndexes = NULL;
    void *code = FindPrimaryModule(targetFun, &functionsPtrs, &globalsData, &globalsIndexes, &functionsIndexes);
    g_recoveredCode = code;
    g_functionsPtrsGlobal = functionsPtrs;
    g_globalsData = globalsData;
    g_globalsIndexes = globalsIndexes;
    g_functionsIndexes = functionsIndexes;
    if (!code) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] module_recover: no validated match this run - name-based resolution will fail closed until the game is restarted");
    } else {
        hlx_log(HLX_LOG_DEBUG, "[hlx-boot] module_recover: OK");
    }
    return code != NULL;
}

void *module_get_code(void)
{
    return g_recoveredCode;
}

bool module_is_recovered(void)
{
    return g_recoveredCode != NULL;
}

void **module_get_functions_ptrs(void)
{
    return g_functionsPtrsGlobal;
}

void *module_get_globals_data(void)
{
    return g_globalsData;
}

int *module_get_globals_indexes(void)
{
    return g_globalsIndexes;
}

int *module_get_functions_indexes(void)
{
    return g_functionsIndexes;
}
