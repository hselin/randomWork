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
    uint64_t timeDuration;
};

uint64_t getElapsedTimeUS(struct timespec *start, struct timespec *end)
{
    return ((end->tv_sec - start->tv_sec) * 1000000) + ((end->tv_nsec - start->tv_nsec) / 1000);
}

static inline uint64_t randomNumber(uint64_t min, uint64_t max)
{
    return (rand() % (max + 1 - min) + min);
}

void *execFunc(void *arg)
{
    struct timespec t1, t2;
    struct threadIoCntx *cntx = (struct threadIoCntx *)arg;

    srand(time(NULL));
    
    clock_gettime(CLOCK_MONOTONIC_RAW, &t1);

    while(1)
    {
        clock_gettime(CLOCK_MONOTONIC_RAW, &t2);
        cntx->tr.elapsedTime = getElapsedTimeUS(&t1, &t2);

        if(US_TO_MS(cntx->tr.elapsedTime) >= cntx->timeDuration)
        {
            break;
        }
        else
        {
            //work work
            usleep(1);
        }
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        printf("Usage: ./randWork <num threads> <time duration>\n");
        exit(0);
    }

    char *numThreadStr = argv[1];
    char *timeDurationStr = argv[2];

    printf("numThread: %s | %d\n", numThreadStr, atoi(numThreadStr));
    printf("timeDuration: %s | %lu\n", timeDurationStr, strtoul(timeDurationStr, NULL, 0));

    int numThread = atoi(numThreadStr);
    uint64_t timeDuration = strtoul(timeDurationStr, NULL, 0);

    pthread_t threadIDs[numThread];
    struct threadIoCntx threadIOContexts[numThread];

    for(int i = 0; i < numThread; i++)
    {
        threadIOContexts[i].tr = {0};
        threadIOContexts[i].timeDuration = timeDuration;
        pthread_create(&threadIDs[i], NULL, execFunc, (void *)&threadIOContexts[i]);
    }

    for (int i = 0; i < numThread; i++)
       pthread_join(threadIDs[i], NULL);
    
    return 0;
}

