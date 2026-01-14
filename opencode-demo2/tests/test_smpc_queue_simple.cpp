#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include <random>
#include <string>
#include "SPMCQueue.h"

// Simple test data structure
struct TestItem {
    int value;
    
    TestItem() : value(0) {}
    TestItem(int v) : value(v) {}
    
    bool operator==(const TestItem& other) const {
        return value == other.value;
    }
};

class SPMCQueueSimpleTest : public ::testing::Test {
protected:
    void SetUp() override {
        queue_name = "test_spmc_queue_" + std::to_string(test_counter++);
    }
    
    void TearDown() override {
        // Clean up any existing shared memory
        std::string cleanup_cmd = "rm -f /dev/shm/" + queue_name + "* 2>/dev/null";
        system(cleanup_cmd.c_str());
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    static int test_counter;
    std::string queue_name;
};

// Define static member outside class
int SPMCQueueSimpleTest::test_counter = 0;

// Basic queue functionality tests
TEST_F(SPMCQueueSimpleTest, EmptyQueueInitialState) {
    SPMCQueue<int> queue(queue_name);
    
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);
}

TEST_F(SPMCQueueSimpleTest, SinglePushPop) {
    SPMCQueue<int> queue(queue_name);
    
    queue.push(42);
    EXPECT_FALSE(queue.empty());
    EXPECT_EQ(queue.size(), 1);
    
    int value;
    EXPECT_TRUE(queue.try_pop(value));
    EXPECT_EQ(value, 42);
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);
}

TEST_F(SPMCQueueSimpleTest, MultiplePushPop) {
    SPMCQueue<int> queue(queue_name);
    
    const int num_items = 100;
    for (int i = 0; i < num_items; ++i) {
        queue.push(i);
    }
    
    EXPECT_EQ(queue.size(), num_items);
    
    std::vector<int> popped_items;
    int value;
    while (queue.try_pop(value)) {
        popped_items.push_back(value);
    }
    
    EXPECT_EQ(popped_items.size(), num_items);
    for (int i = 0; i < num_items; ++i) {
        EXPECT_EQ(popped_items[i], i);
    }
}

TEST_F(SPMCQueueSimpleTest, EmplaceBackBasic) {
    SPMCQueue<int> queue(queue_name);
    
    queue.emplace_back(123);
    EXPECT_FALSE(queue.empty());
    EXPECT_EQ(queue.size(), 1);
    
    int value;
    EXPECT_TRUE(queue.try_pop(value));
    EXPECT_EQ(value, 123);
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);
}

TEST_F(SPMCQueueSimpleTest, MultiConsumerScenario) {
    SPMCQueue<int> queue(queue_name);
    
    const int num_items = 1000;
    const int num_consumers = 3;
    
    std::atomic<int> total_consumed{0};
    std::atomic<bool> stop_consumers{false};
    std::vector<std::thread> consumers;
    
    // Start consumer threads first
    for (int c = 0; c < num_consumers; ++c) {
        consumers.emplace_back([&queue, &total_consumed, &stop_consumers]() {
            while (!stop_consumers.load(std::memory_order_acquire)) {
                int value;
                if (queue.try_pop(value)) {
                    total_consumed.fetch_add(1, std::memory_order_relaxed);
                } else {
                    std::this_thread::yield();
                }
            }
            
            // One final check after stop signal to drain any remaining items
            int value;
            while (queue.try_pop(value)) {
                total_consumed.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }
    
    // Producer thread
    std::thread producer([&queue]() {
        for (int i = 0; i < num_items; ++i) {
            while (!queue.try_push(i)) {
                std::this_thread::yield();
            }
        }
    });
    
    producer.join();
    
    // Signal consumers to stop
    stop_consumers.store(true, std::memory_order_release);
    
    for (auto& consumer : consumers) {
        consumer.join();
    }
    
    EXPECT_EQ(total_consumed.load(), num_items);
}