#ifndef _AUDIOMANAGER_C_BRIDGE_H_

#ifdef __cplusplus 
#define __EXTERN_C__ extern "C"
#else
#define __EXTERN_C__
#endif

// Create an XAudioBuffer and return its handle
__EXTERN_C__ int XAudioBuffer_Create(const char* filename);

// Free an XAudioBuffer through its handle. Returns true on success
__EXTERN_C__ bool XAudioBuffer_Free(int);

// Play an XAudioBuffer through its handle
__EXTERN_C__ void XAudioBuffer_Play(int);

#undef __EXTERN_C__

#endif // #ifndef __AUDIOMANAGER_C_BRIDGE_H__
