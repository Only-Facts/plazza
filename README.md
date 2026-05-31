# Plazza

> **Who said anything about pizzas?**

Plazza is a C++ simulation of a pizzeria built around **processes**, **threads**, **IPC**, and **load balancing**.

The reception receives pizza orders from an interactive shell, creates kitchen child processes when needed, sends pizzas through pipes, and receives completion/status messages asynchronously.

---

## Project goal

The goal of the project is to model a pizzeria composed of:

- a **reception** that reads user commands;
- several **kitchen processes** created with `fork()`;
- several **cook threads** inside each kitchen;
- a shared kitchen **stock** that regenerates over time;
- an **IPC protocol** used to communicate between reception and kitchens.

The project focuses on:

```text
Processes
IPC
Thread pools
Mutexes
Condition variables
Load balancing
Serialization
Process cleanup
````

---

## Build

```bash
make
```

Clean object files:

```bash
make clean
```

Clean everything:

```bash
make fclean
```

Rebuild:

```bash
make re
```

---

## Run

```bash
./plazza <multiplier> <cooks_per_kitchen> <restock_time_ms>
```

Example:

```bash
./plazza 0.5 2 2000
```

Arguments:

| Argument            | Meaning                                                                          |
| ------------------- | -------------------------------------------------------------------------------- |
| `multiplier`        | Multiplies pizza cooking time. Supports values between `0` and `1`.              |
| `cooks_per_kitchen` | Number of cook threads created in every kitchen process.                         |
| `restock_time_ms`   | Time in milliseconds before each kitchen regenerates 1 unit of every ingredient. |

---

## Shell commands

Once started, Plazza opens an interactive reception shell.

### Order pizzas

```text
regina XXL x2
```

Multiple orders can be chained with `;`:

```text
regina XXL x2; fantasia M x3; margarita S x1
```

### Display status

```text
status
```

This asks every active kitchen process for its current state through IPC.

### Exit

```text
exit
```

This shuts down the reception and active kitchen processes.

---

## Supported pizzas

| Pizza     | Ingredients                                      | Cooking time      |
| --------- | ------------------------------------------------ | ----------------- |
| Margarita | dough, tomato, gruyere                           | `1s * multiplier` |
| Regina    | dough, tomato, gruyere, ham, mushrooms           | `2s * multiplier` |
| Americana | dough, tomato, gruyere, steak                    | `2s * multiplier` |
| Fantasia  | dough, tomato, eggplant, goat cheese, chief love | `4s * multiplier` |

Supported sizes:

```text
S M L XL XXL
```

---

## Order grammar

The parser follows the expected grammar:

```text
S      := TYPE SIZE NUMBER [; TYPE SIZE NUMBER]*
TYPE   := [a..zA..Z]+
SIZE   := S|M|L|XL|XXL
NUMBER := x[1..9][0..9]*
```

Valid examples:

```text
margarita S x1
regina XXL x2; americana L x4
fantasia M x10
```

Rejected examples:

```text
regina XXL x0
regina XXL x01
regina
regina XXXL x1
regina S x1 extra
; regina S x1
regina S x1;
```

---

## Architecture

Current runtime architecture:

```text
Reception process
|
|-- KitchenManager
|   |
|   |-- KitchenProcess #1
|   |   |-- pid
|   |   |-- pipe to kitchen
|   |   |-- pipe from kitchen
|   |   `-- estimated load
|   |
|   `-- KitchenProcess #2
|       `-- ...
|
`-- poll() loop
    |-- reads stdin without blocking kitchen updates
    `-- receives PIZZA_DONE / STATUS_RESPONSE / KITCHEN_CLOSING

Kitchen child process
|
|-- Kitchen
|   |-- ThreadPool
|   |   |-- Cook thread 1
|   |   |-- Cook thread 2
|   |   `-- ...
|   |-- Stock
|   `-- Stock regeneration thread
|
`-- IPC loop
    |-- receives NEW_PIZZA
    |-- receives STATUS_REQUEST
    `-- auto-closes after 5 seconds idle
```

---

## IPC protocol

Reception and kitchens communicate using newline-delimited messages over pipes.

Messages are packed/unpacked through the `Message` class.

Examples:

