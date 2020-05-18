#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>

unsigned bit;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_rwlock_t rw_lock = PTHREAD_RWLOCK_INITIALIZER;
int *operationsArray;
int memberOpCount;
int insertOpCount;
int deleteOpCount;

unsigned genUniqueRandNum(unsigned short *lfsr)
{
    bit = ((*lfsr >> 0) ^ (*lfsr >> 2) ^ (*lfsr >> 3) ^ (*lfsr >> 5)) & 1;
    return *lfsr = (*lfsr >> 1) | (bit << 15);
}

//int Member(int value, struct list_node_s * head_p);
struct list_node_s
{
    int data;
    struct list_node_s *next;
};

struct thread_args
{
    long numOperations;
    unsigned short threadId;
    long numThreads;
    short unsigned seed;
    struct list_node_s **head;
};

int Member(int value, struct list_node_s *head_p)
{
    struct list_node_s *curr_p = head_p;

    while (curr_p != NULL && curr_p->data < value)
        curr_p = curr_p->next;

    if (curr_p == NULL || curr_p->data > value)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

int Insert(int value, struct list_node_s **head_pp)
{
    struct list_node_s *curr_p = *head_pp;
    struct list_node_s *pred_p = NULL;
    struct list_node_s *temp_p;

    while (curr_p != NULL && curr_p->data < value)
    {
        pred_p = curr_p;
        curr_p = curr_p->next;
    }
    if (curr_p == NULL || curr_p->data > value)
    {
        temp_p = malloc(sizeof(struct list_node_s));
        temp_p->data = value;
        temp_p->next = curr_p;
        if (pred_p == NULL)
            *head_pp = temp_p;
        else
            pred_p->next = temp_p;
        return 1;
    }
    else
    {
        return 0;
    }
}

int Delete(int value, struct list_node_s **head_pp)
{
    struct list_node_s *curr_p = *head_pp;
    struct list_node_s *pred_p = NULL;

    while (curr_p != NULL && curr_p->data < value)
    {
        pred_p = curr_p;
        curr_p = curr_p->next;
    }

    if (curr_p != NULL && curr_p->data == value)
    {
        if (pred_p == NULL)
        {
            *head_pp = curr_p->next;
            free(curr_p);
        }
        else
        {
            pred_p->next = curr_p->next;
            free(curr_p);
        }
        return 1;
    }
    else
    {
        return 0;
    }
}

void Traverse(struct list_node_s *node)
{
    struct list_node_s *traversingNode = node;
    while (traversingNode != NULL)
    {
        printf(" %d ", traversingNode->data);
        traversingNode = traversingNode->next;
    }
    printf("\n");
}

void populateLinkedList(struct list_node_s **head, int n)
{
    unsigned short lfsr = time(0);
    printf("\npopulating %d numbers...\n",n);
    for (int i = 0; i < n; ++i)
    {
        int a = genUniqueRandNum(&lfsr);
        Insert(a, head);
        printf("%d %d\n",i, a);
    }
    printf("\n");
}

void SerialProgram(int numOperations, struct list_node_s **head)
{
    for (int i = 0; i < numOperations; ++i)
    {
        unsigned short lfsr = rand();
        int randNum = genUniqueRandNum(&lfsr);
//        printf("%d,", operationsArray[i]);
        if (operationsArray[i] <= memberOpCount)
        {
             printf("%d Member %d \n",i, randNum);
            Member(randNum, *head);
        }
        else if (operationsArray[i] > memberOpCount && operationsArray[i] <= memberOpCount + insertOpCount)
        {
             printf("%d Insert %d \n",i, randNum);
            Insert(randNum, head);
        }
        else if (operationsArray[i] > memberOpCount + insertOpCount && operationsArray[i] <= memberOpCount + insertOpCount + deleteOpCount)
        {
             printf("%d Delete %d \n",i, randNum);
            Delete(randNum, head);
        }
    }
    printf("\n");
}

void *mutexProgram(void *ptr)
{
    struct thread_args *args = (struct thread_args *)ptr;
    long numOperations = args->numOperations;
    unsigned short threadId = args->threadId;
    long numThreads = args->numThreads;
    short unsigned seed = args->seed;
    struct list_node_s **head = args->head;

    for (int i = threadId; i < numOperations; i = i + numThreads)
    {
        int randNum = genUniqueRandNum(&seed);
        if (operationsArray[i] < memberOpCount)
        {
            // printf("inside for member\n");
            pthread_mutex_lock(&lock);
            // printf("Member %d : ", randNum);
            Member(randNum, *head);
            // printf("%d\n", result);
            pthread_mutex_unlock(&lock);
        }
        else if (operationsArray[i] >= memberOpCount && operationsArray[i] < memberOpCount + insertOpCount)
        {
            // printf("inside for insert\n");
            pthread_mutex_lock(&lock);
            // printf("Insert %d : ", randNum);
            Insert(randNum, head);
            // printf("%d\n", result);
            pthread_mutex_unlock(&lock);
        }
        else if (operationsArray[i] >= memberOpCount + insertOpCount && operationsArray[i] < memberOpCount + insertOpCount + deleteOpCount)
        {
            // printf("inside for delete\n");
            pthread_mutex_lock(&lock);
            // printf("Delete %d : ", randNum);
            Delete(randNum, head);
            // printf("%d\n", result);
            pthread_mutex_unlock(&lock);
        }
    }
    return EXIT_SUCCESS;
}

void *readWriteLockProgram(void *ptr)
{
    struct thread_args *args = (struct thread_args *)ptr;
    long numOperations = args->numOperations;
    unsigned short threadId = args->threadId;
    long numThreads = args->numThreads;
    short unsigned seed = args->seed;
    struct list_node_s **head = args->head;

    for (int i = threadId; i < numOperations; i = i + numThreads)
    {
        int randNum = genUniqueRandNum(&seed);
        if (operationsArray[i] < memberOpCount)
        {
            pthread_rwlock_rdlock(&rw_lock);
            // printf("Member %d : ", randNum);
            Member(randNum, *head);
            // printf("%d\n", result);
            pthread_rwlock_unlock(&rw_lock);
        }
        else if (operationsArray[i] >= memberOpCount && operationsArray[i] < memberOpCount + insertOpCount)
        {
            pthread_rwlock_wrlock(&rw_lock);
            // printf("Insert %d : ", randNum);
            Insert(randNum, head);
            // printf("%d\n", result);
            pthread_rwlock_unlock(&rw_lock);
        }
        else if (operationsArray[i] >= memberOpCount + insertOpCount && operationsArray[i] < memberOpCount + insertOpCount + deleteOpCount)
        {
            pthread_rwlock_wrlock(&rw_lock);
            // printf("Delete %d : ", randNum);
            Delete(randNum, head);
            // printf("%d\n", result);
            pthread_rwlock_unlock(&rw_lock);
        }
    }
    return EXIT_SUCCESS;
}

void runSerialProgram(struct list_node_s **header, int numOperations)
{
    struct list_node_s **head = header;
    SerialProgram(numOperations, head);
}

void runMutexProgram(struct list_node_s **header, int numOperations)
{
    struct list_node_s **head = header;
    long numThreads;
    pthread_t *threadHandles;

    printf("Enter number of threads : ");
    scanf("%ld", &numThreads);

    threadHandles = malloc(numThreads * sizeof(pthread_t));
    unsigned short thread;
    for (thread = 0; thread < numThreads; ++thread)
    {
        struct thread_args *thread_args_struct = malloc(sizeof(struct thread_args));
        thread_args_struct->numOperations = numOperations;
        thread_args_struct->head = head;
        thread_args_struct->threadId = thread;
        thread_args_struct->numThreads = numThreads;
        thread_args_struct->seed = thread + 1000;
        pthread_create(&threadHandles[thread], NULL, mutexProgram, (void *)thread_args_struct);
    }
    int t;
    for (t = 0; t < numThreads; ++t)
    {
        pthread_join(threadHandles[t], NULL);
    }
    free(threadHandles);
    pthread_mutex_destroy(&lock);
}

void runRWLockProgram(struct list_node_s **header, int numOperations)
{
    struct list_node_s **head = header;
    long numThreads;
    pthread_t *threadHandles;

    printf("Enter number of threads : ");
    scanf("%ld", &numThreads);

    threadHandles = malloc(numThreads * sizeof(pthread_t));
    unsigned short thread;
    for (thread = 0; thread < numThreads; ++thread)
    {
        struct thread_args *thread_args_struct = malloc(sizeof(struct thread_args));
        thread_args_struct->numOperations = numOperations;
        thread_args_struct->head = head;
        thread_args_struct->threadId = thread;
        thread_args_struct->numThreads = numThreads;
        thread_args_struct->seed = thread + 1000;

        pthread_create(&threadHandles[thread], NULL, readWriteLockProgram, (void *)thread_args_struct);
    }
    int t;
    for (t = 0; t < numThreads; ++t)
    {
        pthread_join(threadHandles[t], NULL);
    }
    free(threadHandles);
    pthread_mutex_destroy(&lock);
}

void createArray(int numOperations)
{
    operationsArray = malloc(sizeof(int) * numOperations);
    for (int i = 0; i < numOperations; ++i)
    {
        operationsArray[i] = i;
    }
}

void shuffleArray(int n)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int usec = tv.tv_usec;
    srand48(usec);
    if (n > 1)
    {
        int i;
        for (i = n - 1; i > 0; i--)
        {
            size_t j = (unsigned int)(drand48() * (i + 1));
            int t = operationsArray[j];
            operationsArray[j] = operationsArray[i];
            operationsArray[i] = t;
        }
    }
}

void assignOperationCounts(int numOperations,float probMember, float probInsert, float probDelete) {
    memberOpCount = numOperations * probMember;
    insertOpCount = numOperations * probInsert;
    deleteOpCount = numOperations * probDelete;
}

int main()
{
    int numKeys;
    float probMember;
    float probInsert;
    float probDelete;
    long numOperations;
    printf("Enter amount of keys to populate : ");
    scanf("%d", &numKeys);
    printf("Enter number of operations to run : ");
    scanf("%ld", &numOperations);
    printf("Enter probability of Member operations : ");
    scanf("%f", &probMember);
    printf("Enter probability of Insert operations : ");
    scanf("%f", &probInsert);
    printf("Enter probability of Delete operations : ");
    scanf("%f", &probDelete);

    createArray(numOperations);
    shuffleArray(numOperations);
    assignOperationCounts(numOperations, probMember, probInsert, probDelete);

    struct list_node_s *head = NULL;
    populateLinkedList(&head, numKeys);
     runSerialProgram(&head, numOperations);
    // runMutexProgram(&head, numOperations);
//    runRWLockProgram(&head, numOperations);
    Traverse(head);
    return 0;
}