# Developer Guide

This document explains the main features of Plazza and how to extend the project.

---

## 1. Project architecture

Plazza is split into two main runtime parts:

```text
Reception process
|
|-- KitchenManager
|   |-- KitchenProcess #1
|   |-- KitchenProcess #2
|   `-- ...
|
`-- poll() loop
    |-- reads user input
    `-- listens for kitchen messages

Kitchen child process
|
|-- Kitchen
|   |-- ThreadPool
|   |-- Stock
|   `-- stock regeneration thread
|
`-- IPC loop
    |-- receives NEW_PIZZA
    |-- receives STATUS_REQUEST
    `-- exits after inactivity
```

The reception never cooks pizzas directly.
It parses orders, creates kitchen processes, sends pizzas through IPC, and receives completion/status messages.

Each kitchen process owns its own `Kitchen`, `ThreadPool`, and `Stock`.

---

## 2. Main features

### Interactive reception

The reception reads commands from standard input.

Supported commands:

```text
margarita S x1
regina XXL x2; fantasia M x3
status
exit
```

Handled mainly by:

```text
Reception.hpp
Reception.cpp
Parser.hpp
Parser.cpp
```

---

### Order parser

The parser validates user orders using the expected grammar:

```text
TYPE SIZE NUMBER [; TYPE SIZE NUMBER]*
```

Example:

```text
regina XXL x2; margarita S x1
```

The parser converts this into individual `Pizza` objects.

Example:

```text
regina XXL x2
```

becomes:

```text
Pizza(Regina, XXL)
Pizza(Regina, XXL)
```

Files:

```text
Parser.hpp
Parser.cpp
Pizza.hpp
Pizza.cpp
```

---

### Pizza model

Pizza types use the required enum values:

```cpp
enum class PizzaType {
  Regina = 1,
  Margarita = 2,
  Americana = 4,
  Fantasia = 8
};
```

Pizza sizes use:

```cpp
enum class PizzaSize {
  S = 1,
  M = 2,
  L = 4,
  XL = 8,
  XXL = 16
};
```

Files:

```text
Pizza.hpp
Pizza.cpp
```

---

### Kitchen processes

Kitchens are created with `fork()`.

The parent process keeps a `KitchenProcess` object that stores:

```text
- kitchen id
- process pid
- pipe to kitchen
- pipe from kitchen
- current estimated load
- max capacity
```

The real `Kitchen` object lives inside the child process.

Files:

```text
KitchenManager.hpp
KitchenManager.cpp
KitchenProcess.hpp
KitchenProcess.cpp
Kitchen.hpp
Kitchen.cpp
```

---

### IPC system

Reception and kitchens communicate through pipes.

Messages are serialized as strings.

Examples:

```text
NEW_PIZZA|PIZZA|2|1
PIZZA_DONE|PIZZA|2|1
STATUS_REQUEST
STATUS_RESPONSE|TEXT|...
KITCHEN_CLOSING
```

Files:

```text
PipeIPC.hpp
PipeIPC.cpp
Message.hpp
Message.cpp
```

---

### Thread pool

Each kitchen owns a `ThreadPool`.

The thread pool contains:

```text
- one thread per cook
- a queue of pizza tasks
- a mutex
- a condition variable
- a completion callback
```

When a pizza is added to the kitchen, it is pushed into the thread pool queue.
A cook thread wakes up, consumes ingredients, waits for the cooking time, then reports completion.

Files:

```text
ThreadPool.hpp
ThreadPool.cpp
Kitchen.hpp
Kitchen.cpp
```

---

### Stock system

Each kitchen owns a `Stock`.

Every kitchen starts with 5 units of each ingredient:

```text
dough
tomato
gruyere
ham
mushrooms
steak
eggplant
goat cheese
chief love
```

The stock regenerates 1 unit of every ingredient every `restock_time_ms`.

The stock is thread-safe because it can be accessed by:

```text
- cook threads
- status requests
- stock regeneration thread
```

Files:

```text
Stock.hpp
Stock.cpp
```

---

### Status command

When the user types:

```text
status
```

The reception sends a `STATUS_REQUEST` message to every active kitchen process.

Each kitchen responds with `STATUS_RESPONSE`.

The reception then displays the received status text.

Files:

```text
Reception.cpp
KitchenManager.cpp
Kitchen.cpp
Message.cpp
```

---

### Auto-close

