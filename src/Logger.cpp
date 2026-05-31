#include "Logger.hpp"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>

void Logger::log(const std::string& message)
{
  static std::mutex mutex;
  std::lock_guard<std::mutex> lock(mutex);

  std::filesystem::create_directories("logs");

  std::ofstream file("logs/plazza.log", std::ios::app);
  if (!file)
    return;

  auto now = std::chrono::system_clock::now();
  std::time_t time = std::chrono::system_clock::to_time_t(now);
  std::tm tm = *std::localtime(&time);

  file << "[" << std::put_time(&tm, "%F %T") << "] " << message << std::endl;
}
