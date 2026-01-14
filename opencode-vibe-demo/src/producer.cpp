#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <cstring>
#include <signal.h>
#include "SPMCQueue.h"

// Fixed-size message structure for inter-process communication
struct IPCMessage {
    uint64_t timestamp;
    uint32_t producer_id;
    uint32_t sequence;
    char payload[64];
    
    IPCMessage() : timestamp(0), producer_id(0), sequence(0) {
        std::memset(payload, 0, sizeof(payload));
    }
    
    IPCMessage(uint32_t prod_id, uint32_t seq, const std::string& data) 
        : producer_id(prod_id), sequence(seq) {
        timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        strncpy(payload, data.c_str(), sizeof(payload) - 1);
        payload[sizeof(payload) - 1] = '\0';
    }
    
    bool operator==(const IPCMessage& other) const {
        return timestamp == other.timestamp && 
               producer_id == other.producer_id && 
               sequence == other.sequence &&
               strcmp(payload, other.payload) == 0;
    }
};

volatile sig_atomic_t stop_producer = 0;

void signal_handler(int /* signal */) {
    stop_producer = 1;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cout << "Usage: " << argv[0] << " <queue_name> <num_messages> <producer_id>" << std::endl;
        std::cout << "Example: " << argv[0] << " ipc_test 1000000 1" << std::endl;
        return 1;
    }
    
    std::string queue_name = argv[1];
    int num_messages = std::stoi(argv[2]);
    int producer_id = std::stoi(argv[3]);
    
    // Set up signal handler for graceful shutdown
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    std::cout << "Producer Process Started" << std::endl;
    std::cout << "Process ID: " << getpid() << std::endl;
    std::cout << "Queue Name: " << queue_name << std::endl;
    std::cout << "Messages to Produce: " << num_messages << std::endl;
    std::cout << "Producer ID: " << producer_id << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    try {
        // Open/create shared memory queue
        SPMCQueue<IPCMessage> queue(queue_name);
        
        // Performance tracking
        auto start_time = std::chrono::high_resolution_clock::now();
        auto last_report = start_time;
        int messages_produced = 0;
        int last_report_count = 0;
        
        // Random data generator
        std::mt19937 rng(producer_id);
        std::uniform_int_distribution<int> data_dist(1, 1000);
        
        std::cout << "Starting production..." << std::endl;
        
        for (int i = 0; i < num_messages && !stop_producer; ++i) {
            // Create message with variable content
            std::string payload = "Producer_" + std::to_string(producer_id) + 
                                "_Message_" + std::to_string(i) + 
                                "_Data_" + std::to_string(data_dist(rng));
            
            IPCMessage msg(producer_id, i, payload);
            
            // Try to push with timeout
            bool pushed = false;
            int attempts = 0;
            while (!pushed && attempts < 1000 && !stop_producer) {
                pushed = queue.try_push(std::move(msg));
                if (!pushed) {
                    attempts++;
                    std::this_thread::sleep_for(std::chrono::microseconds(1));
                }
            }
            
            if (pushed) {
                messages_produced++;
                
                // Report progress every second
                auto now = std::chrono::high_resolution_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_report);
                
                if (elapsed.count() >= 1) {
                    auto total_elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - start_time);
                    double rate = (messages_produced - last_report_count) * 1000000.0 / elapsed.count();
                    double overall_rate = messages_produced * 1000000.0 / total_elapsed.count();
                    
                    std::cout << "Producer " << producer_id 
                              << " | Messages: " << messages_produced << "/" << num_messages
                              << " | Current Rate: " << static_cast<int>(rate) << " msg/s"
                              << " | Overall Rate: " << static_cast<int>(overall_rate) << " msg/s"
                              << " | Queue Size: " << queue.size()
                              << std::endl;
                    
                    last_report = now;
                    last_report_count = messages_produced;
                }
            } else if (!stop_producer) {
                std::cerr << "Failed to push message after 1000 attempts!" << std::endl;
                break;
            }
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto total_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        
        std::cout << "----------------------------------------" << std::endl;
        std::cout << "Producer Process Completed" << std::endl;
        std::cout << "Messages Produced: " << messages_produced << std::endl;
        std::cout << "Total Time: " << total_time.count() << " μs" << std::endl;
        std::cout << "Average Rate: " << (messages_produced * 1000000.0 / total_time.count()) << " msg/s" << std::endl;
        std::cout << "Final Queue Size: " << queue.size() << std::endl;
        
        // Wait a bit to let consumers catch up
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
    } catch (const std::exception& e) {
        std::cerr << "Producer Error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "Producer Process Exiting" << std::endl;
    return 0;
}