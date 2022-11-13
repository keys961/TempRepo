#ifndef ENTRY_TASK_FILE_H
#define ENTRY_TASK_FILE_H


#include <cstdio>
#include <string>
#include <unistd.h>

#define MAX_CHUNK 1024

class FileIO {
private:
    std::string dir;
    FILE *fp;
public:
    FileIO(std::string &parent, std::string &path, const char *mode) {
        dir = parent + "/" + path;
        fp = fopen(dir.data(), mode);
    }

    ~FileIO() {
        if (fp != nullptr) {
            fclose(fp);
        }
    }

    FILE *getfp() {
        return fp;
    }

    std::string &getDir() {
        return dir;
    }
    int readData(char* buf, int size);
    int writeData(char* buf, int size);
    int renameFile(std::string &newDir);

    int fileLength();
};


#endif //ENTRY_TASK_FILE_H
