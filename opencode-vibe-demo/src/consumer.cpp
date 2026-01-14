#include <iostream>
#include <chrono>
#include <signal.h>
#include "SPMCQueue.h"

struct IPCMessage {
    uint64_t timestamp;
    uint32_t producer_id;
    uint32_t sequence;
    char payload[64];
    
    IPCMessage() : timestamp(0), producer_id(0), sequence(0) {
        std::memset(payload, 0, sizeof(payload));
    }
    
    bool operator==(const IPCMessage& other) const {
        return timestamp == other.timestamp && 
               producer_id == other.producer_id && 
               sequence == other.sequence &&
               strcmp(payload, other.payload) == 0;
    }
};

volatile sig_atomic_t stop_consumer = 0;

void signal_handler(int /* signal */) {
    stop_consumer = 1;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cout << "Usage: " << argv[0] << " <queue_name> <consumer_id> <duration_ms>" << std::endl;
        std::cout << "Example: " << argv[0] << " ipc_test 1 30000" << std::endl;
        return 1;
    }
    
    std::string queue_name = argv[1];
    int consumer_id = std::stoi(argv[2]);
    int duration_ms = std::stoi(argv[3]);
    
    // Set up signal handler
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    try {
        std::cout << "Consumer " << consumer_id << " starting..." << std::endl;
        
        // Open shared memory queue
        SPMCQueue<IPCMessage> queue(queue_name);
        
        std::cout << "Consumer " << consumer_id << " connected to queue" << std::endl;
        
        auto start_time = std::chrono::high_resolution_clock::now();
        auto last_report = start_time;
        int messages_consumed = 0;
        int last_report_count = 0;
        
        auto end_time = duration_ms > 0 ? 
            start_time + std::chrono::milliseconds(duration_ms) : 
            std::chrono::time_point<std::chrono::high_resolution_clock>::max();
        
        // Main consumption loop
        while (std::chrono::high_resolution_clock::now() < end_time && !stop_consumer) {
            IPCMessage msg;
            
            // Try to consume message with timeout
            int attempts = 0;
            bool consumed = false;
            
            while (!consumed && attempts < 10 && !stop_consumer) {
                consumed = queue.try_pop(msg);
                if (!consumed) {
                    attempts++;
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            }
            
            if (consumed) {
                messages_consumed++;
                
                // Report progress every second
                auto now = std::chrono::high_resolution_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_report);
                
                if (elapsed.count() >= 1) {
                    auto total_elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - start_time);
                    double rate = (messages_consumed - last_report_count) * 1000000.0 / elapsed.count();
                    double overall_rate = messages_consumed * 1000000.0 / total_elapsed.count();
                    
                    std::cout << "Consumer " << consumer_id 
                              << " | Messages: " << messages_consumed
                              << " | Rate: " << static_cast<int>(rate) << " msg/s"
                              << " | Total: " << static_cast<int>(overall_rate) << " msg/s"
                              << " | Queue: " << queue.size() << std::endl;
                    
                    last_report = now;
                    last_report_count = messages_consumed;
                }
            } else {
                // No message available, brief sleep
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
        
        auto actual_end = std::chrono::high_resolution_clock::now();
        auto total_time = std::chrono::duration_cast<std::chrono::microseconds>(actual_end - start_time);
        
        std::cout << "Consumer " << consumer_id << " completed" << std::endl;
        std::cout << "Messages Consumed: " << messages_consumed << std::endl;
        std::cout << "Total Time: " << total_time.count() << " μs" << std::endl;
        if (messages_consumed > 0) {
            std::cout << "Average Rate: " << (messages_consumed * 1000000.0 / total_time.count()) << " msg/s" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Consumer Error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "Consumer " << consumer_id << " exiting" << std::endl;
    return 0;
}