A kitchen closes if it stays inactive for more than 5 seconds.

A kitchen is considered inactive when:

```text
load == 0
```

and no new pizza arrives for 5 seconds.

When closing, the kitchen sends:

```text
KITCHEN_CLOSING
```

to the reception.

The reception removes it from the active kitchen list and calls `waitpid()` to clean the process.

Files:

```text
KitchenManager.cpp
KitchenProcess.cpp
```

---

## 3. How to add a new pizza

To add a new pizza, update these files:

```text
Pizza.hpp
Pizza.cpp
Stock.cpp
```

### Step 1: add a new pizza type

In `Pizza.hpp`, add a new enum value.

Example:

```cpp
enum class PizzaType {
  Regina = 1,
  Margarita = 2,
  Americana = 4,
  Fantasia = 8,
  Calzone = 16
};
```

Use a power of two to stay consistent with the subject style.

---

### Step 2: add string conversion

In `Pizza.cpp`, update `stringToType()`:

```cpp
if (value == "calzone")
  return PizzaType::Calzone;
```

Update `typeToString()`:

```cpp
case PizzaType::Calzone:
  return "calzone";
```

---

### Step 3: add ingredients

In `Stock.cpp`, update the function that maps pizzas to ingredients.

Example:

```cpp
case PizzaType::Calzone:
  return {
    Ingredient::Dough,
    Ingredient::Tomato,
    Ingredient::Gruyere,
    Ingredient::Ham
  };
```

---

### Step 4: add cooking time

In `ThreadPool.cpp`, update the cooking time function.

Example:

```cpp
case PizzaType::Calzone:
  baseTime = 3000;
  break;
```

---

### Step 5: test it

Run:

```bash
make re
./plazza 0.1 2 2000
```

Then test:

```text
calzone M x2
```

---

## 4. How to add a new pizza size

Update:

```text
Pizza.hpp
Pizza.cpp
```

### Step 1: add the enum value

```cpp
enum class PizzaSize {
  S = 1,
  M = 2,
  L = 4,
  XL = 8,
  XXL = 16,
  XXXL = 32
};
```

---

### Step 2: update string conversion

In `Pizza.cpp`, update `stringToSize()`:

```cpp
if (value == "XXXL")
  return PizzaSize::XXXL;
```

Update `sizeToString()`:

```cpp
case PizzaSize::XXXL:
  return "XXXL";
```

---

### Step 3: update parser validation

If the parser has a strict size validation list, add:

```text
XXXL
```

to the allowed sizes.

---

## 5. How to add a new ingredient

Update:

```text
Stock.hpp
Stock.cpp
```

### Step 1: add the ingredient enum

```cpp
enum class Ingredient {
  Dough,
  Tomato,
  Gruyere,
  Ham,
  Mushrooms,
  Steak,
  Eggplant,
  GoatCheese,
  ChiefLove,
  Onion
};
```

---

### Step 2: initialize it

In `Stock::Stock()`:

```cpp
_ingredients[Ingredient::Onion] = 5;
```

---

### Step 3: add string conversion

In `ingredientToString()`:

```cpp
case Ingredient::Onion:
  return "onion";
```

---

### Step 4: use it in a pizza recipe

```cpp
case PizzaType::Calzone:
  return {
    Ingredient::Dough,
    Ingredient::Tomato,
    Ingredient::Onion
  };
```

---

## 6. How to add a new IPC message

Update:

```text
Message.hpp
Message.cpp
KitchenManager.cpp
Kitchen.cpp
Reception.cpp
```

### Step 1: add a new message type

In `Message.hpp`:

```cpp
enum class MessageType {
  NewPizza,
  PizzaDone,
  StatusRequest,
  StatusResponse,
  KitchenClosing,
  Error,
  Unknown
};
```

---

### Step 2: add conversion to string

In `Message.cpp`:

```cpp
case MessageType::Error:
  return "ERROR";
```

---

### Step 3: add conversion from string

```cpp
if (value == "ERROR")
  return MessageType::Error;
```

---

### Step 4: handle it where needed

For a kitchen-to-reception message, handle it in the reception polling code.

Example:

```cpp
if (message.getType() == MessageType::Error) {
  // print or log the error
}
```

For a reception-to-kitchen message, handle it in the kitchen child loop.

---

## 7. How to add a new shell command

Update:

```text
Reception.cpp
```

The reception shell checks user input.

Example command:

