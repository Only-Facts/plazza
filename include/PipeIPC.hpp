#pragma once

#include <string>

class PipeIPC {
public:
  PipeIPC();
  PipeIPC(int readFd, int writeFd);
  ~PipeIPC();

  PipeIPC(const PipeIPC&) = delete;
  PipeIPC& operator=(const PipeIPC&) = delete;

  PipeIPC(PipeIPC&& other) noexcept;
  PipeIPC& operator=(PipeIPC&& other) noexcept;

  void send(const std::string& message) const;
  std::string receive() const;

  int getReadFd() const;
  int getWriteFd() const;

  void closeRead();
  void closeWrite();
  void closeAll();

private:
  int _readFd;
  int _writeFd;
};
