#pragma once

#include <xaudio2.h>
#include <vector>

#include "XSourceVoice.hpp"

class AudioManager {
 public:
  AudioManager();

  int Init();
  int Shutdown();
 private:
  IXAudio2* m_pXAudio2;
  IXAudio2MasteringVoice* m_pMasterVoice;

  std::vector<std::unique_ptr<XSourceVoice>> m_sourceVoices;
};