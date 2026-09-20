#ifndef HLX_DRIVER_H
#define HLX_DRIVER_H

#include <stdbool.h>
#include <stdint.h>

/* Lazily LoadLibrary's hlx/drivers/game_version/game_version.dll and calls its
 * hlx_driver_game_version_v1 export, once, caching the result for the rest of the process -
 * returns false (leaving the outs untouched) if the driver is absent, exports nothing by that
 * name, or fails to resolve a version. Callers must treat false as "unknown," not "0.0.0.0". */
bool driver_try_get_game_version(int32_t *outMajor, int32_t *outMinor, int32_t *outPatch, int32_t *outBuild);

#endif /* HLX_DRIVER_H */
