#ifndef HLX_CONFIG_H
#define HLX_CONFIG_H

#include <stdbool.h>
#include "log.h"

void LoadLoaderConfig(void);
bool ConfigGameTraceEnabled(void);
HlxLogLevel ConfigGetLogLevel(void);

#endif /* HLX_CONFIG_H */
