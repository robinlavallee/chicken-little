
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

static WAVEFORMATEX getWaveFormat() {
  WAVEFORMATEX wfx;
  wfx.wFormatTag = WAVE_FORMAT_PCM;
  wfx.nChannels = 2;
  wfx.nSamplesPerSec = 44100;
  wfx.wBitsPerSample = 16;
  wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
  wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.wBitsPerSample * wfx.nChannels / 8;
  wfx.cbSize = 0;

  return wfx;
}

int AudioManager::Init() {
  if (!m_pXAudio2) {
    return -1;
  }
    
  WAVEFORMATEX wfx = getWaveFormat();

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

  // TODO: Use empty voices instead of cycling like this
  static int currentVoice = 0;
  m_sourceVoices[currentVoice++]->play(*(*it).second.get());
  if (currentVoice >= MaxVoices) {
    currentVoice = 0;
  }
}

bool AudioManager::FreeBuffer(int bufferHandle) {
  auto it = m_audioBuffers.find(bufferHandle-1);
  if (it == m_audioBuffers.end()) {
    return false;
  }

  m_audioBuffers.erase(it);

  return true;
}

int AudioManager::LoadMusic(const std::string& filename) {
  m_musics[m_nextMusicIndex++] = MusicFile(m_pXAudio2, filename);
  return m_nextMusicIndex;
}

void AudioManager::PlayMusic(int musicHandle) {
  auto it = m_musics.find(musicHandle-1);
  if (it != m_musics.end()) {
    (*it).second.m_musicStreamer->play();
  }
}

void AudioManager::StopMusic(int musicHandle) {
  auto it = m_musics.find(musicHandle - 1);
  if (it != m_musics.end()) {
    (*it).second.m_musicStreamer->stop();
  }
}

// Implement me
bool AudioManager::FreeMusic(int musicHandle) {
  //assert(false);
  return false;
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

AudioManager::MusicFile::MusicFile(IXAudio2* xAudio2, const std::string& _path) 
    : m_pXAudio2(xAudio2)
    , path(_path) {
  m_musicVoice = std::make_unique<XSourceVoice>();

  WAVEFORMATEX wfx = getWaveFormat();
  m_musicVoice->configure(m_pXAudio2, wfx);

  m_musicStreamer = std::make_unique<XMusicStreamer>(m_musicVoice.get(), path);
}
