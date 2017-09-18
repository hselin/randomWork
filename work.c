#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>                // for gettimeofday()
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>
#include <openssl/sha.h>

#define KB                  (1024)
#define MB                  (1024 * KB)
#define GB                  (1024 * MB)
#define US_TO_MS(us)        (us / 1000)

#define DIV_ROUND_UP(n,d)   (((n) + (d) - 1) / (d))


struct threadRecord
{
    double elapsedTime;
};


struct threadIoCntx
{
    struct threadRecord tr;
    uint64_t numDigest;
    unsigned char digest[SHA256_DIGEST_LENGTH];
    char *buffer;
    uint64_t bufferSize;
};

uint64_t getElapsedTimeUS(struct timespec *start, struct timespec *end)
{
    return ((end->tv_sec - start->tv_sec) * 1000000) + ((end->tv_nsec - start->tv_nsec) / 1000);
}

static inline uint64_t randomNumber(uint64_t min, uint64_t max)
{
    return (rand() % (max + 1 - min) + min);
}


void sha256(char *buf, uint64_t bufLen, unsigned char *hash)
{
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, buf, bufLen);
    SHA256_Final(hash, &ctx);

    /*
    char outputBuffer[65];
    int i = 0;
    for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    }
    outputBuffer[64] = 0;

    printf("%s\n", outputBuffer);
    */
}

void *execFunc(void *arg)
{
    struct threadIoCntx *cntx = (struct threadIoCntx *)arg;

    while(cntx->numDigest)
    {
        for(size_t i = 0; i < cntx->bufferSize; i++)
            cntx->buffer[i] = rand() % 256;

        sha256((char *)cntx->buffer, cntx->bufferSize, cntx->digest);
        cntx->numDigest--;
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    if(argc != 4)
    {
        printf("Usage: ./randWork <num threads> <buffer size> <num digests>\n");
        exit(0);
    }

    char *numThreadStr = argv[1];
    char *bufferSizeStr = argv[2];
    char *numDigestStr = argv[3];

    printf("numThread: %s | %d\n", numThreadStr, atoi(numThreadStr));
    printf("bufferSizeStr: %s | %lu\n", bufferSizeStr, strtoul(bufferSizeStr, NULL, 0));
    printf("numDigestStr: %s | %lu\n", numDigestStr, strtoul(numDigestStr, NULL, 0));

    int numThread = atoi(numThreadStr);
    uint64_t bufferSize = strtoul(bufferSizeStr, NULL, 0);
    uint64_t numDigest = strtoul(numDigestStr, NULL, 0);

    srand(time(NULL));

    pthread_t threadIDs[numThread];
    struct threadIoCntx threadIOContexts[numThread];

    for(int i = 0; i < numThread; i++)
    {
        threadIOContexts[i].tr = {0};
        threadIOContexts[i].numDigest = numDigest;
        threadIOContexts[i].buffer = (char *)malloc(bufferSize);
        assert(threadIOContexts[i].buffer);
        threadIOContexts[i].bufferSize = bufferSize;
        pthread_create(&threadIDs[i], NULL, execFunc, (void *)&threadIOContexts[i]);
    }

    for (int i = 0; i < numThread; i++)
       pthread_join(threadIDs[i], NULL);
    
    return 0;
}

