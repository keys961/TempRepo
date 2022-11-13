# Entry Task: C++ Object Storage Server

## 1. Requirement

- C++ 11 with `g++`
- `cmake`
- RocksDB 6.29.x and its dependencies
- Ubuntu 16.04

## 2. Brief Design

### Server

1. Use `epoll` to listen to the incoming connections and read events
2. Use thread pool to handle the incoming request 
3. Directly write data buffer to `connfd`, no chunk optimization currently
4. Directly store the data on local storage, with its key as file name
   1. `PUT`: If it's new value, just create a new file. Or create a temp file, then remove the old file, finally rename it 
   2. `GET`: Fetch all data into memory and output currently, optimization may include:
      - `mmap`
      - `sendfile`
      - ...
   3. `DELETE`: Delete the file directly
   4. All the operation are updated on RocksDB
5. Metadata stores in RocksDB, with key itself and value as file size/buffer size
   1. RocksDB is not optimized
6. Use segmented mutexes to ensure that only 1 thread operate that key, to ensure atomicity
7. `Request` and `Response` with their format in comments

### Client

1. Just using standard socket to connect to the server
2. Reuse `Request` and `Response` and their serialization & deserialization logic

## 3. Schedule

- [x] Get familiar with basic C++, network programming & environment configuration (0.5 day)
- [x] Build the skeleton design (0.5 day)
- [x] Build the basic server & client and pass smoking test (1 day)
- [x] Test, bugfix and generate performance test doc (1-2 day)
- [x] Finish design, deployment, debug & concluding docs (1 day)