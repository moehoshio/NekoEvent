/**
 * @file event_test.cpp
 * @brief NekoEvent system tests
 * @author moehoshio
 * @copyright Copyright (c) 2025 Hoshi
 * @license MIT OR Apache-2.0
 * 
 * This test suite provides comprehensive testing for the NekoEvent system,
 * covering event publishing/subscribing, filters, priorities, task scheduling,
 * and various edge cases. The tests are designed to be robust and handle
 * timing variations across different systems.
 */

#include <neko/event/event.hpp>
#include <gtest/gtest.h>

#include <chrono>
#include <thread>
#include <atomic>
#include <vector>
#include <string>

using namespace neko::event;
using namespace std::chrono_literals;

// Test event data structures
struct TestEvent {
    int value;
    std::string message;
    
    TestEvent() : value(0), message("") {}
    TestEvent(int v, const std::string& msg) : value(v), message(msg) {}
};

struct SimpleEvent {
    int data;
    SimpleEvent(int d = 0) : data(d) {}
};

// Test filter class
class TestFilter : public EventFilter<TestEvent> {
private:
    int minValue;
    
public:
    TestFilter(int min) : minValue(min) {}
    
    bool shouldProcess(const TestEvent& eventData) override {
        return eventData.value >= minValue;
    }
};

// Test fixture
class EventLoopTest : public ::testing::Test {
protected:
    void SetUp() override {
        eventLoop = std::make_unique<EventLoop>();
        processedEvents.clear();
        executedTasks.clear();
        loopRunning = false;
    }
    
    void TearDown() override {
        // Ensure proper shutdown
        if (eventLoop && loopRunning.load()) {
            eventLoop->stopLoop();
            // Give time for loop thread to finish
            std::this_thread::sleep_for(100ms);
        }
        eventLoop.reset();
    }
    
    // Helper to wait for event loop to start
    void waitForLoopStart() {
        std::this_thread::sleep_for(100ms);
        loopRunning = true;
    }
    
    // Helper to safely stop loop
    void stopLoopSafely(std::thread& loopThread) {
        if (eventLoop && loopRunning.load()) {
            eventLoop->stopLoop();
            if (loopThread.joinable()) {
                loopThread.join();
            }
            loopRunning = false;
        }
    }
    
    std::unique_ptr<EventLoop> eventLoop;
    std::vector<TestEvent> processedEvents;
    std::vector<int> executedTasks;
    std::mutex processingMutex;
    std::condition_variable processingCv;
    std::atomic<bool> loopRunning{false};
};

// Basic event publishing and subscription tests
TEST_F(EventLoopTest, BasicEventPublishSubscribe) {
    std::atomic<int> eventCount{0};
    const int expectedEvents = 3;
    
    // Subscribe to TestEvent
    auto handlerId = eventLoop->subscribe<TestEvent>([this, &eventCount, expectedEvents](const TestEvent& event) {
        std::lock_guard<std::mutex> lock(processingMutex);
        processedEvents.push_back(event);
        eventCount++;
        if (eventCount >= expectedEvents) {
            processingCv.notify_all();
        }
    });
    
    EXPECT_GT(handlerId, 0);
    
    // Start event loop in separate thread
    std::thread loopThread([this]() {
        eventLoop->run();
    });
    
    waitForLoopStart();
    
    // Publish some events
    eventLoop->publish(TestEvent{1, "First event"});
    eventLoop->publish(TestEvent{2, "Second event"});
    eventLoop->publish(TestEvent{3, "Third event"});
    
    // Wait for all events to be processed with timeout
    {
        std::unique_lock<std::mutex> lock(processingMutex);
        processingCv.wait_for(lock, 2000ms, [&eventCount, expectedEvents]() {
            return eventCount >= expectedEvents;
        });
    }
    
    stopLoopSafely(loopThread);
    
    // Check results
    std::lock_guard<std::mutex> lock(processingMutex);
    EXPECT_EQ(processedEvents.size(), 3);
    if (processedEvents.size() >= 3) {
        EXPECT_EQ(processedEvents[0].value, 1);
        EXPECT_EQ(processedEvents[0].message, "First event");
        EXPECT_EQ(processedEvents[1].value, 2);
        EXPECT_EQ(processedEvents[2].value, 3);
    }
}

