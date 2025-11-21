#include <neko/event/event.hpp>
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "Testing vcpkg installation of NekoEvent..." << std::endl;
    
    // Create an event loop
    neko::event::EventLoop eventLoop;
    
    bool callback_executed = false;
    
    // Subscribe to int events
    eventLoop.subscribe<int>([&callback_executed](const int& value) {
        std::cout << "Event triggered with value: " << value << std::endl;
        callback_executed = true;
    });
    
    // Publish an event
    eventLoop.publish(42);
    
    // Start event loop in a separate thread
    std::thread loopThread([&eventLoop]() {
        eventLoop.run();
    });
    
    // Wait a bit for the event to be processed
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Stop the event loop
    eventLoop.stopLoop();
    loopThread.join();
    
    if (callback_executed) {
        std::cout << "✓ vcpkg installation test passed!" << std::endl;
        return 0;
    } else {
        std::cerr << "✗ vcpkg installation test failed!" << std::endl;
        return 1;
    }
}
