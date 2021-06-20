#pragma once

#include <xaudio2.h>
#include <string>
#include <vector>

// Wrapper class over the XAUDIO2_BUFFER structure. Also manage sound data life-cycle.
class XAudioBuffer {
 public:
  // Create an XAudioBuffer given a PCM sample from disk
  XAudioBuffer(const std::string& inputFile);

  // Create an XAudioBuffer given a specific buffer size
  XAudioBuffer(const size_t bufferSize);

  const XAUDIO2_BUFFER& getBuffer() const { return m_buffer; }
  XAUDIO2_BUFFER& getBuffer() { return m_buffer; }

  uint8_t* getData() { return m_bytes.data(); }

  bool hasRendered() const { return m_state == State::RENDERED; }

  void onBufferStart();
  void onBufferEnd();

 private:
  XAUDIO2_BUFFER m_buffer;
  std::vector<uint8_t> m_bytes;

  enum class State { STARTED, RENDERED } m_state = State::STARTED;
};