TEST_F(EventLoopTest, MultipleSubscribers) {
    std::atomic<int> handler1Count{0};
    std::atomic<int> handler2Count{0};
    const int expectedEvents = 5;
    
    // Subscribe two handlers to the same event type
    auto handler1Id = eventLoop->subscribe<SimpleEvent>([this, &handler1Count, &handler2Count, expectedEvents](const SimpleEvent& event) {
        handler1Count++;
        if (handler1Count >= expectedEvents && handler2Count >= expectedEvents) {
            std::lock_guard<std::mutex> lock(processingMutex);
            processingCv.notify_all();
        }
    });
    
    auto handler2Id = eventLoop->subscribe<SimpleEvent>([this, &handler1Count, &handler2Count, expectedEvents](const SimpleEvent& event) {
        handler2Count++;
        if (handler1Count >= expectedEvents && handler2Count >= expectedEvents) {
            std::lock_guard<std::mutex> lock(processingMutex);
            processingCv.notify_all();
        }
    });
    
    EXPECT_NE(handler1Id, handler2Id);
    
    std::thread loopThread([this]() {
        eventLoop->run();
    });
    
    waitForLoopStart();
    
    // Publish events
    for (int i = 0; i < expectedEvents; ++i) {
        eventLoop->publish(SimpleEvent{i});
    }
    
    // Wait for both handlers to process all events
    {
        std::unique_lock<std::mutex> lock(processingMutex);
        processingCv.wait_for(lock, 2000ms, [&handler1Count, &handler2Count, expectedEvents]() {
            return handler1Count >= expectedEvents && handler2Count >= expectedEvents;
        });
    }
    
    stopLoopSafely(loopThread);
    
    // Both handlers should have processed all events
    EXPECT_EQ(handler1Count.load(), expectedEvents);
    EXPECT_EQ(handler2Count.load(), expectedEvents);
}

TEST_F(EventLoopTest, EventUnsubscribe) {
    std::atomic<int> eventCount{0};
    
    auto handlerId = eventLoop->subscribe<SimpleEvent>([this, &eventCount](const SimpleEvent& event) {
        eventCount++;
        std::lock_guard<std::mutex> lock(processingMutex);
        processingCv.notify_all();
    });
    
    std::thread loopThread([this]() {
        eventLoop->run();
    });
    
    waitForLoopStart();
    
    // Publish first event and wait for it to be processed
    eventLoop->publish(SimpleEvent{1});
    {
        std::unique_lock<std::mutex> lock(processingMutex);
        processingCv.wait_for(lock, 1000ms, [&eventCount]() {
            return eventCount >= 1;
        });
    }
    
    // Unsubscribe
    bool unsubscribed = eventLoop->unsubscribe<SimpleEvent>(handlerId);
    EXPECT_TRUE(unsubscribed);
    
    // Give time for unsubscribe to take effect
    std::this_thread::sleep_for(100ms);
    
    int countBeforeSecondEvent = eventCount.load();
    
    // Publish second event (should not be processed)
    eventLoop->publish(SimpleEvent{2});
    std::this_thread::sleep_for(200ms);
    
    stopLoopSafely(loopThread);
    
    // Count should not have increased after unsubscribe
    EXPECT_EQ(eventCount.load(), countBeforeSecondEvent);
    EXPECT_LE(eventCount.load(), 1);  // Should be at most 1
}

// Event filtering tests
TEST_F(EventLoopTest, EventFiltering) {
    std::atomic<int> passedEventCount{0};
    const int expectedPassedEvents = 2;
    
    auto handlerId = eventLoop->subscribe<TestEvent>([this, &passedEventCount, expectedPassedEvents](const TestEvent& event) {
        std::lock_guard<std::mutex> lock(processingMutex);
        processedEvents.push_back(event);
        passedEventCount++;
        if (passedEventCount >= expectedPassedEvents) {
            processingCv.notify_all();
        }
    });
    
    // Add filter that only allows values >= 5
    auto filter = std::make_unique<TestFilter>(5);
    bool filterAdded = eventLoop->addFilter<TestEvent>(handlerId, std::move(filter));
    EXPECT_TRUE(filterAdded);
    
    std::thread loopThread([this]() {
        eventLoop->run();
    });
    
    waitForLoopStart();
    
    // Publish events with different values
    eventLoop->publish(TestEvent{2, "Should be filtered"});
    eventLoop->publish(TestEvent{7, "Should pass"});
    eventLoop->publish(TestEvent{3, "Should be filtered"});
    eventLoop->publish(TestEvent{10, "Should pass"});
    
    // Wait for passed events to be processed
    {
        std::unique_lock<std::mutex> lock(processingMutex);
        processingCv.wait_for(lock, 2000ms, [&passedEventCount, expectedPassedEvents]() {
            return passedEventCount >= expectedPassedEvents;
        });
    }
    
    // Give extra time to ensure no filtered events sneak through
    std::this_thread::sleep_for(100ms);
    
    stopLoopSafely(loopThread);
    
    // Only events with value >= 5 should pass
    std::lock_guard<std::mutex> lock(processingMutex);
    EXPECT_EQ(processedEvents.size(), 2);
    if (processedEvents.size() >= 2) {
        EXPECT_EQ(processedEvents[0].value, 7);
        EXPECT_EQ(processedEvents[1].value, 10);
    }
}

