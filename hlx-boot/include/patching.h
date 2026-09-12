#ifndef HLX_PATCHING_H
#define HLX_PATCHING_H

int install_patch(void *realAddress, const void *realType, void *receiverFn, const unsigned short *label);
void *call_original(int handle, void *argsArray);

#endif /* HLX_PATCHING_H */
