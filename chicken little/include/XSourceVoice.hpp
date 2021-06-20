#pragma once

#include <xaudio2.h>
#include <mutex>
#include <unordered_set>

#include "XAudioBuffer.hpp"

// Wrapper over IXAudio2SourceVoice, handle life-cycle as well
class XSourceVoice : public IXAudio2VoiceCallback {
 public:
  XSourceVoice();
  XSourceVoice(IXAudio2SourceVoice* sourceVoice);
  ~XSourceVoice();

  XSourceVoice(const XSourceVoice& rhs) = delete;
  XSourceVoice& operator=(const XSourceVoice& rhs) = delete;

  HRESULT configure(IXAudio2* xAudio2, WAVEFORMATEX wfx);
  HRESULT play(const XAudioBuffer& xAudioBuffer);
  HRESULT start();
  HRESULT stop();

 protected:
  // IXAudio2VoiceCallback overrides
  void __stdcall OnBufferStart(void* pBufferContext) override;
  void __stdcall OnBufferEnd(void* pBufferContext) override;
  void __stdcall OnLoopEnd(void* /*pBufferContext*/) override {}
  void __stdcall OnStreamEnd() override {}
  void __stdcall OnVoiceError(void* /*pBufferContext*/, HRESULT /*Error*/) override;
  void __stdcall OnVoiceProcessingPassEnd() override {}
  void __stdcall OnVoiceProcessingPassStart(UINT32 /*bytesRequired*/) override {}

 private:
  void release();
 
  // Currently active voice buffers
  std::unordered_set<const XAudioBuffer*> m_xBuffers;

  // Voice buffers queued to be removed
  std::unordered_set<const XAudioBuffer*> m_toRemove;
  std::mutex m_mutex;  // to protect concurrent access m_toRemove

  IXAudio2SourceVoice* m_sourceVoice;

  std::thread::id m_threadId;

};