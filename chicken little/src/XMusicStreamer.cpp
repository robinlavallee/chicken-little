#include "XMusicStreamer.hpp"

#include <assert.h>
#include <filesystem>

XMusicStreamer::XMusicStreamer(XSourceVoice* xSourceVoice, const std::string& filename) 
    : m_sourceVoice(xSourceVoice)
    , m_filename(filename) {

  assert(xSourceVoice);
  m_xAudioBuffers.resize(MAX_BUFFER_COUNT);
  for (auto& xBuffer : m_xAudioBuffers) {
    xBuffer = std::make_unique<XAudioBuffer>(STREAMING_BUFFER_SIZE);
  }

  m_playMusic = false;
  m_destroy = false;

  m_thread = std::make_unique<std::thread>([this]() { run(); });
}

void XMusicStreamer::processCommands() {
  // check for any pending command
  Command cmd = Command::None;
  {
    std::lock_guard<std::mutex> lock(m_commandsMutex);
    if (!m_commands.empty()) {
      cmd = m_commands.front();
      m_commands.pop();
    }
  }

  if (cmd == Command::Play) {
    m_sourceVoice->start();
    m_playMusic = true;
  } else if (cmd == Command::Stop) {
    m_playMusic = false;
    m_sourceVoice->stop();
    SetFilePointer(m_hFile, 0, 0, FILE_BEGIN);
    m_overlapped.Offset = m_overlapped.OffsetHigh = 0;
  } else if (cmd == Command::Destroy) {
    m_playMusic = false;
    m_sourceVoice->stop();
    m_destroy = true;
  }
}

void XMusicStreamer::play() {
  std::lock_guard<std::mutex> lock(m_commandsMutex);
  m_commands.push(Command::Play);
}

void XMusicStreamer::stop() {
  std::lock_guard<std::mutex> lock(m_commandsMutex);
  m_commands.push(Command::Stop);
}

void XMusicStreamer::destroy() {
  {
    std::lock_guard<std::mutex> lock(m_commandsMutex);
    m_commands.push(Command::Destroy);
  }

  if (m_thread) {
    m_thread->join();
  }
}
 

void XMusicStreamer::run() {
  m_overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

  std::filesystem::path p(m_filename);
  unsigned int cbWaveSize = std::filesystem::file_size(p);

  OFSTRUCT ofstruct;
  memset(&ofstruct, 0, sizeof(ofstruct));
  ofstruct.cBytes = sizeof(ofstruct);
  m_hFile = (HANDLE)OpenFile(m_filename.c_str(), &ofstruct, OF_READ);

  while (!m_destroy) {
    unsigned int CurrentDiskReadBuffer = 0;
    unsigned int CurrentPosition = 0;
    HRESULT hr = S_OK;
    m_overlapped.Offset = m_overlapped.OffsetHigh = 0;
    while (CurrentPosition < cbWaveSize) {
      processCommands();

      while (!m_playMusic) {
        if (m_destroy) {
          break;
        }
        ::Sleep(10);
        processCommands();
      }

      if (m_destroy) {
        break;
      }
    
      while (m_sourceVoice->getQueuedCount() >= MAX_BUFFER_COUNT) {
        ::Sleep(5);
      }

      DWORD cbValid = min(STREAMING_BUFFER_SIZE, cbWaveSize - CurrentPosition);
      DWORD dwRead;
      if (0 == ReadFile(m_hFile, m_xAudioBuffers[CurrentDiskReadBuffer]->getData(), STREAMING_BUFFER_SIZE, &dwRead,
                 &m_overlapped))
        hr = HRESULT_FROM_WIN32(GetLastError());
      m_overlapped.Offset += cbValid;

      // update the file position to where it will be once the read finishes
      CurrentPosition += cbValid;

      DWORD NumberBytesTransferred;
      ::GetOverlappedResult(m_hFile, &m_overlapped, &NumberBytesTransferred, TRUE);

      XAUDIO2_BUFFER& xAudio2Buffer = m_xAudioBuffers[CurrentDiskReadBuffer]->getBuffer();
      xAudio2Buffer.AudioBytes = cbValid;
      m_sourceVoice->play(*m_xAudioBuffers[CurrentDiskReadBuffer]);
      CurrentDiskReadBuffer++;
      CurrentDiskReadBuffer %= MAX_BUFFER_COUNT;
    }

    hr = SetFilePointer(m_hFile, 0, 0, FILE_BEGIN);
    assert(hr == S_OK);
  }
  
  CloseHandle(m_hFile);
}