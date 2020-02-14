#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/errno.h>
#include <sys/time.h>

#define CONTENT_SIZE 1024 * 1024 * 4
#define SAMPLE_STRING "SAMP"
#define MAX_THREAD_COUNT 64

void *test_file_write(void *arguments);
void *test_mmap_write(void *arguments);
void *test_file_read(void *arguments);
void *test_mmap_read(void *arguments);
double diff_time(struct timeval *start, struct timeval *end);

typedef struct thread_args {
    int thread_no;
    long long file_size;
    int enable_sync;
    char* content;
} thread_args;

int main(int argc, char *argv[]) {
    if(argc < 5) {
        printf("Wrong format. Format: program thread file_size_in_mb r/w sync/async.\n");
        return 1;
    }
    // mmap
    // thread count, file size (MB), w/r, sync/async
    pthread_t file_threads[MAX_THREAD_COUNT];
    pthread_t mmap_threads[MAX_THREAD_COUNT];
    int thread_cnt = atoi(argv[1]);
    char *content = (char *)malloc(CONTENT_SIZE + 1);
    char *cursor = content;
    memset(content, 0, CONTENT_SIZE);
    int sample_str_len = strlen(SAMPLE_STRING);
    for(int i = 0; i < CONTENT_SIZE / sample_str_len; i++) {
        memcpy(cursor, SAMPLE_STRING, sample_str_len);
        cursor += sample_str_len;
    }
    thread_args args[thread_cnt];
    for(int i = 0; i < thread_cnt; i++) {
        args[i].thread_no = i;
        args[i].file_size = atol(argv[2]);
        args[i].content = content;
        args->enable_sync = strcmp("sync", argv[4]) == 0;
    }
    struct timeval start_time, end_time;
    if(strcmp("r", argv[3]) == 0) {
        // read
        gettimeofday(&start_time, NULL);
        for(int i = 0; i < thread_cnt; i++) {
            pthread_create(&file_threads[i], NULL, test_file_read, &args[i]);
        }
        for(int i = 0; i < thread_cnt; i++) {
            pthread_join(file_threads[i], NULL);
        }
        gettimeofday(&end_time, NULL);
        printf("Cost %.2f ms on FILE API, read %d files with each file %lld MB.\n",
               diff_time(&start_time, &end_time), thread_cnt, args[0].file_size);

        gettimeofday(&start_time, NULL);
        for(int i = 0; i < thread_cnt; i++) {
            pthread_create(&mmap_threads[i], NULL, test_mmap_read, &args[i]);
        }
        for(int i = 0; i < thread_cnt; i++) {
            pthread_join(mmap_threads[i], NULL);
        }
        gettimeofday(&end_time, NULL);
        printf("Cost %.2f ms on mmap API, read %d files with each file %lld MB.\n",
               diff_time(&start_time, &end_time), thread_cnt, args[0].file_size);
    } else {
        // write
        gettimeofday(&start_time, NULL);
        for(int i = 0; i < thread_cnt; i++) {
            pthread_create(&file_threads[i], NULL, test_file_write, &args[i]);
        }
        for(int i = 0; i < thread_cnt; i++) {
            pthread_join(file_threads[i], NULL);
        }
        gettimeofday(&end_time, NULL);
        printf("Cost %.2f ms on FILE API, created %d files with each file %lld MB.\n",
                diff_time(&start_time, &end_time), thread_cnt, args[0].file_size);

        gettimeofday(&start_time, NULL);
        for(int i = 0; i < thread_cnt; i++) {
            pthread_create(&mmap_threads[i], NULL, test_mmap_write, &args[i]);
        }
        for(int i = 0; i < thread_cnt; i++) {
            pthread_join(mmap_threads[i], NULL);
        }
        gettimeofday(&end_time, NULL);
        printf("Cost %.2f ms on mmap API, created %d files with each file %lld MB.\n",
               diff_time(&start_time, &end_time), thread_cnt, args[0].file_size);
    }
    free(content);
    return 0;
}

