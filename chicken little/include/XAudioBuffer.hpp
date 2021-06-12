#pragma once

#include <xaudio2.h>
#include <string>
#include <vector>

// Wrapper class over the XAUDIO2_BUFFER structure. Also manage sound data life-cycle.
class XAudioBuffer {
 public:
  XAudioBuffer(const std::string& inputFile);

  const XAUDIO2_BUFFER& getBuffer() const { return m_buffer; }

  bool hasRendered() const { return m_state == State::RENDERED; }

  void onBufferStart();
  void onBufferEnd();
  void onFlush();

 private:
  XAUDIO2_BUFFER m_buffer;
  std::vector<uint8_t> m_bytes;

  enum class State { UNRENDERED, STARTED, FLUSHED, RENDERED } m_state = State::UNRENDERED;
};