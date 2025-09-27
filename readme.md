# NekoEvent

This is a modern, type-safe, and high-performance event handling system for C++. It supports synchronous/asynchronous events, event filtering, priority levels, scheduling (delayed and repeating tasks).  
It is suitable for game engines, application frameworks, or any C++ project requiring event-driven architecture. For example, you can use events to decouple business logic modules from UI modules, enabling flexible and maintainable interactions between different parts of your application.  

It is easy to use—just include a single header file, and it provides a simple and intuitive API that allows developers to easily create and manage events.

[![License](https://img.shields.io/badge/License-MIT%20OR%20Apache--2.0-blue.svg)](./LICENSE)


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

### CMake

1. Using CMake's `FetchContent` to include NekoEvent in your project:

```cmake
include(FetchContent)

# Add NekoEvent to your CMake project
FetchContent_Declare(
    NekoEvent
    GIT_REPOSITORY https://github.com/moehoshio/NekoEvent.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(NekoEvent)

target_link_libraries(your_target PRIVATE NekoEvent)
```

2. Include the header files in your code

```cpp
#include <neko/event/event.hpp>
```

### Manual

When installing manually, you need to manually fetch the dependency [`NekoSchema`](https://github.com/moehoshio/NekoSchema).

After installing the dependency, please continue:

1. Clone this repository to your local machine:

```sh
git clone https://github.com/moehoshio/NekoEvent.git
```

2. Copy the contents of the `NekoEvent/include` folder into your project's `include` directory.

```shell
cp -r NekoEvent/include/ /path/to/your/include/
```

3. Add the following include directive in your source file:

```cpp
#include "neko/event/event.hpp"
```

## Basic Usage

In the following example, we will create a simple event loop, subscribe to an event, and publish it.

```cpp
#include "neko/event/event.hpp"
#include <iostream>

struct StartEvent {};
struct QuitEvent {};

int main() {
    neko::event::EventLoop loop;

    // Subscribe to StartEvent events
    auto handlerId = loop.subscribe<StartEvent>([](const StartEvent &event) {
        std::cout << "Received StartEvent" << std::endl;
    });

    // Publish a StartEvent
    loop.publish(StartEvent{});

    // Subscribe to QuitEvent events
    // This will stop the event loop when QuitEvent is received
    loop.subscribe<QuitEvent>([&loop](const QuitEvent &event) {
        std::cout << "Received QuitEvent" << std::endl;
        loop.stopLoop(); // Stop the event loop
    });

    // Publish a QuitEvent after 2000ms to stop the loop
    loop.publishAfter(2000, QuitEvent{});

    // Run the event loop
    loop.run();
}
```

result in the following output:

``` sh
Received StartEvent
// After 2000ms
Received QuitEvent
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