```text
help
```

Add a condition before parsing pizza orders:

```cpp
if (input == "help") {
  displayHelp();
  continue;
}
```

Then add:

```cpp
void Reception::displayHelp() const
{
  std::cout << "Available commands:" << std::endl;
  std::cout << "- status" << std::endl;
  std::cout << "- exit" << std::endl;
  std::cout << "- <pizza> <size> x<number>" << std::endl;
}
```

Do not forget to declare the method in `Reception.hpp`.

---

## 8. How to change load balancing

Load balancing is handled by `KitchenManager`.

Look for the function that chooses the best kitchen.

Current strategy:

```text
choose the kitchen with the lowest load
```

You can change it to another strategy, for example:

```text
choose the kitchen with the most free slots
choose the oldest kitchen
choose the kitchen with enough ingredients
```

Files:

```text
KitchenManager.hpp
KitchenManager.cpp
```

Important rule:

A kitchen must not accept more than:

```text
2 * cooks_per_kitchen
```

pizzas at the same time.

---

## 9. How to change cooking behavior

Cooking behavior lives in:

```text
ThreadPool.cpp
```

The important function is the one that cooks a pizza.

Typical flow:

```text
1. Try to consume ingredients.
2. Print/log cooking start.
3. Sleep for the cooking duration.
4. Print/log cooking end.
5. Trigger completion callback.
```

To change cooking time, update the cooking time function.

To change what happens after cooking, update the completion callback logic.

---

## 10. How to change stock regeneration

Stock regeneration is managed by the kitchen.

Relevant files:

```text
Kitchen.hpp
Kitchen.cpp
Stock.cpp
```

Current behavior:

```text
every restock_time_ms:
    add 1 unit to every ingredient
```

To cap stock at 5, change `Stock::regenerate()`:

```cpp
for (auto& pair : _ingredients) {
  if (pair.second < 5)
    pair.second++;
}
```

To allow unlimited growth, keep the current behavior.

---

## 11. How to debug IPC

Useful places to add logs:

```text
KitchenManager::sendPizza
KitchenProcess::sendPizza
PipeIPC::send
PipeIPC::receive
Message::pack
Message::unpack
```

Common IPC problems:

```text
Program blocks forever:
    probably waiting on receive()

Broken pipe:
    the other process closed its read side

No message received:
    wrong pipe side closed or used

Message unpack error:
    serialized format is wrong
```

Important rule:

Always close unused pipe ends after `fork()`.

Parent should close child-only pipe ends.
Child should close parent-only pipe ends.

---

## 12. How to debug processes

Useful commands:

```bash
ps aux | grep plazza
```

Check for zombie processes:

```bash
ps aux | grep defunct
```

If zombies appear, check:

```text
waitpid()
cleanupClosedKitchens()
SIGCHLD handling if used
```

The reception must clean children when they exit.

---

## 13. How to debug threads

Common thread problems:

```text
Mixed output:
    protect std::cout with a mutex

Random stock values:
    protect stock with a mutex

Program does not exit:
    some thread was not joined

Cook never wakes up:
    condition_variable was not notified
```

Relevant files:

```text
ThreadPool.cpp
Stock.cpp
Kitchen.cpp
```

---

## 14. Suggested commit structure for future changes

Good commit examples:

```bash
git commit -m "Add calzone pizza type"
git commit -m "Add help command to reception"
git commit -m "Improve kitchen status response"
git commit -m "Cap ingredient stock regeneration"
git commit -m "Handle kitchen IPC errors"
git commit -m "Refactor kitchen process cleanup"
```

Each commit should describe one logical change.

---

## 15. Final checklist before submission

Before submitting, test:

```bash
make re
./plazza 0.1 2 1000
./plazza 1 2 2000
./plazza 2 5 2000
```

Test valid orders:

```text
margarita S x1
regina XXL x2
regina XXL x2; fantasia M x3; americana L x5
```

Test invalid orders:

```text
regina XXL x0
regina XXL x01
regina
regina XXXL x1
regina S x1 extra
```

Test runtime behavior:

```text
status
many pizzas
wait 5 seconds
status again
exit
```

Expected behavior:

```text
- pizzas are distributed across kitchens
- new kitchens are created when full
- cooks work in threads
- status displays kitchen information
- kitchens close after inactivity
- reception does not freeze while pizzas cook
- no zombie processes remain after exit
```

```
```
