#include "file.h"

int FileIO::readData(char *buf, int size) {
    fseek(fp, 0, SEEK_SET);
    char* pt = buf;
    int count = size;
    while (count > 0) {
        int c = std::min(MAX_CHUNK, count);
        int len = fread(pt, sizeof(char), c, fp);
        if (len == -1) {
            return -1;
        } else if (len == 0) {
            return size - count;
        }
        pt += len;
        count -= len;
    }
    return size;
}

int FileIO::writeData(char *buf, int size) {
    fseek(fp, 0, SEEK_SET);
    char* pt = buf;
    int count = size;
    while (count > 0) {
        int c = std::min(MAX_CHUNK, count);
        int len = fwrite(pt, sizeof(char), c, fp);
        if (len == -1) {
            return -1;
        } else if (len == 0) {
            return size - count;
        }
        pt += len;
        count -= len;
    }
    fsync(fileno(fp));
    return size;
}

int FileIO::renameFile(std::string &newDir) {
    return rename(dir.data(), newDir.data());
}

int FileIO::fileLength() {
    if (fseek(fp, 0, SEEK_END) < 0) {
        return -1;
    } // seek to end of file
    int size = (int) ftell(fp); // get current file pointer
    if (size < 0 || fseek(fp, 0, SEEK_SET) < 0) {
        // rewind
        return -1;
    }
    return size;
}
