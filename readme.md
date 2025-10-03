# NekoEvent

This is a modern, type-safe, and high-performance event handling system for C++. It supports synchronous/asynchronous events, event filtering, priority levels, scheduling (delayed and repeating tasks).  
It is suitable for game engines, application frameworks, or any C++ project requiring event-driven architecture. For example, you can use events to decouple business logic modules from UI modules, enabling flexible and maintainable interactions between different parts of your application.  

It is easy to use - a simple and intuitive API that allows developers to easily create and manage events.

[![License](https://img.shields.io/badge/License-MIT%20OR%20Apache--2.0-blue.svg)](LICENSE)

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
#include <neko/event/event.hpp>
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

## Tests

You can run the tests to verify that everything is working correctly.

If you haven't configured the build yet, please run:

```shell
cmake -B ./build . -DNEKO_BUILD_TESTS=ON -DNEKO_AUTO_FETCH_DEPS=ON
```

Now, you can build the test files (you must build them manually at least once before running the tests!).

```shell
cmake --build ./build --config Debug
```

Then, you can run the tests with the following commands:

Unix Makefile / Ninja generator：

```shell
cmake --build ./build --target test
```

Visual Studio generator：

```shell
cmake --build ./build --config Debug --target RUN_TESTS
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

### Test Coverage

The test suite covers:

- Basic event publishing and subscription
- Multiple subscribers for same event type
- Event unsubscription
- Event filtering with custom filters
- Event priority handling
- Task scheduling (basic, delayed, repeating)
- Task cancellation
- Delayed event publishing
- Event statistics and queue size tracking
- Exception handling in event handlers

### Disable Tests

If you want to disable building and running tests, you can set the following CMake option when configuring your project:

```shell
cmake -B ./build . -DNEKO_BUILD_TESTS=OFF
```

## License

[License](LICENSE) MIT OR Apache-2.0

## See More

- [NekoLog](https://github.com/moehoshio/nlog): An easy-to-use, modern, lightweight, and efficient C++20 logging library.
- [NekoEvent](https://github.com/moehoshio/NekoEvent): A modern easy to use type-safe and high-performance event handling system for C++.
- [NekoSchema](https://github.com/moehoshio/NekoSchema): A lightweight, header-only C++20 schema library.
- [NekoSystem](https://github.com/moehoshio/NekoSystem): A modern C++20 cross-platform system utility library.
- [NekoFunction](https://github.com/moehoshio/NekoFunction): A comprehensive modern C++ utility library that provides practical functions for common programming tasks.
- [NekoThreadPool](https://github.com/moehoshio/NekoThreadPool): An easy to use and efficient C++ 20 thread pool that supports priorities and submission to specific threads.
