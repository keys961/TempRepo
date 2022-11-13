#include <csignal>
#include "server.h"

int main(int argc, char **argv) {
    if(argc != 6) {
        fprintf(stderr, "Wrong format. Format:./server addr port file-path meta-path threads.\n");
        return 1;
    }
    signal(SIGPIPE, SIG_IGN);
    std::string addr = std::string(argv[1]);
    int port = atoi(argv[2]);
    std::string data = std::string(argv[3]);
    std::string meta = std::string(argv[4]);
    int threads = atoi(argv[5]);
    Configuration conf = {
            addr, port, data, meta, threads
    };
    Server server(conf);
    if(server.start() < 0) {
        perror("error start server");
    }
    return 0;
}

