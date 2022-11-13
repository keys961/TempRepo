#include <sstream>
#include "meta.h"

int MetaStorage::get(std::string &key) {
    std::string value;
    rocksdb::Status status = db->Get(rocksdb::ReadOptions(), key, &value);
    if (status.ok()) {
        return 0;
    }
    return -1;
}

int MetaStorage::del(std::string &key) {
    rocksdb::Status status = db->Delete(rocksdb::WriteOptions(), key);
    if(status.ok() || status.IsNotFound()) {
        return 0;
    }
    return -1;
}

int MetaStorage::put(std::string &key) {
    rocksdb::Status status = db->Put(rocksdb::WriteOptions(), key, "0");
    if (status.ok()) {
        return 0;
    } else {
        perror("put key error: ");
        return -1;
    }
}
