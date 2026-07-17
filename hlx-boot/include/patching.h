#ifndef HLX_PATCHING_H
#define HLX_PATCHING_H

int install_patch(void *realAddress, const void *realType, void *receiverFn);
void *call_original(int handle, void *argsArray);

#endif /* HLX_PATCHING_H */