```text
NEW_PIZZA|PIZZA|2|1
PIZZA_DONE|PIZZA|2|1
STATUS_REQUEST
STATUS_RESPONSE|TEXT|...
KITCHEN_CLOSING
```

Message types:

| Message           | Direction            | Purpose                                 |
| ----------------- | -------------------- | --------------------------------------- |
| `NEW_PIZZA`       | Reception -> Kitchen | Sends a pizza to cook.                  |
| `PIZZA_DONE`      | Kitchen -> Reception | Notifies that a pizza is ready.         |
| `STATUS_REQUEST`  | Reception -> Kitchen | Asks a kitchen for its current state.   |
| `STATUS_RESPONSE` | Kitchen -> Reception | Sends stock/load/cook information.      |
| `KITCHEN_CLOSING` | Both                 | Requests or announces kitchen shutdown. |

---

## Load balancing

Each kitchen can accept at most:

```text
2 * cooks_per_kitchen
```

pizzas at the same time, including pizzas waiting and pizzas currently cooking.

When a new pizza arrives, the reception:

```text
1. Finds the active kitchen with the lowest load.
2. Sends the pizza to it if it has capacity.
3. Creates a new kitchen process if every kitchen is full.
```

---

## Threading model

Each kitchen owns a local `ThreadPool`.

The thread pool contains:

* one worker thread per cook;
* a queue of pizza tasks;
* a mutex protecting the queue;
* a condition variable to wake cooks when pizzas arrive;
* a completion callback that sends `PIZZA_DONE` back to the reception.

Cooks sleep while there is no work and only cook one pizza at a time.

---

## Stock system

Every kitchen starts with 5 units of every ingredient:

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

The stock regenerates 1 unit of each ingredient every `restock_time_ms` milliseconds.

Stock access is thread-safe because cook threads, status requests, and the regeneration thread can touch it at the same time.

---

## Logs

Runtime events are written to:

```text
logs/plazza.log
```

Logged events include:

```text
Plazza started/stopped
Kitchen creation
Pizza sent to kitchen
Pizza ready
Kitchen inactivity shutdown
Order errors
IPC errors
```

---

## Project tree

```text
.
|-- Makefile
|-- README.md
|-- include/
|   |-- Kitchen.hpp
|   |-- KitchenManager.hpp
|   |-- KitchenProcess.hpp
|   |-- Logger.hpp
|   |-- Message.hpp
|   |-- Parser.hpp
|   |-- PipeIPC.hpp
|   |-- Pizza.hpp
|   |-- Reception.hpp
|   |-- Stock.hpp
|   `-- ThreadPool.hpp
`-- src/
    |-- Kitchen.cpp
    |-- KitchenManager.cpp
    |-- KitchenProcess.cpp
    |-- Logger.cpp
    |-- Message.cpp
    |-- Parser.cpp
    |-- PipeIPC.cpp
    |-- Pizza.cpp
    |-- Reception.cpp
    |-- Stock.cpp
    |-- ThreadPool.cpp
    `-- main.cpp
```

---

## Quick test

```bash
make re
./plazza 0.1 2 2000
```

Then in the shell:

```text
margarita S x6
status
```

Expected behavior:

* kitchens are created when capacity is reached;
* pizzas are cooked by cook threads;
* ready pizzas are printed asynchronously;
* `status` displays active kitchen stocks and loads;
* idle kitchens close after 5 seconds.

---

## Implemented features

```text
[x] Interactive reception shell
[x] Hardened order parser
[x] Pizza model and cooking times
[x] Ingredient stock
[x] Thread-safe stock
[x] Stock regeneration
[x] ThreadPool cooks
[x] Forked kitchen processes
[x] Pipe IPC wrapper
[x] Message serialization
[x] Non-blocking reception loop using poll()
[x] Async PIZZA_DONE reception
[x] Status through IPC
[x] Load balancing
[x] Kitchen auto-close after 5 seconds idle
[x] waitpid cleanup
[x] Log file
```

---

## Notes

* `SIGPIPE` is ignored in `main.cpp` so the reception does not crash when a kitchen pipe closes unexpectedly.
* Kitchen process load is tracked from the reception side using sent/done messages.
* Status text is escaped in serialized messages so multiline kitchen status can safely travel through the pipe protocol.
