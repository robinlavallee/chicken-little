
#include "XSourceVoice.hpp"

#include <cassert>
#include <algorithm>

XSourceVoice::XSourceVoice()
    : m_sourceVoice(nullptr) {}

XSourceVoice::XSourceVoice(IXAudio2SourceVoice* sourceVoice)
    : m_sourceVoice(sourceVoice) {}

XSourceVoice::~XSourceVoice() {
}

void XSourceVoice::release() {
  if (!m_sourceVoice) {
    return;
  }

  m_sourceVoice->Stop();

  m_sourceVoice->DestroyVoice();
  m_sourceVoice = nullptr;
}

HRESULT XSourceVoice::configure(IXAudio2* xAudio2, WAVEFORMATEX wfx) {
  HRESULT hr = xAudio2->CreateSourceVoice(&m_sourceVoice, reinterpret_cast<WAVEFORMATEX*>(&wfx), 0, 2.0F, this);
  if (FAILED(hr)) {
    m_sourceVoice = nullptr;
  } else {
    hr = m_sourceVoice->Start();
  }

  return hr;
}

void XSourceVoice::start() {
  m_sourceVoice->Start();
}

void XSourceVoice::stop() {
  m_sourceVoice->Stop();
  m_sourceVoice->FlushSourceBuffers();
}

HRESULT XSourceVoice::play(const XAudioBuffer& xAudioBuffer) {

  if (!m_sourceVoice) {
    return E_FAIL;
  }

  HRESULT hr = m_sourceVoice->SubmitSourceBuffer(&xAudioBuffer.getBuffer());

  if (FAILED(hr)) {
    assert(false);
  } 

  return hr;
}

size_t XSourceVoice::getQueuedCount() const {
  XAUDIO2_VOICE_STATE state;
  m_sourceVoice->GetState(&state);
  return state.BuffersQueued;
}

void XSourceVoice::OnBufferStart(void* pBufferContext) {
  if (!pBufferContext) {
    return;
  }

  XAudioBuffer* audioBuffer = reinterpret_cast<XAudioBuffer*>(pBufferContext);
  audioBuffer->onBufferStart();
}

void XSourceVoice::OnBufferEnd(void* pBufferContext) {
  if (!pBufferContext) {
    return;
  }
}

void XSourceVoice::OnVoiceError(void* /*pBufferContext*/, HRESULT /*error*/) {
  //TRACE_DEBUG("OnVoiceError: %08x", error);
}

