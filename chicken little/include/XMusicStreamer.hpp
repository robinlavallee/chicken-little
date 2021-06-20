#pragma once

#include "XAudioBuffer.hpp"
#include "XSourceVoice.hpp"

#include <thread>
#include <windows.h>

class XMusicStreamer {
 public:
  XMusicStreamer(XSourceVoice* xSourceVoice, const std::string& filename);

 protected:
 private:

  void run();

  OVERLAPPED m_overlapped = {0};

  XSourceVoice* m_sourceVoice;
  std::string m_filename;
  std::unique_ptr<std::thread> m_thread;

  static const int STREAMING_BUFFER_SIZE = 65536;
  static const int MAX_BUFFER_COUNT = 3;
  std::vector<std::unique_ptr<XAudioBuffer>> m_xAudioBuffers;
};