
#include "audiomanager.hpp"
#include "clsfxres.h"

#include <cassert>

AudioManager::AudioManager() {
  HRESULT hr;
  hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
  assert(!FAILED(hr));

  if (FAILED(hr = XAudio2Create(&m_pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR))) {
    assert(false);
  }
   
  if (FAILED(hr = m_pXAudio2->CreateMasteringVoice(&m_pMasterVoice))) {
    assert(false);
  }

  Init();
}

int AudioManager::Init() {
  if (!m_pXAudio2) {
    return -1;
  }
    
  WAVEFORMATEX wfx;
  wfx.wFormatTag = WAVE_FORMAT_PCM;
  wfx.nChannels = 2;
  wfx.nSamplesPerSec = 44100;
  wfx.wBitsPerSample = 16;
  wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
  wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.wBitsPerSample * wfx.nChannels / 8;
  wfx.cbSize = 0;

  const int MaxVoices = 8;
  m_sourceVoices.resize(MaxVoices);
  for (auto& sourceVoice : m_sourceVoices) {
    sourceVoice = std::make_unique<XSourceVoice>();
    sourceVoice->configure(m_pXAudio2, wfx);
  }

  return 0;
}

// Not thread-safe
int AudioManager::AddBuffer(std::unique_ptr<XAudioBuffer>& audioBuffer) {
  m_audioBuffers.emplace(m_nextAudioBufferIndex++, std::move(audioBuffer));
  return m_nextAudioBufferIndex;
}

void AudioManager::PlayBuffer(int bufferHandle) {
  auto it = m_audioBuffers.find(bufferHandle - 1);
  if (it == m_audioBuffers.end()) {
    return;
  }

  // TODO: Use empty voices
  m_sourceVoices[0]->play(*(*it).second.get());
}

bool AudioManager::FreeBuffer(int bufferHandle) {
  auto it = m_audioBuffers.find(bufferHandle-1);
  if (it == m_audioBuffers.end()) {
    return false;
  }

  m_audioBuffers.erase(it);

  return true;
}

int AudioManager::Shutdown() {

  // TODO: Make sure all voices have been stopped before
  m_sourceVoices.clear();
  m_audioBuffers.clear();

  if (m_pMasterVoice) {
    m_pMasterVoice->DestroyVoice();
    m_pMasterVoice = nullptr;
  }

  if (m_pXAudio2) {
    m_pXAudio2->Release();
    m_pXAudio2 = nullptr;
  }

  return 0;
}