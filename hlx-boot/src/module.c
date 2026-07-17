#include "module.h"
#include "boot.h"
#include <windows.h>

typedef struct {
    void *code;
    int codesize;
    int globals_size;
    void *globals_indexes;
    void *globals_data;
    void *functions_ptrs;
    void *functions_indexes;
    void *jit_code;
} hlx_module_mirror_t;

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
                hlx_module_mirror_t *cand = (hlx_module_mirror_t *)(base + off);
                candidatesChecked++;

                if (!cand->jit_code) continue;
                if (cand->codesize <= 0 || cand->codesize > 0x10000000) continue;

                unsigned char *jitStart = (unsigned char *)cand->jit_code;
                unsigned char *jitEnd = jitStart + cand->codesize;
                if ((const unsigned char *)targetFun < jitStart || (const unsigned char *)targetFun >= jitEnd) continue;

                matchesFound++;

                hlx_code_mirror_t *code = (hlx_code_mirror_t *)cand->code;
                void **fps = (void **)cand->functions_ptrs;
                int entrypoint = -1;
                void *epPtr = NULL;
                bool readOk = false;

                if (code && fps) {
                    __try {
                        entrypoint = code->entrypoint;
                        epPtr = fps[entrypoint];
                        readOk = true;
                    } __except (EXCEPTION_EXECUTE_HANDLER) {
                        readOk = false;
                    }
                }

                if (code && fps && readOk && epPtr == targetFun && !foundCode) {
                    hlx_log(HLX_LOG_DEBUG, "[hlx-boot] FindPrimaryModule: candidate #%d at %p VALIDATED - functions_ptrs=%p", matchesFound, (void *)cand, fps);
                    foundCode = cand->code;
                    if (outFunctionsPtrs) *outFunctionsPtrs = fps;
                    if (outGlobalsData) *outGlobalsData = cand->globals_data;
                    if (outGlobalsIndexes) *outGlobalsIndexes = (int *)cand->globals_indexes;
                    if (outFunctionsIndexes) *outFunctionsIndexes = (int *)cand->functions_indexes;
                }
            }
        }
        unsigned char *next = (unsigned char *)mbi.BaseAddress + mbi.RegionSize;
        if (next <= addr) break;
        addr = next;
    }

    hlx_log(HLX_LOG_DEBUG, "[hlx-boot] FindPrimaryModule: done in %lums - %d regions, %lu candidates checked, %d match(es)", (unsigned long)(GetTickCount() - startTick), regionsScanned, candidatesChecked, matchesFound);
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
        hlx_log(HLX_LOG_INFO, "[hlx-boot] module_recover: OK");
    }
    return code != NULL;
}

void *module_get_code(void)
{
    return g_recoveredCode;
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
