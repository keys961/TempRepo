#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <cstring>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <type_traits>

template<typename T>
concept SPMCQueueElement = 
    std::is_trivially_copyable_v<T> &&
    std::is_standard_layout_v<T> &&
    sizeof(T) > 0 &&
    !std::is_polymorphic_v<T> &&
    !std::is_reference_v<T>;

template<typename T, size_t SegmentSize = 1024 * 1024> // 1MB segments
requires SPMCQueueElement<T>
class SPMCQueue {

private:
    struct QueueSegment {
        alignas(64) std::atomic<size_t> write_pos{0};
        alignas(64) std::atomic<size_t> read_pos{0};
        alignas(64) std::atomic<QueueSegment*> next_segment{nullptr};
        alignas(64) T data[SegmentSize / sizeof(T)];
        
        QueueSegment() {
            write_pos.store(0, std::memory_order_relaxed);
            read_pos.store(0, std::memory_order_relaxed);
            next_segment.store(nullptr, std::memory_order_relaxed);
        }
    };

    struct SharedMemoryHeader {
        alignas(64) std::atomic<QueueSegment*> head_segment{nullptr};
        alignas(64) std::atomic<QueueSegment*> tail_segment{nullptr};
        alignas(64) std::atomic<size_t> total_segments{0};
        alignas(64) std::atomic<size_t> total_size{0}; // Total number of items in queue
    };

    SharedMemoryHeader* header_;
    void* shared_memory_;
    size_t memory_size_;
    int shm_fd_;
    bool is_creator_;
    std::string shm_name_;
    
    // For simplicity, use local memory segments
    std::vector<std::unique_ptr<QueueSegment>> allocated_segments_;
    QueueSegment* allocate_segment() {
        auto segment = std::make_unique<QueueSegment>();
        QueueSegment* ptr = segment.get();
        allocated_segments_.push_back(std::move(segment));
        return ptr;
    }

public:
    SPMCQueue(const std::string& name, size_t /* initial_segments */ = 1) 
        : header_(nullptr), shared_memory_(nullptr), memory_size_(sizeof(SharedMemoryHeader)), 
          shm_fd_(-1), is_creator_(false), shm_name_(name) {
        
        shm_name_ = "/" + shm_name_;
        
        // Try to create shared memory
        shm_fd_ = shm_open(shm_name_.c_str(), O_CREAT | O_RDWR, 0666);
        if (shm_fd_ == -1) {
            shm_fd_ = shm_open(shm_name_.c_str(), O_RDWR, 0666);
            if (shm_fd_ == -1) {
                throw std::runtime_error("Failed to open shared memory");
            }
            is_creator_ = false;
        } else {
            is_creator_ = true;
        }

        size_t initial_size = sizeof(SharedMemoryHeader);
        if (ftruncate(shm_fd_, initial_size) == -1) {
            throw std::runtime_error("Failed to size shared memory");
        }

        shared_memory_ = mmap(nullptr, initial_size,
                             PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_, 0);
        if (shared_memory_ == MAP_FAILED) {
            throw std::runtime_error("Failed to mmap shared memory");
        }

        header_ = static_cast<SharedMemoryHeader*>(shared_memory_);
        memory_size_ = initial_size;

        if (is_creator_) {
            QueueSegment* first_segment = allocate_segment();
            header_->head_segment.store(first_segment, std::memory_order_release);
            header_->tail_segment.store(first_segment, std::memory_order_release);
            header_->total_segments.store(1, std::memory_order_release);
            header_->total_size.store(0, std::memory_order_release);
        }
    }

    ~SPMCQueue() {
        if (shared_memory_) {
            munmap(shared_memory_, memory_size_);
        }
        if (shm_fd_ != -1) {
            close(shm_fd_);
        }
        if (is_creator_) {
            shm_unlink(shm_name_.c_str());
        }
    }

    // Producer API - Single Producer Only (COMPLETELY LOCK-FREE)
    
    // Copy-based push
    bool try_push(const T& item) {
        QueueSegment* tail = header_->tail_segment.load(std::memory_order_acquire);
        if (!tail) return false;

        size_t write_pos = tail->write_pos.load(std::memory_order_acquire);
        size_t capacity = sizeof(tail->data) / sizeof(T);
        
        // Check if current segment is full
        if (write_pos >= capacity) {
            // Try to extend with new segment
            QueueSegment* new_segment = allocate_segment();
            
            // Link new segment to current tail
            QueueSegment* expected_next = nullptr;
            if (!tail->next_segment.compare_exchange_strong(expected_next, new_segment,
                                                       std::memory_order_acq_rel)) {
                return false;
            }
            
            // Update tail pointer
            header_->tail_segment.store(new_segment, std::memory_order_release);
            header_->total_segments.fetch_add(1, std::memory_order_acq_rel);
            
            // Write to new segment
            new_segment->data[0] = item;
            new_segment->write_pos.store(1, std::memory_order_release);
            
            header_->total_size.fetch_add(1, std::memory_order_acq_rel);
            return true;
        }

        // Write to current segment
        tail->data[write_pos] = item;
        
        // Memory barrier
        std::atomic_thread_fence(std::memory_order_release);
        
        // Update write position
        tail->write_pos.store(write_pos + 1, std::memory_order_release);
        
        // Update total size
        header_->total_size.fetch_add(1, std::memory_order_acq_rel);
        
        return true;
    }

