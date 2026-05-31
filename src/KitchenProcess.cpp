#include "KitchenProcess.hpp"
#include "Message.hpp"

KitchenProcess::KitchenProcess(int id, pid_t pid, PipeIPC toKitchen, PipeIPC fromKitchen, int capacity)
  : _id(id),
    _pid(pid),
    _toKitchen(std::move(toKitchen)),
    _fromKitchen(std::move(fromKitchen)),
    _load(0),
    _capacity(capacity)
{
}

KitchenProcess::KitchenProcess(KitchenProcess&& other) noexcept
  : _id(other._id),
    _pid(other._pid),
    _toKitchen(std::move(other._toKitchen)),
    _fromKitchen(std::move(other._fromKitchen)),
    _load(other._load),
    _capacity(other._capacity)
{
  other._pid = -1;
  other._load = 0;
  other._capacity = 0;
}

KitchenProcess& KitchenProcess::operator=(KitchenProcess&& other) noexcept {
  if (this != &other) {
    _id = other._id;
    _pid = other._pid;
    _toKitchen = std::move(other._toKitchen);
    _fromKitchen = std::move(other._fromKitchen);
    _load = other._load;
    _capacity = other._capacity;

    other._pid = -1;
    other._load = 0;
    other._capacity = 0;
  }
  return *this;
}

int KitchenProcess::getId() const {
  return _id;
}

pid_t KitchenProcess::getPid() const {
  return _pid;
}

int KitchenProcess::getLoad() const {
  return _load;
}

int KitchenProcess::getCapacity() const {
  return _capacity;
}

bool KitchenProcess::canAcceptPizza() const {
  return _load < _capacity;
}

void KitchenProcess::incrementLoad() {
  _load++;
}

void KitchenProcess::decrementLoad() {
  if (_load > 0)
    _load--;
}

void KitchenProcess::sendPizza(const Pizza& pizza) const {
  Message message(MessageType::NewPizza, pizza);

  _toKitchen.send(message.pack());
}

bool KitchenProcess::receiveDonePizza(Pizza& pizza) const {
  Message response = Message::unpack(_fromKitchen.receive());

  if (response.getType() != MessageType::PizzaDone || !response.hasPizza())
    return false;

  pizza = response.getPizza();
  return true;
}
