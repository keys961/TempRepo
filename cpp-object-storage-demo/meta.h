#ifndef ENTRY_TASK_META_H
#define ENTRY_TASK_META_H

#include <string>
#include "rocksdb/db.h"

class MetaStorage {
private:
    rocksdb::DB* db;

public:
    MetaStorage(std::string& path) {
        rocksdb::Options options;
        options.create_if_missing = true;
        rocksdb::Status status = rocksdb::DB::Open(options, path.data(), &db);
        assert(status.ok());
    }

    ~MetaStorage() {
       delete db;
    }

    int get(std::string& key);
    int del(std::string &key);

    int put(std::string &key);
};

#endif //ENTRY_TASK_META_H
