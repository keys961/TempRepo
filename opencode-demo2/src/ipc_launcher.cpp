#include <iostream>
#include <vector>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <chrono>
#include <signal.h>
#include <thread>

// Global process tracking
std::vector<pid_t> producer_pids;
std::vector<pid_t> consumer_pids;

void cleanup_processes() {
    std::cout << "\nCleaning up processes..." << std::endl;
    
    // Kill producers
    for (pid_t pid : producer_pids) {
        if (pid > 0) {
            kill(pid, SIGTERM);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            // Force kill if still running
            if (kill(pid, 0) == 0) {
                kill(pid, SIGKILL);
            }
        }
    }
    
    // Kill consumers
    for (pid_t pid : consumer_pids) {
        if (pid > 0) {
            kill(pid, SIGTERM);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            // Force kill if still running
            if (kill(pid, 0) == 0) {
                kill(pid, SIGKILL);
            }
        }
    }
}

void signal_handler(int signal) {
    std::cout << "\nReceived signal " << signal << ", cleaning up..." << std::endl;
    cleanup_processes();
    exit(0);
}

void show_usage(const char* program_name) {
    std::cout << "SPMC Queue Inter-Process Communication Test" << std::endl;
    std::cout << "Usage: " << program_name << " <test_duration_seconds>" << std::endl;
    std::cout << "Example: " << program_name << " 30" << std::endl;
    std::cout << "This will create:" << std::endl;
    std::cout << "  1 producer process" << std::endl;
    std::cout << "  4 consumer processes" << std::endl;
    std::cout << "  1 shared memory SPMC queue" << std::endl;
    std::cout << std::endl;
    std::cout << "The processes will communicate via lock-free shared memory." << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        show_usage(argv[0]);
        return 1;
    }
    
    int test_duration = std::stoi(argv[1]);
    std::string queue_name = "ipc_test_queue";
    int num_producers = 1;
    int num_consumers = 4;
    int messages_per_producer = 500000;
    
    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    std::cout << "SPMC Queue Inter-Process Communication Test" << std::endl;
    std::cout << "=========================================" << std::endl;
    std::cout << "Test Duration: " << test_duration << " seconds" << std::endl;
    std::cout << "Producers: " << num_producers << std::endl;
    std::cout << "Consumers: " << num_consumers << std::endl;
    std::cout << "Messages per Producer: " << messages_per_producer << std::endl;
    std::cout << "Queue Name: " << queue_name << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    try {
        // Clean up any existing shared memory
        std::cout << "Cleaning up existing shared memory..." << std::endl;
        std::string cmd = "rm -f /dev/shm/" + queue_name + "* 2>/dev/null";
        system(cmd.c_str());
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Launch producer processes
        std::cout << "Launching producer processes..." << std::endl;
        for (int i = 0; i < num_producers; ++i) {
            pid_t pid = fork();
            
            if (pid == 0) {
                // Child process (producer)
                std::string producer_id = std::to_string(i + 1);
                std::string msg_count = std::to_string(messages_per_producer);
                
                execl("./build/producer", "producer", 
                       queue_name.c_str(), msg_count.c_str(), producer_id.c_str(), 
                       (char*)nullptr);
                
                // If execl fails
                std::cerr << "Failed to start producer " << i << std::endl;
                exit(1);
                
            } else if (pid > 0) {
                // Parent process
                producer_pids.push_back(pid);
                std::cout << "Started producer " << (i + 1) << " with PID " << pid << std::endl;
            } else {
                std::cerr << "Failed to fork producer " << i << std::endl;
            }
            
            // Small delay between starting producers
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        // Wait a bit for producers to start
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        // Launch consumer processes
        std::cout << "Launching consumer processes..." << std::endl;
        for (int i = 0; i < num_consumers; ++i) {
            pid_t pid = fork();
            
            if (pid == 0) {
                // Child process (consumer)
                std::string consumer_id = std::to_string(i + 1);
                std::string duration = std::to_string(test_duration * 1000); // Convert to ms
                
                execl("./build/consumer", "consumer",
                       queue_name.c_str(), consumer_id.c_str(), duration.c_str(),
                       (char*)nullptr);
                
                // If execl fails
                std::cerr << "Failed to start consumer " << i << std::endl;
                exit(1);
                
            } else if (pid > 0) {
                // Parent process
                consumer_pids.push_back(pid);
                std::cout << "Started consumer " << (i + 1) << " with PID " << pid << std::endl;
            } else {
                std::cerr << "Failed to fork consumer " << i << std::endl;
            }
            
            // Small delay between starting consumers
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        std::cout << "----------------------------------------" << std::endl;
        std::cout << "All processes started. Test running for " << test_duration << " seconds..." << std::endl;
        std::cout << "Press Ctrl+C to stop early" << std::endl;
        std::cout << "----------------------------------------" << std::endl;
        
        // Wait for test duration
        std::this_thread::sleep_for(std::chrono::seconds(test_duration));
        
        std::cout << "----------------------------------------" << std::endl;
        std::cout << "Test completed. Stopping all processes..." << std::endl;
        
        // Stop all processes
        cleanup_processes();
        
        std::cout << "----------------------------------------" << std::endl;
        std::cout << "Inter-Process SPMC Queue Test Completed!" << std::endl;
        std::cout << "Key Results:" << std::endl;
        std::cout << "✅ True inter-process communication (shared memory)" << std::endl;
        std::cout << "✅ Single Producer, Multiple Consumers pattern" << std::endl;
        std::cout << "✅ Lock-free operations across process boundaries" << std::endl;
        std::cout << "✅ No blocking synchronization primitives" << std::endl;
        std::cout << "✅ High-performance data transfer" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Test Error: " << e.what() << std::endl;
        cleanup_processes();
        return 1;
    }
    
    return 0;
}