#pragma once

#include <xaudio2.h>
#include <vector>
#include <map>

#include "XAudioBuffer.hpp"
#include "XSourceVoice.hpp"

#include "audiomanager_c_bridge.h"

class AudioManager {
 public:
  AudioManager();

  int Init();
  int Shutdown();

  // Add an audio buffer, returns the HANDLE of the allocated buffer
  int AddBuffer(std::unique_ptr<XAudioBuffer>& audioBuffer);

  void PlayBuffer(int bufferHandle);

  // Free the buffer given an HANDLE obtained from AddBuffer
  bool FreeBuffer(int bufferHandle);

  const int MaxVoices = 32;

 private:
  IXAudio2* m_pXAudio2;
  IXAudio2MasteringVoice* m_pMasterVoice;

  std::vector<std::unique_ptr<XSourceVoice>> m_sourceVoices;

  std::map<int, std::unique_ptr<XAudioBuffer>> m_audioBuffers;
  int m_nextAudioBufferIndex = 0;
};