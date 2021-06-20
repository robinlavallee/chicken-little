#include "XMusicStreamer.hpp"

#include <assert.h>
#include <filesystem>

XMusicStreamer::XMusicStreamer(XSourceVoice* xSourceVoice, const char* filename) 
    : m_sourceVoice(xSourceVoice)
    , m_filename(filename) {

  m_xAudioBuffers.resize(MAX_BUFFER_COUNT);
  for (auto& xBuffer : m_xAudioBuffers) {
    xBuffer = std::make_unique<XAudioBuffer>(STREAMING_BUFFER_SIZE);
  }

  m_thread = std::make_unique<std::thread>([this]() { run(); });
}

void XMusicStreamer::run() {
  m_overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

  std::filesystem::path p(m_filename);
  unsigned int cbWaveSize = std::filesystem::file_size(p);

  OFSTRUCT ofstruct;
  memset(&ofstruct, 0, sizeof(ofstruct));
  ofstruct.cBytes = sizeof(ofstruct);
  HANDLE hFile = (HANDLE)OpenFile(m_filename.c_str(), &ofstruct, OF_READ);

loop:
  unsigned int CurrentDiskReadBuffer = 0;
  unsigned int CurrentPosition = 0;
  HRESULT hr = S_OK;
  while (CurrentPosition < cbWaveSize) {

    DWORD cbValid = min(STREAMING_BUFFER_SIZE, cbWaveSize - CurrentPosition);
    DWORD dwRead;
    if (0 == ReadFile(hFile, m_xAudioBuffers[CurrentDiskReadBuffer]->getData(), STREAMING_BUFFER_SIZE, &dwRead, &m_overlapped))
      hr = HRESULT_FROM_WIN32(GetLastError());
    m_overlapped.Offset += cbValid;

    // update the file position to where it will be once the read finishes
    CurrentPosition += cbValid;

    DWORD NumberBytesTransferred;
    ::GetOverlappedResult(hFile, &m_overlapped, &NumberBytesTransferred, TRUE);

    // I think this should be higher in the loop instead? otherwise we risk overwriting the first buffer if we read fast?
    while (m_sourceVoice->getQueuedCount() >= MAX_BUFFER_COUNT) {
      ::Sleep(10);
    }

    XAUDIO2_BUFFER& xAudio2Buffer = m_xAudioBuffers[CurrentDiskReadBuffer]->getBuffer();
    xAudio2Buffer.AudioBytes = cbValid;
    m_sourceVoice->play(*m_xAudioBuffers[CurrentDiskReadBuffer]);
    CurrentDiskReadBuffer++;
    CurrentDiskReadBuffer %= MAX_BUFFER_COUNT;
  }

  hr = SetFilePointer(hFile, 0, 0, FILE_BEGIN);
  assert(hr == S_OK);
  goto loop;

  CloseHandle(hFile);
}