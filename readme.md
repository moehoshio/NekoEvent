# NekoEvent

This is a modern, type-safe, and high-performance event handling system for C++. It supports synchronous/asynchronous events, event filtering, priority levels, scheduling (delayed and repeating tasks).  
It is suitable for game engines, application frameworks, or any C++ project requiring event-driven architecture. For example, you can use events to decouple business logic modules from UI modules, enabling flexible and maintainable interactions between different parts of your application.  

It is easy to use - a simple and intuitive API that allows developers to easily create and manage events.

[![License](https://img.shields.io/badge/License-MIT%20OR%20Apache--2.0-blue.svg)](LICENSE)
![Require](https://img.shields.io/badge/%20Require%20-%3E=%20C++%2020-orange.svg)
[![CMake](https://img.shields.io/badge/CMake-3.14+-green.svg)](https://cmake.org/)
![Module Support](https://img.shields.io/badge/Modules-C%2B%2B20-blueviolet.svg)
[![CI Status](https://github.com/moehoshio/NekoEvent/actions/workflows/ci.yml/badge.svg)](https://github.com/moehoshio/NekoEvent/actions/workflows/ci.yml)

## Features

- **Event priority**: Control event processing order
- **Sync/Async processing**: Flexible event handling modes
- **Extensible filters and handlers**: Customize event processing
- **Task scheduling**: Built-in delayed and repeating tasks
- **Header-only**: Easy integration with minimal setup
- **Type-safe**: Leverages C++20 type system
- **Thread-safe**: Safe for concurrent use
- **Event statistics**: Track and monitor event activity
- **C++20 Modules support**: Optional modern C++ module support

## Integration

### Prerequisites

- C++20 or later
- CMake 3.14 or later
- Git

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

target_link_libraries(your_target PRIVATE Neko::Event)
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
#include <neko/event/event.hpp>
```

### C++20 Module Support

NekoEvent supports C++20 modules

#### Building with Module Support

To enable C++20 module support, use the `NEKO_EVENT_ENABLE_MODULE` option:

```cmake
include(FetchContent)

FetchContent_Declare(
    NekoEvent
    GIT_REPOSITORY https://github.com/moehoshio/NekoEvent.git
    GIT_TAG        main
)

# Enable module support
set(NEKO_EVENT_ENABLE_MODULE ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(NekoEvent)

# Link against the module target
add_executable(your_target main.cpp)
target_link_libraries(your_target PRIVATE Neko::Event::Module)
```

#### Using the Module

Instead of including headers, simply import the module:

```cpp
#include <iostream>
import neko.event;

int main() {
    neko::event::EventLoop loop;
    // Your code here
}
```

## Basic Usage

In the following example, we will create a simple event loop, subscribe to an event, and publish it.

```cpp
#include <neko/event/event.hpp>
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

## Test

You can run the tests to verify that everything is working correctly.

If you haven't configured the build yet, please run:

```shell
cmake -B ./build -D NEKO_EVENT_BUILD_TESTS=ON -D NEKO_EVENT_AUTO_FETCH_DEPS=ON -S .
```

Now, you can build the test files with the following command:

```shell
cmake --build ./build --config Debug
```

Then, you can run the tests with the following commands:

```shell
cd ./build && ctest --output-on-failure
```

If everything is set up correctly, you should see output similar to the following:

```shell
...
[ RUN      ] EventLoopTest.DelayedEventPublishing
[       OK ] EventLoopTest.DelayedEventPublishing (1107 ms)
[ RUN      ] EventLoopTest.EventStatistics
[       OK ] EventLoopTest.EventStatistics (207 ms)
[ RUN      ] EventLoopTest.QueueSizeTracking
[       OK ] EventLoopTest.QueueSizeTracking (55 ms)
[ RUN      ] EventLoopTest.ExceptionHandling
[       OK ] EventLoopTest.ExceptionHandling (113 ms)
[----------] 12 tests from EventLoopTest (2571 ms total)

[----------] Global test environment tear-down
[==========] 12 tests from 1 test suite ran. (2572 ms total)
[  PASSED  ] 12 tests.
```

### Disable Tests

If you want to disable building and running tests, you can set the following CMake option when configuring your project:

```shell
cmake -B ./build -DNEKO_EVENT_BUILD_TESTS=OFF -S .
```

This will skip test targets during the build process.

## License

[License](LICENSE) MIT OR Apache-2.0

## See More

- [NekoNet](https://github.com/moehoshio/NekoNet): A modern , easy-to-use C++20 networking library via libcurl.
- [NekoLog](https://github.com/moehoshio/NekoLog): An easy-to-use, modern, lightweight, and efficient C++20 logging library.
- [NekoEvent](https://github.com/moehoshio/NekoEvent): A modern easy to use type-safe and high-performance event handling system for C++.
- [NekoSchema](https://github.com/moehoshio/NekoSchema): A lightweight, header-only C++20 schema library.
- [NekoSystem](https://github.com/moehoshio/NekoSystem): A modern C++20 cross-platform system utility library.
- [NekoFunction](https://github.com/moehoshio/NekoFunction): A comprehensive modern C++ utility library that provides practical functions for common programming tasks.
- [NekoThreadPool](https://github.com/moehoshio/NekoThreadPool): An easy to use and efficient C++ 20 thread pool that supports priorities and submission to specific threads.
