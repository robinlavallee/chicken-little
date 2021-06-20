
#include "XSourceVoice.hpp"

#include <cassert>
#include <algorithm>

XSourceVoice::XSourceVoice()
    : m_sourceVoice(nullptr), m_threadId(std::this_thread::get_id()) {}

XSourceVoice::XSourceVoice(IXAudio2SourceVoice* sourceVoice)
    : m_sourceVoice(sourceVoice), m_threadId(std::this_thread::get_id()) {}

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

  //XAudioBuffer* audioBuffer = reinterpret_cast<XAudioBuffer*>(pBufferContext);
  //audioBuffer->onBufferEnd();
  //if (audioBuffer->hasRendered()) {
  //  auto sample = audioBuffer->getSample();
  //  m_lastRenderedTime = sample.decodeTime + sample.duration;
  //}

  //{
  //  std::lock_guard<std::mutex> lock(m_mutex);
  //  // Add 'to be deleted'. Must be fast since this is a XA2 callback. (<1ms).
  //  assert(m_toRemove.find(audioBuffer) == m_toRemove.end());
  //  m_toRemove.insert(audioBuffer);
  //}

  //m_semaphore.notify();
}

void XSourceVoice::OnVoiceError(void* /*pBufferContext*/, HRESULT /*error*/) {
  //TRACE_DEBUG("OnVoiceError: %08x", error);
}

