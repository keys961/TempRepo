FROM gcc:9.4.0

RUN apt-get update && \
    apt-get -y install build-essential wget cmake libsnappy-dev zlib1g-dev libbz2-dev liblz4-dev libzstd-dev libgflags-dev && \
    wget https://github.com/facebook/rocksdb/archive/refs/tags/v6.29.5.zip && \
    unzip v6.29.5.zip && \
    cd rocksdb-6.29.5 && \
    make shared_lib && cp librocksdb.so.6.29.5 /usr/local/lib && \
    make static_lib && make install && \
    ln -s librocksdb.so.6.29.5 /usr/local/lib/librocksdb.so.6.29 && \
    ln -s librocksdb.so.6.29.5 /usr/local/lib/librocksdb.so.6 && \
    ln -s librocksdb.so.6.29.5 /usr/local/lib/librocksdb.so && \
    echo 'LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib' >> /etc/profile && \
    echo 'export LD_LIBRARY_PATH' >> /etc/profile && \
    . /etc/profile && \
    make clean

COPY . /usr/src/entry-task
WORKDIR /usr/src/entry-task

RUN cmake -B "./build" && \
    cd ./build && \
    make && \
    cp entry_task-server .. && \
    cd ..

ENV ADDR="0.0.0.0"
ENV PORT="8080"
ENV DATA="./data"
ENV META="./meta"
ENV THREADS="2"
ENV LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/usr/local/lib"

EXPOSE 8080

VOLUME ["/usr/src/entry-task/data", "/usr/src/entry-task/meta"]

ENTRYPOINT ["sh", "-c", "./entry_task-server $ADDR $PORT $DATA $META $THREADS"]




