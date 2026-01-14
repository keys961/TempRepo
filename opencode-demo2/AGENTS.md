# AGENTS.md - C++20 SPMC Queue Project

This document provides essential information for agentic coding agents working in this repository.

## Build Commands

### Basic Build
```bash
cmake -S . -B build           # Configure CMake
cmake --build build           # Build all targets
```

### Individual Targets
```bash
cmake --build build --target producer     # Build producer only
cmake --build build --target consumer     # Build consumer only  
cmake --build build --target ipc_launcher # Build IPC launcher
cmake --build build --target tests        # Build test executable
```

### Test Commands
```bash
# Run all tests
cd build && ctest --output-on-failure

# Run specific test suite
cd build && ctest -R "spmc" --output-on-failure

# Run tests directly (useful for debugging)
./build/tests

# Single test execution (via gtest filter)
./build/tests --gtest_filter="*TestName*"
```

### Clean
```bash
cmake --build build --target clean
rm -rf build/                        # Full clean
```

## Project Structure

- **include/**: Header files (SPMCQueue.h)
- **src/**: Source files (producer.cpp, consumer.cpp, ipc_launcher.cpp)
- **tests/**: Unit tests using Google Test framework
- **build/**: Build directory (auto-generated)

## Code Style Guidelines

### C++ Standard and Compiler
- **Language**: C++20
- **Compiler**: GCC with strict warnings enabled
- **Warnings**: `-Wall -Wextra -Wpedantic -Werror` (treated as errors)

### Type Safety Requirements
The SPMCQueue template enforces strict compile-time constraints:
```cpp
requires SPMCQueueElement<T>
concept SPMCQueueElement = 
    std::is_trivially_copyable_v<T> &&
    std::is_standard_layout_v<T> &&
    sizeof(T) > 0 &&
    !std::is_polymorphic_v<T> &&
    !std::is_reference_v<T>;
```

**Valid Types**: POD structs, int, double, fixed-size arrays, no dynamic memory or virtual functions
**Invalid Types**: Classes with virtual functions, references, non-trivial destructors

### Naming Conventions
- **Classes**: PascalCase (e.g., `SPMCQueue`, `QueueSegment`)
- **Structs**: PascalCase (e.g., `IPCMessage`, `SharedMemoryHeader`)
- **Variables**: snake_case (e.g., `write_pos`, `memory_size_`)
- **Private members**: trailing underscore (`header_`, `shared_memory_`)
- **Functions**: snake_case (e.g., `try_push`, `try_pop`)
- **Constants**: UPPER_CASE (e.g., `SegmentSize` template parameter)

### Import and Include Organization
```cpp
// 1. System headers (alphabetical)
#include <atomic>
#include <memory>
#include <string>

// 2. C system headers (alphabetical)
#include <sys/mman.h>
#include <unistd.h>

// 3. Project headers
#include "SPMCQueue.h"
```

### Memory and Performance Guidelines
- **Cache-line alignment**: Use `alignas(64)` for performance-critical atomic variables
- **Memory ordering**: Prefer `memory_order_relaxed` for counters, `memory_order_acquire/release` for synchronization
- **Zero-copy**: Use `emplace_back()` with perfect forwarding for in-place construction
- **Shared memory**: Use proper mmap/munmap for inter-process communication

### Error Handling
- **Exceptions**: Use `std::runtime_error` for exceptional circumstances
- **Shared memory**: Clean up resources in destructors and error paths
- **Atomic operations**: Never throw in atomic contexts
- **Return codes**: Return `false` or `nullptr` for non-exceptional failures

### Thread Safety Patterns
- **SPMC**: Single Producer, Multiple Consumer pattern
- **Lock-free**: All operations must be lock-free atomic operations
- **Memory barriers**: Proper acquire/release semantics for data visibility
- **ABA prevention**: Use appropriate atomic operations and version counters

### Code Organization
```cpp
template<typename T, size_t SegmentSize = 1024 * 1024>
requires SPMCQueueElement<T>
class SPMCQueue {
private:
    // Forward declarations
    struct QueueSegment;
    struct SharedMemoryHeader;
    
    // Private members (trailing underscore)
    SharedMemoryHeader* header_;
    
public:
    // Public interface
    bool try_push(const T& item);
    bool try_pop(T& item);
    
private:
    // Helper functions
    QueueSegment* allocate_segment();
};
```

### Testing Guidelines
- **Framework**: Google Test (gtest)
- **Test naming**: `TEST_F(ClassName, TestDescription)` with descriptive names
- **Setup/Teardown**: Use `SetUp()` and `TearDown()` for test isolation
- **Shared memory**: Clean up `/dev/shm/*test*` files between test runs
- **Timeout**: 30 seconds per test (configured in CMake)

### Performance Considerations
- **Batch operations**: Where possible, batch queue operations
- **Minimize atomics**: Reduce contention on shared atomic variables
- **Memory allocation**: Pre-allocate segments to avoid runtime allocation
- **Cache locality**: Structure data for optimal cache utilization

### Platform-Specific Notes
- **Shared memory**: Linux `/dev/shm/` for inter-process communication
- **Signals**: Handle `SIGINT` and `SIGTERM` gracefully
- **Threading**: Use `std::thread` with proper joining
- **Timing**: Use `std::chrono` for high-resolution timing

## VSCode Integration
- **CMake Tools**: Automatic configuration on open
- **IntelliSense**: C++20 standard, GCC compiler detection
- **Debug configurations**: Available for all executables
- **Build tasks**: Pre-configured build tasks via CMake Tools

## Key Files to Understand
- `include/SPMCQueue.h:23`: Main queue implementation
- `src/producer.cpp:10`: IPC message structure definition
- `CMakeLists.txt:44`: Test executable configuration
- `tests/test_spmc_queue.cpp:22`: Test class setup and utilities

Remember: This is a high-performance, lock-free system. Always consider cache effects, atomic operation costs, and inter-process communication overhead when making changes.