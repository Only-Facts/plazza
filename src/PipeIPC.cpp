#include "PipeIPC.hpp"

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <unistd.h>

PipeIPC::PipeIPC()
  : _readFd(-1), _writeFd(-1)
{
}

PipeIPC::PipeIPC(int readFd, int writeFd)
  : _readFd(readFd), _writeFd(writeFd)
{
}

PipeIPC::~PipeIPC()
{
  closeAll();
}

PipeIPC::PipeIPC(PipeIPC&& other) noexcept
  : _readFd(other._readFd), _writeFd(other._writeFd)
{
  other._readFd = -1;
  other._writeFd = -1;
}

PipeIPC& PipeIPC::operator=(PipeIPC&& other) noexcept
{
  if (this != &other) {
    closeAll();
    _readFd = other._readFd;
    _writeFd = other._writeFd;
    other._readFd = -1;
    other._writeFd = -1;
  }
  return *this;
}

void PipeIPC::send(const std::string& message) const
{
  if (_writeFd == -1)
    throw std::runtime_error("Pipe write fd is closed");

  std::string data = message + "\n";
  const char* buffer = data.c_str();
  std::size_t remaining = data.size();

  while (remaining > 0) {
    ssize_t written = write(_writeFd, buffer, remaining);
    if (written == -1) {
      if (errno == EINTR)
        continue;
      throw std::runtime_error("write failed: " + std::string(std::strerror(errno)));
    }
    buffer += written;
    remaining -= static_cast<std::size_t>(written);
  }
}

std::string PipeIPC::receive() const
{
  if (_readFd == -1)
    throw std::runtime_error("Pipe read fd is closed");

  std::string result;
  char c = 0;

  while (true) {
    ssize_t bytesRead = read(_readFd, &c, 1);
    if (bytesRead == -1) {
      if (errno == EINTR)
        continue;
      throw std::runtime_error("read failed: " + std::string(std::strerror(errno)));
    }
    if (bytesRead == 0)
      throw std::runtime_error("pipe closed");
    if (c == '\n')
      break;
    result += c;
  }
  return result;
}

int PipeIPC::getReadFd() const
{
  return _readFd;
}

int PipeIPC::getWriteFd() const
{
  return _writeFd;
}

void PipeIPC::closeRead()
{
  if (_readFd != -1) {
    close(_readFd);
    _readFd = -1;
  }
}

void PipeIPC::closeWrite()
{
  if (_writeFd != -1) {
    close(_writeFd);
    _writeFd = -1;
  }
}

void PipeIPC::closeAll()
{
  closeRead();
  closeWrite();
}