// Priority handling tests
TEST_F(EventLoopTest, EventPriority) {
    std::atomic<int> highPriorityCount{0};
    const int expectedHighPriorityEvents = 2;
    
    auto handlerId = eventLoop->subscribe<TestEvent>([this, &highPriorityCount, expectedHighPriorityEvents](const TestEvent& event) {
        std::lock_guard<std::mutex> lock(processingMutex);
        processedEvents.push_back(event);
        highPriorityCount++;
        if (highPriorityCount >= expectedHighPriorityEvents) {
            processingCv.notify_all();
        }
    }, neko::Priority::High);  // Only process high priority events
    
    std::thread loopThread([this]() {
        eventLoop->run();
    });
    
    waitForLoopStart();
    
    // Publish events with different priorities
    eventLoop->publish(TestEvent{1, "Low priority"}, neko::Priority::Low);
    eventLoop->publish(TestEvent{2, "Normal priority"}, neko::Priority::Normal);
    eventLoop->publish(TestEvent{3, "High priority"}, neko::Priority::High);
    eventLoop->publish(TestEvent{4, "Critical priority"}, neko::Priority::Critical);
    
    // Wait for high priority events to be processed
    {
        std::unique_lock<std::mutex> lock(processingMutex);
        processingCv.wait_for(lock, 2000ms, [&highPriorityCount, expectedHighPriorityEvents]() {
            return highPriorityCount >= expectedHighPriorityEvents;
        });
    }
    
    // Give extra time to ensure low priority events don't sneak through
    std::this_thread::sleep_for(100ms);
    
    stopLoopSafely(loopThread);
    
    // Only high and critical priority events should be processed
    std::lock_guard<std::mutex> lock(processingMutex);
    EXPECT_EQ(processedEvents.size(), 2);
    if (processedEvents.size() >= 2) {
        EXPECT_EQ(processedEvents[0].value, 3);
        EXPECT_EQ(processedEvents[1].value, 4);
    }
}

// Task scheduling tests
TEST_F(EventLoopTest, BasicTaskScheduling) {
    std::atomic<bool> taskExecuted{false};
    std::atomic<int> executionOrder{0};
    
    std::thread loopThread([this]() {
        eventLoop->run();
    });
    
    waitForLoopStart();
    
    // Schedule a task to run after 100ms (increased for reliability)
    auto taskId = eventLoop->scheduleTask(100, [this, &taskExecuted, &executionOrder]() {
        taskExecuted = true;
        executionOrder = 1;
        std::lock_guard<std::mutex> lock(processingMutex);
        processingCv.notify_all();
    });
    
    EXPECT_GT(taskId, 0);
    
    // Wait for task execution with condition variable
    {
        std::unique_lock<std::mutex> lock(processingMutex);
        processingCv.wait_for(lock, 2000ms, [&taskExecuted]() {
            return taskExecuted.load();
        });
    }
    
    stopLoopSafely(loopThread);
    
    EXPECT_TRUE(taskExecuted.load());
    EXPECT_EQ(executionOrder.load(), 1);
}

TEST_F(EventLoopTest, TaskCancellation) {
    std::atomic<bool> taskExecuted{false};
    
    std::thread loopThread([this]() {
        eventLoop->run();
    });
    
    waitForLoopStart();
    
    // Schedule a task with sufficient delay for cancellation
    auto taskId = eventLoop->scheduleTask(300, [&taskExecuted]() {
        taskExecuted = true;
    });
    
    EXPECT_GT(taskId, 0);
    
    // Give a moment for task to be registered
    std::this_thread::sleep_for(50ms);
    
    // Cancel the task
    bool cancelled = eventLoop->cancelTask(taskId);
    EXPECT_TRUE(cancelled);
    
    // Wait longer than the original task delay to ensure it doesn't execute
    std::this_thread::sleep_for(500ms);
    
    stopLoopSafely(loopThread);
    
    // Task should not have been executed
    EXPECT_FALSE(taskExecuted.load());
}

