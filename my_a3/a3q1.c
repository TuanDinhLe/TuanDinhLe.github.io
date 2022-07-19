//-----------------------------------------
// NAME: Tuan Le Dinh
// STUDENT NUMBER: 7845921
// COURSE: COMP 3430, SECTION: A01
// INSTRUCTOR: Franklin Bristow
// ASSIGNMENT: assignment 3, QUESTION: question 1
// 
// REMARKS: This program implements a simulation
// of the Multi-level feedback queue scheduling policy.
//-----------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <assert.h>
#include <sys/queue.h>
#include <pthread.h>

#define MILISECOND_CONVERSION 0.001

// Number of queues used in the simulation
#define NUM_QUEUE 4

// Indexes of ready queues with descending order of priority
#define PRIORITY_1 0 
#define PRIORITY_2 1
#define PRIORITY_3 2
// The running queue
#define RUNNING_QUEUE 3

// Max number of chaarcters of a line in task.txt
#define MAX_LINE_LEN 100

// Max number of chaarcters of a line in task.txt
#define MAX_ARG_LEN 50

// Number of args in a task
#define TASK_ARGS 4

typedef struct TASK
{
    char* task_name;
    int task_type;
    int task_length;
    int odd_of_io;
    int task_priority;
} task;

typedef struct ENTRY 
{
    task task_data;
    STAILQ_ENTRY(ENTRY) queue_entries;
} entry;

STAILQ_HEAD(QUEUE_HEAD, ENTRY);

typedef struct QUEUE_HEAD queue_head;

queue_head *task_queues;

pthread_mutex_t *queues_lock;

pthread_cond_t new_task_signal = PTHREAD_COND_INITIALIZER;

int new_task_ready = 1;

int done_scheduling = 0;
int done_reading = 0;

void enqueue(queue_head *head, task curr_task);

task dequeue(queue_head *head);

// Count the number of space in a line
int space_count(char *line);

// Return a task object from parsing an input line.
task parse_task(char *line);

// Return a task object from parsing an input line.
int parse_delay_time(char *line);

// Running the scheduler thread
void *task_scheduling(void *args);

void enqueue(queue_head *head, task curr_task)
{
    entry* new_entry = malloc(sizeof(entry));
    new_entry->task_data = curr_task;

    STAILQ_INSERT_TAIL(head, new_entry, queue_entries);
}

task dequeue(queue_head *head)
{
    entry* head_entry = STAILQ_FIRST(head);
    task head_task = head_entry->task_data;

    STAILQ_REMOVE_HEAD(head, queue_entries);

    return head_task;
}

int space_count(char *line)
{
    int space_count = 0;

    // Count the number of spaces to determine the number of each
    // line args at run time.
    for (int i = 0; i < (int) strlen(line); i++)
    {
        if (line[i] == ' ')
        {
            space_count += 1;
        }
    }

    return space_count;
}

task parse_task(char *line)
{
    task curr_task;
    curr_task.task_name = malloc(sizeof(char) * MAX_ARG_LEN);

    // Each arg for a task is separated by a space character.
    char *curr = strtok(strdup(line), " ");
    strcpy(curr_task.task_name, curr);

    curr = strtok(NULL, " ");
    curr_task.task_type = atoi(curr);

    curr = strtok(NULL, " ");
    curr_task.task_length = atoi(curr);

    curr = strtok(NULL, " ");
    curr_task.odd_of_io = atoi(curr);

    curr_task.task_priority = 1;

    return curr_task;
}

int parse_delay_time(char *line)
{
    int delay_time;
    char *curr = strtok(strdup(line), " ");

    if (strcmp(curr, "DELAY") == 0)
    {
        delay_time = atoi(strtok(NULL, " "));
    }
    else
    {
        delay_time = -1;
    }

    return delay_time;
}

void *cpu_processing(void *args)
{
    //(void) args;

    int tid = *(int *) args;

    task curr_task;

    printf("Thread ID %i wakes up.\n", tid);

    while (1)
    {
        printf("Thread ID %i waits for lock.\n", tid);
        pthread_mutex_lock(&queues_lock[RUNNING_QUEUE]);

        // If lower_case thread start first, it will yield the lock
        // and wait for signal to wake up.

        while (STAILQ_EMPTY(&task_queues[RUNNING_QUEUE]))
        {
            printf("Thread ID %i waits.\n", tid);

            // Terminate thread.
            if (done_scheduling && done_reading)
            {
                pthread_mutex_unlock(&queues_lock[RUNNING_QUEUE]);
                printf("Thread ID %i quits!!!!!!!!!!!\n", tid);
                return NULL;
            }

            pthread_cond_wait(&new_task_signal, &queues_lock[RUNNING_QUEUE]);
        }
        printf("Thread ID %i gets the lock.\n", tid);
        new_task_ready = 0;
        curr_task = dequeue(&task_queues[RUNNING_QUEUE]);
        printf("Task %s is being processed by thread id %i\n", curr_task.task_name, tid);

        pthread_mutex_unlock(&queues_lock[RUNNING_QUEUE]);
    }
}