void *test_file_write(void *arguments) {
    thread_args *args = (thread_args *)arguments;
    char* file_name_template = "./file-%d.test";
    char filename[20] = {0};
    sprintf(filename, file_name_template, args->thread_no);
    FILE *fp = fopen(filename, "w+");
    if(fp == NULL) {
        printf("Cannot open file %s.\n", filename);
        return NULL;
    }

    long long written = 0;
    long long total_size = args->file_size * 1024 * 1024;
    while(written < total_size) {
        long long n = write(fileno(fp), args->content, CONTENT_SIZE);
        if(n < CONTENT_SIZE) {
            printf("Write failed, written %lld MB.\n", n / (1024 * 1024));
            break;
        }
        written += n;
    }
    if(args->enable_sync) {
        fsync(fileno(fp));
    }
    fclose(fp);
    return NULL;
}

void *test_mmap_write(void *arguments) {
    // mmap write will cause SIGSEGV
    thread_args *args = (thread_args *)arguments;
    char* file_name_template = "./mmap-%d.test";
    char filename[20] = {0};
    sprintf(filename, file_name_template, args->thread_no);
    FILE *fp = fopen(filename, "w+");
    if(fp == NULL) {
        printf("Cannot open file %s.\n", filename);
        return NULL;
    }
    long long total_size = args->file_size * (1024 * 1024);
    if(ftruncate(fileno(fp), total_size)) {
        printf("Truncated file failed.\n");
        fclose(fp);
        return NULL;
    }
    lseek(fileno(fp), 0, SEEK_SET);
    char *mapped_addr = (char *)mmap(NULL, total_size, PROT_READ | PROT_WRITE,
                             MAP_SHARED, fileno(fp), 0);
    char *cursor = mapped_addr;
    for(int i = 0; i < total_size; i += CONTENT_SIZE) {
        memcpy(cursor, args->content, CONTENT_SIZE);
        cursor += CONTENT_SIZE;
    }
    if(args->enable_sync) {
        msync(mapped_addr, total_size, MS_SYNC);
    }
    munmap(mapped_addr, total_size);
    fclose(fp);
    return NULL;
}

void *test_file_read(void *arguments) {
    thread_args *args = (thread_args *)arguments;
    char* file_name_template = "./file-%d.test";
    char filename[20] = {0};
    sprintf(filename, file_name_template, args->thread_no);
    FILE *fp = fopen(filename, "r+");
    if(fp == NULL) {
        printf("Cannot open file %s.\n", filename);
        return NULL;
    }
    char *buffer = (char *)malloc(CONTENT_SIZE + 1);
    long long total_size = args->file_size * 1024 * 1024;
    long long r = 0;
    while(r < total_size) {
        long long n = read(fileno(fp), buffer, CONTENT_SIZE);
        if(n < CONTENT_SIZE) {
            printf("Read failed, read %lld MB.\n", n / (1024 * 1024));
            break;
        }
        r += n;
    }
    fclose(fp);
    free(buffer);
    return NULL;
}

void *test_mmap_read(void *arguments) {
    thread_args *args = (thread_args *)arguments;
    char* file_name_template = "./mmap-%d.test";
    char filename[20] = {0};
    sprintf(filename, file_name_template, args->thread_no);
    FILE *fp = fopen(filename, "r+");
    if(fp == NULL) {
        printf("Cannot open file %s.\n", filename);
        return NULL;
    }

    char *buffer = (char *)malloc(CONTENT_SIZE + 1);
    char *mapped_addr = (char *)mmap(NULL, args->file_size * (1024 * 1024), PROT_READ | PROT_WRITE,
                                     MAP_SHARED, fileno(fp), 0);
    char *cursor = mapped_addr;

    long long total_size = args->file_size * (1024 * 1024);
    long long r = 0;
    while(r < total_size) {
        memcpy(buffer, cursor, CONTENT_SIZE);
        cursor += CONTENT_SIZE;
        r += CONTENT_SIZE;
    }

    munmap(mapped_addr, total_size);
    free(buffer);
    return NULL;
}

double diff_time(struct timeval *start, struct timeval *end) {
    double d;
    time_t s;
    suseconds_t u;
    s = end->tv_sec - start->tv_sec;
    u = end->tv_usec - start->tv_usec;
    d = s;
    d *= 1000000.0;
    d += u;
    return d / 1000.0;
}
