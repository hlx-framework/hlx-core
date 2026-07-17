#ifndef HLX_BOOT_H
#define HLX_BOOT_H

#include "log.h"

#ifdef _WIN32
#define HLX_API __declspec(dllexport)
#else
#define HLX_API __attribute__((visibility("default")))
#endif

/* HL resolves natives via a sign out-param; this generates that boilerplate. */
#define HLX_NATIVE_EXPORT(exportName, signature, impl) \
    HLX_API void *exportName(const char **sign) \
    { \
        *sign = signature; \
        return (void *)&impl; \
    }

void hlx_log(HlxLogLevel level, const char *fmt, ...);

#endif /* HLX_BOOT_H */
