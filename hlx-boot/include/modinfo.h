#ifndef HLX_MODINFO_H
#define HLX_MODINFO_H

#include <stdbool.h>
#include <stdint.h>

/* Reads an optional minGameVersion/maxGameVersion mod.info at modInfoPath and checks the given
 * version against it. Returns true (unconditionally compatible) if the file doesn't exist. */
bool modinfo_allows_version(const char *modInfoPath, int32_t major, int32_t minor, int32_t patch, int32_t build);

#endif /* HLX_MODINFO_H */