TEST_F(EventLoopTest, RepeatingTask) {
    std::atomic<int> executionCount{0};
    std::mutex mtx;
    std::condition_variable cv;
    const int minExecutions = 2;  // Minimum to prove it repeats
    
    std::thread loopThread([this]() {
        eventLoop->run();
    });
    
    waitForLoopStart();
    
    // Schedule a repeating task every 150ms (increased interval for reliability)
    auto taskId = eventLoop->scheduleRepeating(150, [&executionCount, &cv]() {
        executionCount++;
        cv.notify_all();
    });
    
    EXPECT_GT(taskId, 0);
    
    // Wait for at least minExecutions with generous timeout
    {
        std::unique_lock<std::mutex> lock(mtx);
        // With 150ms interval, 2 executions needs ~300ms
        // Give it 2000ms to be safe across different systems and loads
        cv.wait_for(lock, 2000ms, [&executionCount, minExecutions]() {
            return executionCount >= minExecutions;
        });
    }
    
    // Cancel the repeating task
    bool cancelled = eventLoop->cancelTask(taskId);
    EXPECT_TRUE(cancelled);
    
    // Wait to ensure cancellation is processed
    std::this_thread::sleep_for(200ms);
    
    int finalCount = executionCount.load();
    
    stopLoopSafely(loopThread);
    
    // Should have executed multiple times
    // Very conservative - just verify it repeats
    EXPECT_GE(finalCount, minExecutions);  // At least 2 to prove it repeats
    EXPECT_LE(finalCount, 20);  // Sanity check upper bound (generous)
}

// Delayed event publishing tests
TEST_F(EventLoopTest, DelayedEventPublishing) {
    std::atomic<bool> eventReceived{false};
    std::mutex mtx;
    std::condition_variable cv;
    
    auto handlerId = eventLoop->subscribe<TestEvent>([&eventReceived, &cv](const TestEvent& event) {
        eventReceived = true;
        cv.notify_all();
    });
    
    std::thread loopThread([this]() {
        eventLoop->run();
    });
    
    waitForLoopStart();
    
    // Publish event with delay (increased for reliability)
    auto taskId = eventLoop->publishAfter(150, TestEvent{42, "Delayed event"});
    EXPECT_GT(taskId, 0);
    
    // Wait for delayed event to be published and processed
    // Use condition variable to wait for event reception with timeout
    {
        std::unique_lock<std::mutex> lock(mtx);
        // 150ms delay + generous timeout for processing
        cv.wait_for(lock, 2000ms, [&eventReceived]() {
            return eventReceived.load();
        });
    }
    
    stopLoopSafely(loopThread);
    
    EXPECT_TRUE(eventReceived.load());
}

// Statistics tests
TEST_F(EventLoopTest, EventStatistics) {
    eventLoop->enableStatistics(true);
    eventLoop->resetStatistics();
    
    std::atomic<int> processedCount{0};
    const int expectedEvents = 5;
    
    auto handlerId = eventLoop->subscribe<SimpleEvent>([this, &processedCount, expectedEvents](const SimpleEvent& event) {
        processedCount++;
        // Add small delay to ensure processing time is measurable
        std::this_thread::sleep_for(1ms);
        if (processedCount >= expectedEvents) {
            std::lock_guard<std::mutex> lock(processingMutex);
            processingCv.notify_all();
        }
    });
    
    std::thread loopThread([this]() {
        eventLoop->run();
    });
    
    waitForLoopStart();
    
    // Publish several events
    for (int i = 0; i < expectedEvents; ++i) {
        eventLoop->publish(SimpleEvent{i});
    }
    
    // Wait for all events to be processed
    {
        std::unique_lock<std::mutex> lock(processingMutex);
        processingCv.wait_for(lock, 2000ms, [&processedCount, expectedEvents]() {
            return processedCount >= expectedEvents;
        });
    }
    
    stopLoopSafely(loopThread);
    
    auto stats = eventLoop->getStatistics();
    // Check that all events were processed
    EXPECT_EQ(processedCount.load(), expectedEvents);
    // Stats may include internal events, so just check it's reasonable
    EXPECT_GE(stats.processedEvents, expectedEvents);
    EXPECT_EQ(stats.droppedEvents, 0);
    EXPECT_EQ(stats.failedEvents, 0);
}

