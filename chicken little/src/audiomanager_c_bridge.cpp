#include "audiomanager_c_bridge.h"

#include "audiomanager.hpp"
#include "XAudioBuffer.hpp"

static AudioManager audioManager;

int XAudioBuffer_Create(const char* filename) {
  auto audioBuffer = std::make_unique<XAudioBuffer>(filename);
  return audioManager.AddBuffer(std::move(audioBuffer));
}

bool XAudioBuffer_Free(int bufferHandle) {
  return audioManager.FreeBuffer(bufferHandle);
}

void XAudioBuffer_Play(int bufferHandle) {
	// TODO: Implement me
}