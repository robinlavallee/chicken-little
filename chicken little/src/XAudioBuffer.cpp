#pragma once

#include "XAudioBuffer.hpp"
#include "XSourceVoice.hpp"

#include <cassert>
#include <fstream>

// Assume:
// 16 bits signed
// 44100
// Stereo or Mono
// ffplay -f s16le -ar 44100 -ac 2 0.bin
XAudioBuffer::XAudioBuffer(const std::string& inputFile) {
  std::ifstream stream(inputFile, std::ios::in | std::ios::binary);
  stream.unsetf(std::ios::skipws);

  stream.seekg(0, std::ios::end);
  size_t filesize = (size_t)stream.tellg();
  stream.seekg(0, std::ios::beg);

  m_bytes.reserve(filesize);
  m_bytes.insert(m_bytes.begin(), std::istream_iterator<BYTE>(stream), std::istream_iterator<BYTE>());

  m_buffer.Flags = 0;
  m_buffer.AudioBytes = static_cast<UINT32>(m_bytes.size());
  assert(m_buffer.AudioBytes <= XAUDIO2_MAX_BUFFER_BYTES);

  m_buffer.pAudioData = m_bytes.data();
  m_buffer.PlayBegin = 0;
  m_buffer.PlayLength = 0;
  m_buffer.LoopBegin = 0;
  m_buffer.LoopLength = 0;
  m_buffer.LoopCount = 0;
  m_buffer.pContext = this;
}

void XAudioBuffer::onBufferStart() {
  assert(m_state == State::UNRENDERED);
  m_state = State::STARTED;
}

void XAudioBuffer::onBufferEnd() {
  if (m_state == State::STARTED) {
    m_state = State::RENDERED;
  }
}

void XAudioBuffer::onFlush() {
  m_state = State::FLUSHED;
}
