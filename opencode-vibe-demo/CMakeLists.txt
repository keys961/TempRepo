cmake_minimum_required(VERSION 3.20)
project(cpp_project VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set compiler warnings
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

find_package(Threads REQUIRED)

# Try to find GTest, provide help if not found
find_package(GTest QUIET)
if(NOT GTest_FOUND)
    message(WARNING "GTest not found. You may need to install it:\n"
                   "  Ubuntu/Debian: sudo apt-get install libgtest-dev\n"
                   "  macOS: brew install googletest\n"
                   "  Or build from source: https://github.com/google/googletest")
    find_package(GTest REQUIRED)
endif()

# Modern approach: Use target_include_directories instead of include_directories
include_directories(include)

# IPC Demo Programs
add_executable(producer src/producer.cpp)
target_include_directories(producer PRIVATE include)
target_link_libraries(producer Threads::Threads)

add_executable(consumer src/consumer.cpp)
target_include_directories(consumer PRIVATE include)
target_link_libraries(consumer Threads::Threads)

add_executable(ipc_launcher src/ipc_launcher.cpp)
target_include_directories(ipc_launcher PRIVATE include)
target_link_libraries(ipc_launcher Threads::Threads)

# Test executable
add_executable(tests 
    tests/test_smpc_queue_simple.cpp
    tests/test_spmc_queue.cpp
)
target_include_directories(tests PRIVATE include)
target_link_libraries(tests 
    GTest::gtest
    GTest::gtest_main
    Threads::Threads
)

# Enable testing
enable_testing()

# Register tests
add_test(NAME spmc_queue_tests COMMAND tests)

# Set test properties
set_tests_properties(spmc_queue_tests PROPERTIES
    TIMEOUT 30
)