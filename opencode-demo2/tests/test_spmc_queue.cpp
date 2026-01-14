#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include <random>
#include <string>
#include "SPMCQueue.h"

// Simple test data structure
struct TestData {
    int value;
    
    TestData() : value(0) {}
    TestData(int v) : value(v) {}
    
    bool operator==(const TestData& other) const {
        return value == other.value;
    }
};

class SPMCQueueTest : public ::testing::Test {
protected:
    void SetUp() override {
        queue_name = "test_spmc_queue_" + std::to_string(test_counter++);
        // Clean up any existing shared memory
        std::string cleanup_cmd = "rm -f /dev/shm/" + queue_name + "* 2>/dev/null";
        system(cleanup_cmd.c_str());
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    void TearDown() override {
        // Clean up shared memory
        std::string cleanup_cmd = "rm -f /dev/shm/" + queue_name + "* 2>/dev/null";
        system(cleanup_cmd.c_str());
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    static int test_counter;
    std::string queue_name;
};

// Define static member outside class
int SPMCQueueTest::test_counter = 0;

// Basic queue operations
TEST_F(SPMCQueueTest, EmptyQueueInitialState) {
    SPMCQueue<int> queue(queue_name);
    
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);
}

TEST_F(SPMCQueueTest, SinglePushPop) {
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

TEST_F(SPMCQueueTest, MultiplePushPop) {
    SPMCQueue<int> queue(queue_name);
    
    const int num_items = 1000;
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

// Emplace functionality tests
TEST_F(SPMCQueueTest, EmplaceBackBasic) {
    SPMCQueue<TestData> queue(queue_name);
    
    queue.emplace_back(123);
    EXPECT_FALSE(queue.empty());
    EXPECT_EQ(queue.size(), 1);
    
    TestData value;
    EXPECT_TRUE(queue.try_pop(value));
    EXPECT_EQ(value.value, 123);
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);
}

TEST_F(SPMCQueueTest, EmplaceBackMultipleArgs) {
    SPMCQueue<TestData> queue(queue_name);
    
    queue.emplace_back(456);
    EXPECT_EQ(queue.size(), 1);
    
    TestData value;
    EXPECT_TRUE(queue.try_pop(value));
    EXPECT_EQ(value.value, 456);
}

// Move semantics tests
TEST_F(SPMCQueueTest, PushMoveSemantics) {
    SPMCQueue<TestData> queue(queue_name);
    
    TestData data(789);
    queue.push(std::move(data));  // Move constructor
    EXPECT_EQ(queue.size(), 1);
    
    TestData value;
    EXPECT_TRUE(queue.try_pop(value));
    EXPECT_EQ(value.value, 789);
}

// Performance tests
TEST_F(SPMCQueueTest, PerformanceBenchmark) {
    SPMCQueue<int> queue(queue_name);
    
    const int num_items = 10000;
    
    // Benchmark push performance
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_items; ++i) {
        while (!queue.try_push(i)) {
            // Brief yield if queue is full
            std::this_thread::yield();
        }
    }
    auto push_time = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now() - start);
    
    // Benchmark pop performance
    start = std::chrono::high_resolution_clock::now();
    int popped = 0;
    int value;
    while (popped < num_items) {
        if (queue.try_pop(value)) {
            popped++;
        }
    }
    auto pop_time = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now() - start);
    
    // Performance expectations
    double push_rate = num_items * 1000000.0 / push_time.count();
    double pop_rate = num_items * 1000000.0 / pop_time.count();
    
    EXPECT_GT(push_rate, 100000.0);  // At least 100K items/sec
    EXPECT_GT(pop_rate, 100000.0);   // At least 100K items/sec
    
    // Report performance
    std::cout << "Push Performance: " << static_cast<int>(push_rate) << " items/sec" << std::endl;
    std::cout << "Pop Performance: " << static_cast<int>(pop_rate) << " items/sec" << std::endl;
}

// Type safety tests
TEST_F(SPMCQueueTest, FixedSizeTypesCompile) {
    // These should compile (fixed-size, trivially copyable types)
    SPMCQueue<int> int_queue(queue_name + "_int");
    SPMCQueue<double> double_queue(queue_name + "_double");
    
    EXPECT_TRUE(int_queue.empty());
    EXPECT_TRUE(double_queue.empty());
}

// Multi-consumer tests
TEST_F(SPMCQueueTest, SingleProducerMultipleConsumers) {
    SPMCQueue<int> queue(queue_name);
    
    const int num_consumers = 4;
    const int items_per_producer = 1000;
    
    std::atomic<int> total_consumed{0};
    std::vector<std::thread> consumers;
    
    // Start consumers first
    for (int c = 0; c < num_consumers; ++c) {
        consumers.emplace_back([&queue, &total_consumed]() {
            int consumed = 0;
            while (consumed < items_per_producer / num_consumers) {
                int value;
                if (queue.try_pop(value)) {
                    consumed++;
                    total_consumed.fetch_add(1, std::memory_order_relaxed);
                } else {
                    std::this_thread::yield();
                }
            }
        });
    }
    
    // Producer
    std::thread producer([&queue]() {
        for (int i = 0; i < items_per_producer; ++i) {
            while (!queue.try_push(i)) {
                std::this_thread::yield();
            }
        }
    });
    
    producer.join();
    for (auto& consumer : consumers) {
        consumer.join();
    }
    
    EXPECT_EQ(total_consumed.load(), items_per_producer);
}

TEST_F(SPMCQueueTest, CapacityManagement) {
    SPMCQueue<int, 64> small_queue(queue_name + "_small"); // Small segment
    
    // Fill the small segment
    const int capacity = 64 / sizeof(int);
    for (int i = 0; i < capacity; ++i) {
        small_queue.push(i);
    }
    
    // Next push should still work (new segment allocation)
    small_queue.push(capacity);
    EXPECT_EQ(small_queue.size(), capacity + 1);
}