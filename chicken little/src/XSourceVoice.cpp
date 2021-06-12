
#include "XSourceVoice.hpp"

#include <cassert>
#include <algorithm>

const int MaxQueuedAudioBuffers = 4;  // TODO: TrackBuffer end of stream latency shouldn't depend on audio buffer size.
static_assert(MaxQueuedAudioBuffers <= XAUDIO2_MAX_QUEUED_BUFFERS,
  "MaxQueuedAudioBuffers must be less or equal to XAUDIO2_MAX_QUEUED_BUFFERS");

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

  stop();
 

  garbageCollectBuffers();

  m_sourceVoice->DestroyVoice();
  m_sourceVoice = nullptr;
}

HRESULT XSourceVoice::configure(IXAudio2* xAudio2, WAVEFORMATEX wfx) {
  HRESULT hr = xAudio2->CreateSourceVoice(&m_sourceVoice, reinterpret_cast<WAVEFORMATEX*>(&wfx), 0, 2.0F, this);
  if (FAILED(hr)) {
    m_sourceVoice = nullptr;
  }

  if (m_sourceVoice) {
    m_sourceVoice->SetVolume(m_volume, XAUDIO2_COMMIT_NOW);
  }

  return hr;
}


HRESULT XSourceVoice::start() {
  if (!m_sourceVoice) {
    return E_FAIL;
  }

  return m_sourceVoice->Start();
}

HRESULT XSourceVoice::stop() {
  if (!m_sourceVoice) {
    return E_FAIL;
  }

  return m_sourceVoice->Stop();
}
//
//HRESULT XSourceVoice::submit(const windows::AudioSample& audioSample) {
//  // TODO: Could be called every second or so instead of each call
//  garbageCollectBuffers();
//
//  if (!m_sourceVoice) {
//    return E_FAIL;
//  }
//
//  // We need to make a copy of the MediaSample because it will be recycled and must remain valid while the voice is
//  // playing MSDN: The audio sample data to which buffer points is still 'owned' by the app and must remain allocated
//  // and accessible until the sound stops playing.
//  XAudioBuffer* xBuffer = new XAudioBuffer(audioSample);
//
//  static const int MaxWaitMilliseconds = 2000;
//  bool success = m_semaphore.wait(MaxWaitMilliseconds);
//  if (!success) {
//    TRACE_DEBUG("XSourceVoice::submit - Previous voice hasn't completed after %d milliseconds. Preventing deadlock.",
//      MaxWaitMilliseconds);
//    m_semaphore.notify();
//    return XAUDIO2_E_DEVICE_INVALIDATED;
//  }
//
//  m_sourceVoice->SetFrequencyRatio(m_setPlaybackRate, XAUDIO2_COMMIT_NOW);
//
//  HRESULT hr = m_sourceVoice->SubmitSourceBuffer(&xBuffer->getBuffer());
//
//  if (FAILED(hr)) {
//    assert(false);
//    m_semaphore.notify();
//  } else {
//    m_xBuffers.insert(xBuffer);
//  }
//
//  return hr;
//}

void XSourceVoice::setVolume(float volume) {
  m_volume = volume;
  if (m_sourceVoice) {
    m_sourceVoice->SetVolume(volume, XAUDIO2_COMMIT_NOW);
  }
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

void XSourceVoice::garbageCollectBuffers() {
  //ensureSameThread();

  decltype(m_toRemove) workCopy;
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    workCopy = m_toRemove;
    m_toRemove.clear();
  }

  std::for_each(workCopy.begin(), workCopy.end(), [this](const XAudioBuffer* buf) {
    m_xBuffers.erase(buf);
    delete buf;
  });
}

void XSourceVoice::setPlaybackRate(float rate) {
  m_setPlaybackRate = rate;
}
