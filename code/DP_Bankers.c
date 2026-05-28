#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <unistd.h>
#define NUM 5
#define ITER 1000000

sem_t forks[NUM];
sem_t m;
int Claim[NUM][NUM] = {{1, 1, 0, 0, 0},
                       {0, 1, 1, 0, 0},
                       {0, 0, 1, 1, 0},
                       {0, 0, 0, 1, 1},
                       {1, 0, 0, 0, 1}};
int Allocation[NUM][NUM];
int CA[NUM][NUM];
int Resource[NUM];
int V[NUM];

bool safe(int phil, int fork)
{
    if (!V[fork])
        return false;
    bool res = true;
    int tmp_CA[NUM][NUM];
    for (int i = 0; i < NUM; ++i)
        for (int j = 0; j < NUM; ++j)
            tmp_CA[i][j] = CA[i][j];
    int tmp_V[NUM];
    for (int i = 0; i < NUM; ++i)
        tmp_V[i] = V[i];
    tmp_CA[phil][fork] = 0;
    tmp_V[fork] = 0;

    bool finished[NUM];
    for (int i = 0; i < NUM; ++i)
        finished[i] = false;
    while (true)
    {
        bool nxt = true;

        for (int i = 0; i < NUM; ++i)
        {
            if (finished[i])
                continue;
            nxt = true;
            for (int j = 0; j < NUM; ++j)
            {
                if (tmp_CA[i][j] > tmp_V[j])
                    nxt = false;
            }
            if (nxt)
            {
                tmp_CA[i][i] = 0;
                tmp_CA[i][(i + 1) % NUM] = 0;
                tmp_V[i] = 1;
                tmp_V[(i + 1) % NUM] = 1;
                finished[i] = true;
                break;
            }
        }

        if (!nxt)
        {
            return false;
        }

        bool com = true;
        for (int i = 0; i < NUM; ++i)
        {
            if (!finished[i])
                com = false;
        }
        if (com)
            break;
    }
    return res;
}

void pickup(int phil, int fork)
{
    sem_wait(&forks[fork]);
    Allocation[phil][fork] = 1;
    CA[phil][fork] = 0;
    V[fork] = 0;
}
void putdown(int phil, int fork)
{
    sem_post(&forks[fork]);
    Allocation[phil][fork] = 0;
    CA[phil][fork] = 1;
    V[fork] = 1;
}
void thinking(int philosopher_num)
{
    printf("philosopher %d is thinking\n", philosopher_num);
    return;
}
void eating(int philosopher_num)
{
    printf("philosopher %d is eating\n", philosopher_num);
    return;
}
void *philosopher(void *arg)
{
    int philosopher_num;
    philosopher_num = (unsigned long int)arg;
    for (int cnt = 0; cnt < ITER; ++cnt)
    {
        sem_wait(&m);
        if (safe(philosopher_num, philosopher_num))
            pickup(philosopher_num, philosopher_num);
        else
        {
            cnt--;
            sem_post(&m);
            usleep(100);
            continue;
        }
        sem_post(&m);
        printf("philosopher %d picks up the fork %d.\n", philosopher_num, philosopher_num);
        sem_wait(&m);
        if (safe(philosopher_num, (philosopher_num + 1) % NUM))
            pickup(philosopher_num, (philosopher_num + 1) % NUM);
        else
        {
            putdown(philosopher_num, philosopher_num);
            cnt--;
            sem_post(&m);
            usleep(100);
            continue;
        }
        sem_post(&m);
        printf("philosopher %d picks up the fork %d.\n", philosopher_num, (philosopher_num + 1) % NUM);
        eating(philosopher_num);
        putdown(philosopher_num, (philosopher_num + 1) % NUM);
        printf("philosopher %d puts down the fork %d.\n", philosopher_num, (philosopher_num + 1) % NUM);
        putdown(philosopher_num, philosopher_num);
        printf("philosopher %d puts down the fork %d.\n", philosopher_num, philosopher_num);
        thinking(philosopher_num);
    }
    return NULL;
}

int main()
{
    sem_init(&m, 0, 1);
    for (int i = 0; i < NUM; i++)
    {
        sem_init(&forks[i], 0, 1);
    }
    for (int i = 0; i < NUM; ++i)
    {
        V[i] = 1;
        Resource[i] = 1;
        for (int j = 0; j < NUM; ++j)
        {
            Allocation[i][j] = 0;
            CA[i][j] = Claim[i][j] - Allocation[i][j];
        }
    }

    pthread_t threads[NUM];
    for (unsigned long int i = 0; i < NUM; i++)
    {
        pthread_create(&threads[i], NULL, philosopher, (void *)i);
    }
    for (int i = 0; i < NUM; i++)
    {
        pthread_join(threads[i], NULL);
    }
    printf("Bankers : NO DEADLOCK\n");
    sem_destroy(&m);
    return 0;
}