void *task_scheduling(void *args)
{
    int num_cpu_threads = *(int *) args;
    printf("%i\n", num_cpu_threads);
    int empty_queues = 0;

    pthread_t *cpu_threads = malloc(sizeof(pthread_t) * num_cpu_threads);

    task curr_task;

    // Initialize CPU threads
    for (int i = 0; i < num_cpu_threads; i++)
    {
        pthread_create(&cpu_threads[i], NULL, cpu_processing, &cpu_threads[i]);
    }

    printf("Scheduling is here\n");

    while(!done_scheduling)
    {
        for (int i = 0; i < NUM_QUEUE - 1; i++)
        {
            if (!STAILQ_EMPTY(&task_queues[i]))
            {
                pthread_mutex_lock(&queues_lock[i]);
                curr_task = dequeue(&task_queues[i]);
                printf("A task is removed from queue priority %i\n", i+1);
                pthread_mutex_unlock(&queues_lock[i]);

                pthread_mutex_lock(&queues_lock[RUNNING_QUEUE]);
                enqueue(&task_queues[RUNNING_QUEUE], curr_task);
                printf("New task is added to the running queue\n");
                new_task_ready = 1;
                pthread_cond_signal(&new_task_signal);
                pthread_mutex_unlock(&queues_lock[RUNNING_QUEUE]);
            }
            else
            {
                empty_queues += 1;
            }
        }

        if (done_reading && empty_queues == NUM_QUEUE - 1)
        {
            done_scheduling = 1;
        }

        empty_queues = 0;
    }

    new_task_ready = 1;
    pthread_cond_broadcast(&new_task_signal);
    printf("Signal sent\n");

    for (int i = 0; i < num_cpu_threads; i++)
    {
        pthread_join(cpu_threads[i], NULL);
    }

    // while (!STAILQ_EMPTY(&task_queues[RUNNING_QUEUE]))
    // {
    //     pthread_mutex_lock(&queues_lock[RUNNING_QUEUE]);
    //     curr_task = dequeue(&task_queues[RUNNING_QUEUE]);
    //     pthread_mutex_unlock(&queues_lock[RUNNING_QUEUE]);
    //     printf("Task name is: %s\n", curr_task.task_name);
    // }

    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t scheduler;

    int cpu_threads;
    int boost_time;
    FILE *task_stream;

    int args_count;
    int delay_time;

    char* line;
    task curr_task;

    if (argc == 4)
    {
        task_queues = malloc(sizeof(queue_head) * NUM_QUEUE);
        queues_lock = malloc(sizeof(pthread_mutex_t) * NUM_QUEUE);

        cpu_threads = atoi(argv[1]);
        boost_time = atoi(argv[2]);
        task_stream = fopen(argv[3], "r");

        for (int i = 0; i < NUM_QUEUE; i++)
        {
            STAILQ_INIT(&task_queues[i]);
            pthread_mutex_init(&queues_lock[i], NULL);
        }

        printf("Starting MLFQ simulation with %i threads and boost time of %i miliseconds.\n", cpu_threads, boost_time);

        pthread_create(&scheduler, NULL, task_scheduling, &cpu_threads);

        line = malloc(sizeof(char) * MAX_LINE_LEN);

        while(fgets(line, MAX_LINE_LEN, task_stream) != NULL)
        {
            args_count = space_count(line) + 1;
            
            if (args_count == 4)
            {
                curr_task = parse_task(line);
                pthread_mutex_lock(&queues_lock[PRIORITY_1]);
                enqueue(&task_queues[PRIORITY_1], curr_task);
                printf("New task is added to queue priority 1.\n");
                pthread_mutex_unlock(&queues_lock[PRIORITY_1]);
            }
            else if (args_count == 2)
            {
                delay_time = parse_delay_time(line);

                if (delay_time == -1)
                {
                    printf("Invalid format for DELAY task.\n");
                    exit(EXIT_FAILURE);
                }
                sleep(delay_time * MILISECOND_CONVERSION);
            }
            else
            {
                printf("Invalid task argument format.\n");
                exit(EXIT_FAILURE);
            }
        }
        done_reading = 1;

        pthread_join(scheduler, NULL);
        printf("End of file.\n");

        exit(EXIT_SUCCESS);
    }
    else
    {
        printf("Insufficient arguments are provided.\n");
        exit(EXIT_FAILURE);
    }
}
