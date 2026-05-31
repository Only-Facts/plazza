#pragma once

#include "PipeIPC.hpp"
#include "Pizza.hpp"
#include "Message.hpp"

#include <sys/types.h>

class KitchenProcess {
public:
  KitchenProcess(int id, pid_t pid, PipeIPC toKitchen, PipeIPC fromKitchen, int capacity);
  ~KitchenProcess();

  KitchenProcess(const KitchenProcess&) = delete;
  KitchenProcess& operator=(const KitchenProcess&) = delete;

  KitchenProcess(KitchenProcess&& other) noexcept;
  KitchenProcess& operator=(KitchenProcess&& other) noexcept;

  int getId() const;
  pid_t getPid() const;
  int getLoad() const;
  int getCapacity() const;
  bool isClosing() const;

  bool canAcceptPizza() const;

  void incrementLoad();
  void decrementLoad();
  void markClosing();

  void sendPizza(const Pizza& pizza) const;
  void sendMessage(const Message& message) const;
  bool receiveMessage(Message& message) const;

  int getReadFd() const;
  void closePipes();

private:
  int _id;
  pid_t _pid;
  PipeIPC _toKitchen;
  PipeIPC _fromKitchen;
  int _load;
  int _capacity;
  bool _closing;
};