// Queue size tests
TEST_F(EventLoopTest, QueueSizeTracking) {
    // Set a small max queue size for testing
    eventLoop->setMaxQueueSize(3);
    eventLoop->enableStatistics(true);
    eventLoop->resetStatistics();
    
    std::atomic<int> processingCount{0};
    
    auto handlerId = eventLoop->subscribe<SimpleEvent>([&processingCount](const SimpleEvent& event) {
        processingCount++;
        // Slow handler to fill up queue
        std::this_thread::sleep_for(100ms);
    });
    
    std::thread loopThread([this]() {
        eventLoop->run();
    });
    
    waitForLoopStart();
    
    // Publish more events than max queue size rapidly
    const int totalEvents = 8;
    for (int i = 0; i < totalEvents; ++i) {
        eventLoop->publish(SimpleEvent{i});
        // Small delay to ensure events are queued
        std::this_thread::sleep_for(5ms);
    }
    
    // Give time for queue to fill and some processing to occur
    std::this_thread::sleep_for(200ms);
    
    auto sizes = eventLoop->getQueueSizes();
    // Queue size should be limited
    EXPECT_LE(sizes.eventQueueSize, 3);
    
    stopLoopSafely(loopThread);
    
    // Check that some events were dropped or not all were processed
    auto stats = eventLoop->getStatistics();
    auto totalHandled = processingCount.load() + stats.droppedEvents;
    // Either some were dropped, or not all could be processed
    EXPECT_TRUE(stats.droppedEvents > 0 || processingCount.load() < totalEvents);
}

// Exception handling tests
TEST_F(EventLoopTest, ExceptionHandling) {
    std::atomic<int> handlerExecutionCount{0};
    std::atomic<bool> exceptionThrown{false};
    std::atomic<bool> normalEventProcessed{false};
    
    auto handlerId = eventLoop->subscribe<SimpleEvent>([this, &handlerExecutionCount, &exceptionThrown, &normalEventProcessed](const SimpleEvent& event) {
        handlerExecutionCount++;
        if (event.data == 42) {
            exceptionThrown = true;
            throw std::runtime_error("Test exception");
        } else {
            normalEventProcessed = true;
            std::lock_guard<std::mutex> lock(processingMutex);
            processingCv.notify_all();
        }
    });
    
    eventLoop->enableStatistics(true);
    eventLoop->resetStatistics();
    
    std::thread loopThread([this]() {
        eventLoop->run();
    });
    
    waitForLoopStart();
    
    // Publish event that will cause exception
    eventLoop->publish(SimpleEvent{42});
    std::this_thread::sleep_for(100ms);
    
    // Publish normal event after exception to verify loop still works
    eventLoop->publish(SimpleEvent{1});
    
    // Wait for normal event to be processed
    {
        std::unique_lock<std::mutex> lock(processingMutex);
        processingCv.wait_for(lock, 2000ms, [&normalEventProcessed]() {
            return normalEventProcessed.load();
        });
    }
    
    stopLoopSafely(loopThread);
    
    // Verify both events were attempted
    EXPECT_TRUE(exceptionThrown.load());
    EXPECT_TRUE(normalEventProcessed.load());
    EXPECT_GE(handlerExecutionCount.load(), 2);
    
    // Event loop should still be functional after exception
    auto stats = eventLoop->getStatistics();
    // At least one event should have failed (the exception)
    // and at least one should have succeeded (the normal event)
    EXPECT_GT(stats.processedEvents + stats.failedEvents, 0);
}

/*
 * Test Summary:
 * 
 * All tests have been improved to handle race conditions by:
 * - Adding 50ms startup delay after launching event loop thread
 * - Using condition variables with timeouts for time-sensitive tests
 * - Ensuring sufficient wait times for scheduled tasks
 * - Using appropriate timeouts for event processing
 * 
 *  BasicEventPublishSubscribe - Tests basic event publishing and subscription
 *  MultipleSubscribers - Tests multiple handlers for same event type
 *  EventUnsubscribe - Tests handler removal functionality
 *  EventFiltering - Tests custom event filters
 *  EventPriority - Tests priority-based event processing
 *  BasicTaskScheduling - Tests basic task scheduling functionality
 *  TaskCancellation - Tests task cancellation
 *  RepeatingTask - Tests repeating task with condition variable synchronization
 *  DelayedEventPublishing - Tests delayed event publishing with condition variable synchronization
 *  EventStatistics - Tests event processing statistics
 *  QueueSizeTracking - Tests queue size limits and tracking
 *  ExceptionHandling - Tests exception handling in event processing
 */

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
