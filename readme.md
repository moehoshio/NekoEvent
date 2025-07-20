# Neko event.hpp

This project is a standalone C++ event system module, extracted from the [NekoLc](https://github.com/moehoshio/NekoLauncher) project.

[![License](https://img.shields.io/badge/License-MIT%20OR%20Apache--2.0-blue.svg)](./LICENSE)

## Introduction

`neko-event.hpp` provides a modern, type-safe, and high-performance event handling system for C++. It supports synchronous/asynchronous events, event filtering, priority levels, scheduling (delayed and repeating tasks).  
is suitable for game engines, application frameworks, or any C++ project requiring event-driven architecture.  

It is easy to use—just include a single header file, and it provides a simple and intuitive API that allows developers to easily create and manage events.

## Features

- Cross-platform
- Header-only
- Type-safe event data of any type
- Event priority and sync/async processing modes
- Extensible event filters and handlers
- Built-in task scheduling (delayed and repeating)
- Thread-safe
- Event statistics

## Integration

1. Clone this repository to your local machine:

```sh
git clone https://github.com/moehoshio/event.hpp.git && cd event.hpp
```

2. Copy the contents of the `event.hpp/include` folder into your project's `include` directory.
3. Add the following include directive in your source file:

```cpp
#include "neko/event/event.hpp"
```

If you prefer a single-header usage, you can use the [Release](https://github.com/moehoshio/event.hpp/releases/) version of `neko-event.hpp`, which makes integration into your project more convenient. However, note that it is only updated with each release and does not include the latest fixes and features.

## Basic Usage

```cpp
#include "neko/event/event.hpp"
#include <iostream>

struct HelloWorldEvent {};

int main() {
    neko::event::EventLoop loop;

    // Subscribe to HelloWorldEvent events
    auto handlerId = loop.subscribe<>([](const HelloWorldEvent &event) {
        std::cout << "Received HelloWorldEvent" << std::endl;
    });

    // Publish a HelloWorldEvent
    loop.publish(HelloWorldEvent{});

    // Run the event loop
    loop.run();
}
```

## More Usage Examples

### 1. Custom Event Types

```cpp
struct MyEvent {
    int id;
    std::string message;
};

loop.subscribe<MyEvent>([](const MyEvent &evt) {
    std::cout << "MyEvent: id=" << evt.id << ", message=" << evt.message << std::endl;
});

loop.publish(MyEvent{1, "Hello Event System"});
```

### 2. Event Priority and Sync Mode

```cpp
loop.publish<int>(100, neko::Priority::High, neko::SyncMode::Sync);
```

### 3. Delayed and Repeating Events

```cpp
// Publish an event after 1000ms
loop.publishAfter(1000, std::string("Delayed event"));

// Schedule a repeating task every 500ms
loop.scheduleRepeating(500, []() {
    std::cout << "Repeating task triggered!" << std::endl;
});
```

### 4. Event Filters

```cpp
// Define a filter that only allows even numbers
class EvenFilter : public neko::event::EventFilter<int> {
public:
    bool shouldProcess(const int &value) override {
        return value % 2 == 0;
    }
};

auto handlerId = loop.subscribe<int>([](const int &v) {
    std::cout << "Even int: " << v << std::endl;
});
loop.addFilter<int>(handlerId, std::make_unique<EvenFilter>());

loop.publish(1); // Filtered out
loop.publish(2); // Will be processed
```

### 5. Cancelling Scheduled Tasks

```cpp
auto taskId = loop.scheduleRepeating(1000, []() {
    std::cout << "This will repeat every second." << std::endl;
});

// Cancel after some condition
loop.cancelTask(taskId);
```

### 6. Event Statistics

```cpp
// Get statistics about event processing
auto stats = loop.getEventStatistics();
std::cout << "Total events processed: " << stats.totalEvents << std::endl;
std::cout << "Event max processing time: " << stats.maxProcessingTime << "ms" << std::endl;
```

## License

[License](./LICENSE) MIT OR Apache-2.0