    // Move-based push
    bool try_push(T&& item) {
        QueueSegment* tail = header_->tail_segment.load(std::memory_order_acquire);
        if (!tail) return false;

        size_t write_pos = tail->write_pos.load(std::memory_order_acquire);
        size_t capacity = sizeof(tail->data) / sizeof(T);
        
        // Check if current segment is full
        if (write_pos >= capacity) {
            // Try to extend with new segment
            QueueSegment* new_segment = allocate_segment();
            
            // Link new segment to current tail
            QueueSegment* expected_next = nullptr;
            if (!tail->next_segment.compare_exchange_strong(expected_next, new_segment,
                                                       std::memory_order_acq_rel)) {
                return false;
            }
            
            // Update tail pointer
            header_->tail_segment.store(new_segment, std::memory_order_release);
            header_->total_segments.fetch_add(1, std::memory_order_acq_rel);
            
            // Move to new segment
            new_segment->data[0] = std::move(item);
            new_segment->write_pos.store(1, std::memory_order_release);
            
            header_->total_size.fetch_add(1, std::memory_order_acq_rel);
            return true;
        }

        // Move to current segment
        tail->data[write_pos] = std::move(item);
        
        // Memory barrier
        std::atomic_thread_fence(std::memory_order_release);
        
        // Update write position
        tail->write_pos.store(write_pos + 1, std::memory_order_release);
        
        // Update total size
        header_->total_size.fetch_add(1, std::memory_order_acq_rel);
        
        return true;
    }

    // Emplace with perfect forwarding
    template<typename... Args>
    bool try_emplace_back(Args&&... args) {
        QueueSegment* tail = header_->tail_segment.load(std::memory_order_acquire);
        if (!tail) return false;

        size_t write_pos = tail->write_pos.load(std::memory_order_acquire);
        size_t capacity = sizeof(tail->data) / sizeof(T);
        
        // Check if current segment is full
        if (write_pos >= capacity) {
            // Try to extend with new segment
            QueueSegment* new_segment = allocate_segment();
            
            // Link new segment to current tail
            QueueSegment* expected_next = nullptr;
            if (!tail->next_segment.compare_exchange_strong(expected_next, new_segment,
                                                       std::memory_order_acq_rel)) {
                return false;
            }
            
            // Update tail pointer
            header_->tail_segment.store(new_segment, std::memory_order_release);
            header_->total_segments.fetch_add(1, std::memory_order_acq_rel);
            
            // Emplace in new segment (perfect forwarding)
            new(&new_segment->data[0]) T(std::forward<Args>(args)...);
            new_segment->write_pos.store(1, std::memory_order_release);
            
            header_->total_size.fetch_add(1, std::memory_order_acq_rel);
            return true;
        }

        // Emplace in current segment (perfect forwarding)
        new(&tail->data[write_pos]) T(std::forward<Args>(args)...);
        
        // Memory barrier
        std::atomic_thread_fence(std::memory_order_release);
        
        // Update write position
        tail->write_pos.store(write_pos + 1, std::memory_order_release);
        
        // Update total size
        header_->total_size.fetch_add(1, std::memory_order_acq_rel);
        
        return true;
    }

    // Convenience wrappers
    void push(const T& item) {
        while (!try_push(item)) {
            std::this_thread::yield();
        }
    }

    void push(T&& item) {
        while (!try_push(std::move(item))) {
            std::this_thread::yield();
        }
    }

    template<typename... Args>
    void emplace_back(Args&&... args) {
        while (!try_emplace_back(std::forward<Args>(args)...)) {
            std::this_thread::yield();
        }
    }

    // Consumer API - Multiple Consumers Supported (COMPLETELY LOCK-FREE)
    bool try_pop(T& item) {
        QueueSegment* head = header_->head_segment.load(std::memory_order_acquire);
        if (!head) return false;

        while (true) {
            size_t read_pos = head->read_pos.load(std::memory_order_acquire);
            size_t write_pos = head->write_pos.load(std::memory_order_acquire);
            
            if (read_pos >= write_pos) {
                // Current segment is empty, try to move to next
                QueueSegment* next_segment = head->next_segment.load(std::memory_order_acquire);
                if (next_segment) {
                    // Try to advance head pointer
                    if (header_->head_segment.compare_exchange_strong(head, next_segment, 
                                                                     std::memory_order_acq_rel)) {
                        // Successfully advanced, continue with new head
                        head = next_segment;
                        continue;
                    } else {
                        // CAS failed, another consumer moved the head, reload and retry
                        head = header_->head_segment.load(std::memory_order_acquire);
                        continue;
                    }
                }
                // No next segment and current is empty -> queue is empty
                return false;
            }

            // Try to claim this position for reading
            size_t expected_read_pos = read_pos;
            if (head->read_pos.compare_exchange_strong(expected_read_pos, read_pos + 1, 
                                                      std::memory_order_acq_rel)) {
                // Successfully claimed this position
                
                // Memory barrier
                std::atomic_thread_fence(std::memory_order_acquire);
                
                // Copy item (or move if T supports move)
                item = head->data[read_pos];
                
                // Update total size
                header_->total_size.fetch_sub(1, std::memory_order_acq_rel);
                
                return true;
            }
            // CAS failed, another consumer claimed this position, retry
        }
    }

    bool empty() const {
        return header_->total_size.load(std::memory_order_acquire) == 0;
    }

    size_t size() const {
        return header_->total_size.load(std::memory_order_acquire);
    }

    // Non-blocking wait for data
    template<typename Rep, typename Period>
    bool wait_for_data(const std::chrono::duration<Rep, Period>& timeout) {
        auto start = std::chrono::steady_clock::now();
        while (empty()) {
            if (std::chrono::steady_clock::now() - start > timeout) {
                return false;
            }
            std::this_thread::yield();
        }
        return true;
    }
};