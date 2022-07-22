//-----------------------------------------
// NAME: Tuan Le Dinh
// STUDENT NUMBER: 7845921
// COURSE: COMP 3430, SECTION: A01
// INSTRUCTOR: Franklin Bristow
// ASSIGNMENT: assignment 3, QUESTION: question 1
// 
// REMARKS: This program implements a simulation
// of the multi-level feedback queue scheduling policy.
//-----------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <assert.h>
#include <sys/queue.h>
#include <pthread.h>
#include <time.h>

// Time constants given by assignment 3.

#define TIME_SLICE 50
#define ALLOTTED_TIME 200

#define NANOS_PER_USEC 1000
#define USEC_PER_SEC   1000000

#define MILISEC_PER_SEC 1000

#define IO_UPPERBOUND   100

// Number of priority queues.
#define NUM_PRIORITY_QUEUE 3

// Number of queues used in the simulation.
#define NUM_QUEUE 5

// The task scheduler queue index.
#define TASK_SCHEDULE_QUEUE 0

// Indexes of ready queues with descending order of priority.
#define PRIORITY_1 1 
#define PRIORITY_2 2
#define PRIORITY_3 3

// The running queue that the cpu threads will get task from.
#define RUNNING_QUEUE 4

// Max number of characters of a line from task.txt .
#define MAX_LINE_LEN 100

// Max number of characters of a task name from task.txt .
#define MAX_ARG_LEN 100

// Number of args in a task.
#define TASK_ARGS 4

// Number of task types.
#define TASK_TYPES 4

// Task type indexes.
#define TASK_TYPE_1 0
#define TASK_TYPE_2 1
#define TASK_TYPE_3 2
#define TASK_TYPE_IO 3

typedef struct timespec time_spec;
typedef struct SIMULATION_BOOKEEPER
{
    time_t *total_turnaround_time;
    time_t *total_response_time;
    int *task_num;
    int total_task_num;
    int total_task_done;
} simulation_bookeeper;

// Define type for the wrapper STAILQ provided by <sys/queue.h>.
STAILQ_HEAD(QUEUE_HEAD, QUEUE_ENTRY);
typedef struct QUEUE_HEAD queue_head;

typedef struct TASK
{
    char* task_name;
    int task_type;
    time_t task_length;
    int io_odd;
    int task_priority;
    time_t alloted_time;
    int is_first_run;
    time_spec arrival_time;
} task;

typedef struct QUEUE_ENTRY 
{
    task task_data;
    STAILQ_ENTRY(QUEUE_ENTRY) queue_entries;
} entry;

// Store the relevant data to produce simulation report
// on turnaround time and response time.
simulation_bookeeper bookeeper;

// Store the 5 data queues whose indexes are defined above.
queue_head *task_queues;

// Store 5 locks for each of the queue above.
pthread_mutex_t *queues_lock;

// Lock when modifying the bookeeper.
pthread_mutex_t simulation_bookeeper_lock;

// Signal that cpu threads listen from the scheduler thread.
pthread_cond_t new_task_signal = PTHREAD_COND_INITIALIZER;

// Time to sleep until a priority boosting is done.
int boost_time;

// Conditions to end simulation.
int done_scheduling = 0;
int done_reading = 0;

static void microsleep(unsigned int usecs);

time_spec diff(time_spec start, time_spec end);

// Convert from time_spec to microseconds.
time_t timespec_to_usec(time_spec time);

// Wrapper function to insert and remove a task from a queue.
void enqueue(queue_head *head_ptr, task curr_task);
task dequeue(queue_head *head_ptr);

// Count the number of space in a line to distinguish
// between a task and a delay.
int space_count(char *line);

// Return a task object from parsing an input line.
task parse_task(char *line);

// Return a task object from parsing an input line.
int parse_delay_time(char *line);

// Running the priority booster thread.
void *priority_boosting(void *args);

// Internal function that the priority booster thread can
// call to raise priority of tasks that are not in the ready
// queues (more details at function's implementation).
void priority_boosting_queue(queue_head* head_ptr);

// Running the cpu thread.
void *cpu_processing(void *args);

// Running the scheduler thread.
void *task_scheduling(void *args);

static void microsleep(unsigned int usecs)
{
    long seconds = usecs / USEC_PER_SEC;
    long nanos   = (usecs % USEC_PER_SEC) * NANOS_PER_USEC;
    struct timespec t = { .tv_sec = seconds, .tv_nsec = nanos };
    int ret;
    do
    {
        ret = nanosleep( &t, &t );
        // need to loop, `nanosleep` might return before sleeping
        // for the complete time (see `man nanosleep` for details)
    } while (ret == -1 && (t.tv_sec || t.tv_nsec));
}

time_spec diff(time_spec start, time_spec end)
{
    printf("Start time is: %ld\n", timespec_to_usec(start));
    printf("End time is: %ld\n", timespec_to_usec(end));

	time_spec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}

    printf("Difference time is: %ld\n", timespec_to_usec(temp));

	return temp;

}

time_t timespec_to_usec(time_spec time)
{
    time_t time_in_usec = time.tv_sec * USEC_PER_SEC + time.tv_nsec / NANOS_PER_USEC;
    return time_in_usec;
}

void enqueue(queue_head *head_ptr, task curr_task)
{
    entry* new_entry = malloc(sizeof(entry));
    new_entry->task_data = curr_task;

    STAILQ_INSERT_TAIL(head_ptr, new_entry, queue_entries);
}

task dequeue(queue_head *head_ptr)
{
    entry* head_entry = STAILQ_FIRST(head_ptr);
    task head_task = head_entry->task_data;

    STAILQ_REMOVE_HEAD(head_ptr, queue_entries);

    free(head_entry);

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
    curr_task.task_length = (time_t) atoi(curr);

    curr = strtok(NULL, " ");
    curr_task.io_odd = atoi(curr);

    curr_task.task_priority = 1;

    curr_task.alloted_time = ALLOTTED_TIME;

    curr_task.is_first_run = 1;

    clock_gettime(CLOCK_REALTIME, &curr_task.arrival_time);

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
    int io_odd;
    int tid = *(int *) args;

    // The actual time that a cpu thread will 'work'
    time_t run_time_usec = TIME_SLICE;

    task curr_task;

    time_spec finish_time;
    time_spec turnaround_time;
    time_spec first_run_time;
    time_spec response_time;

    srand(time(NULL));
    io_odd = rand() % (IO_UPPERBOUND + 1);

    printf("Thread ID %i wakes up.\n", tid);

    while (1)
    {
        //printf("Thread ID %i waits for lock.\n", tid);
        pthread_mutex_lock(&queues_lock[RUNNING_QUEUE]);

        while (STAILQ_EMPTY(&task_queues[RUNNING_QUEUE]))
        {
            //printf("Thread ID %i waits.\n", tid);

            // Terminate thread.
            if (done_scheduling && done_reading)
            {
                pthread_mutex_unlock(&queues_lock[RUNNING_QUEUE]);
                printf("Thread ID %i quits!!!!!!!!!!!\n", tid);
                return NULL;
            }

            pthread_cond_wait(&new_task_signal, &queues_lock[RUNNING_QUEUE]);
        }
        //printf("Thread ID %i gets the lock.\n", tid);
        curr_task = dequeue(&task_queues[RUNNING_QUEUE]);
        //printf("Task %s is being processed by thread id %i\n", curr_task.task_name, tid);

        if (curr_task.is_first_run)
        {
            curr_task.is_first_run = 0;

            // get the first runtime of this task
            clock_gettime(CLOCK_REALTIME, &first_run_time);

            // get the response time and add it to the total response time
            // accoriding to their task type.
            response_time = diff(curr_task.arrival_time, first_run_time);
            printf("Task %s response time is %ld usec.\n", curr_task.task_name, timespec_to_usec(response_time));

            pthread_mutex_lock(&simulation_bookeeper_lock);
            bookeeper.total_response_time[curr_task.task_type] += timespec_to_usec(response_time);
            bookeeper.task_num[curr_task.task_type] += 1;
            //printf("Current count for task type %i is %i.\n", curr_task.task_type, bookeeper.task_num[curr_task.task_type]);
            pthread_mutex_unlock(&simulation_bookeeper_lock);
        }

        // get correct run_time

        if (curr_task.task_length <= TIME_SLICE)
        {
            run_time_usec = curr_task.task_length;
        }

        if (io_odd <= curr_task.io_odd)
        {
            run_time_usec = rand() % (TIME_SLICE + 1);
            //printf("Task %s do I/O\n", curr_task.task_name);
        }

        // run
        microsleep(run_time_usec);

        // decrement remaining task time
        curr_task.task_length -= run_time_usec;

        if (curr_task.task_length <= 0)
        {
            // get the arrival runtime
            clock_gettime(CLOCK_REALTIME, &finish_time);

            // get the turnaround time and add it to the total turnaround time
            // accoriding to their task type.
            turnaround_time = diff(curr_task.arrival_time, finish_time);
            printf("Task %s turnaround time is %ld usec.\n", curr_task.task_name, timespec_to_usec(turnaround_time));

            pthread_mutex_lock(&simulation_bookeeper_lock);
            bookeeper.total_turnaround_time[curr_task.task_type] += timespec_to_usec(turnaround_time);
            printf("Current total turnaround time for task type %i is %ld", curr_task.task_type, bookeeper.total_turnaround_time[curr_task.task_type]);
            bookeeper.total_task_done += 1;
            pthread_mutex_unlock(&simulation_bookeeper_lock);

            free(curr_task.task_name);
        }
        else
        {
            //printf("Remaining time for task %s is %ld usecs.\n", curr_task.task_name, curr_task.task_length);

            // check if allot time is more than run time
            if (curr_task.alloted_time > run_time_usec)
            {
                curr_task.alloted_time -= run_time_usec;
                //printf("Remaining allottemnt time for task %s is %ld usecs.\n", curr_task.task_name, curr_task.alloted_time);
            }
            else
            {
                if (curr_task.task_priority != PRIORITY_3)
                {
                    curr_task.task_priority += 1;
                }
                curr_task.alloted_time = ALLOTTED_TIME;
            }

            // move to task scheduling queue
            pthread_mutex_lock(&queues_lock[TASK_SCHEDULE_QUEUE]);
            enqueue(&task_queues[TASK_SCHEDULE_QUEUE], curr_task);
            pthread_mutex_unlock(&queues_lock[TASK_SCHEDULE_QUEUE]);
        }

        pthread_mutex_unlock(&queues_lock[RUNNING_QUEUE]);
    }
}

void *task_scheduling(void *args)
{
    int curr_priority;
    int curr_task_type;
    task curr_task;

    int num_cpu_threads = *(int *) args;
    pthread_t *cpu_threads = malloc(sizeof(pthread_t) * num_cpu_threads);

    int empty_queues = 0;

    // Initialize CPU threads
    for (int i = 0; i < num_cpu_threads; i++)
    {
        pthread_create(&cpu_threads[i], NULL, cpu_processing, &cpu_threads[i]);
    }

    //printf("Scheduling is here\n");

    while(!done_scheduling)
    {
        pthread_mutex_lock(&queues_lock[TASK_SCHEDULE_QUEUE]);

        // Read all tasks available from the task loader
        while (!STAILQ_EMPTY(&task_queues[TASK_SCHEDULE_QUEUE]))
        {
            curr_task = dequeue(&task_queues[TASK_SCHEDULE_QUEUE]);
            curr_priority = curr_task.task_priority;
            curr_task_type = curr_task.task_type;

            pthread_mutex_lock(&queues_lock[curr_priority]);
            enqueue(&task_queues[curr_priority], curr_task);
            pthread_mutex_unlock(&queues_lock[curr_priority]);
        }

        pthread_mutex_unlock(&queues_lock[TASK_SCHEDULE_QUEUE]);

        for (int i = PRIORITY_1; i <= PRIORITY_3; i++)
        {

            pthread_mutex_lock(&queues_lock[i]);

            while (!STAILQ_EMPTY(&task_queues[i]))
            {
                curr_task = dequeue(&task_queues[i]);
                //printf("A task %s is removed from queue priority %i\n", curr_task.task_name, i);

                pthread_mutex_lock(&queues_lock[RUNNING_QUEUE]);
                enqueue(&task_queues[RUNNING_QUEUE], curr_task);
                //printf("New task is added to the running queue\n");
                pthread_cond_signal(&new_task_signal);
                pthread_mutex_unlock(&queues_lock[RUNNING_QUEUE]);
            }

            pthread_mutex_unlock(&queues_lock[i]);

            empty_queues += 1;
        }

        pthread_mutex_lock(&queues_lock[TASK_SCHEDULE_QUEUE]);

        if (done_reading && 
            STAILQ_EMPTY(&task_queues[TASK_SCHEDULE_QUEUE]) && 
            empty_queues == NUM_PRIORITY_QUEUE && 
            bookeeper.total_task_num == bookeeper.total_task_done
            )
        {
            done_scheduling = 1;
        }
        else
        {
            empty_queues = 0;
        }

        pthread_mutex_unlock(&queues_lock[TASK_SCHEDULE_QUEUE]);
    }

    pthread_cond_broadcast(&new_task_signal);
    //printf("Signal sent\n");

    for (int i = 0; i < num_cpu_threads; i++)
    {
        pthread_join(cpu_threads[i], NULL);
    }

    free(cpu_threads);

    return NULL;
}

void *priority_boosting(void *args)
{
    (void) args;

    task curr_task;

    while(!done_scheduling)
    {
        sleep(boost_time / MILISEC_PER_SEC);

        pthread_mutex_lock(&queues_lock[RUNNING_QUEUE]);
        priority_boosting_queue(&task_queues[RUNNING_QUEUE]);
        pthread_mutex_unlock(&queues_lock[RUNNING_QUEUE]);

        for (int i = PRIORITY_2; i <= PRIORITY_3; i++)
        {
            pthread_mutex_lock(&queues_lock[i]);

            while (!STAILQ_EMPTY(&task_queues[i]))
            {
                curr_task = dequeue(&task_queues[i]);
                curr_task.task_priority = PRIORITY_1;
                curr_task.alloted_time = ALLOTTED_TIME;

                pthread_mutex_lock(&queues_lock[PRIORITY_1]);
                enqueue(&task_queues[PRIORITY_1], curr_task);
                pthread_mutex_unlock(&queues_lock[PRIORITY_1]);
            }

            pthread_mutex_unlock(&queues_lock[i]);
        }

        pthread_mutex_lock(&queues_lock[TASK_SCHEDULE_QUEUE]);
        priority_boosting_queue(&task_queues[TASK_SCHEDULE_QUEUE]);
        pthread_mutex_unlock(&queues_lock[TASK_SCHEDULE_QUEUE]);
    }

    return NULL;
}

void priority_boosting_queue(queue_head* head_ptr)
{
    queue_head temp_head;
    STAILQ_INIT(&temp_head);

    task curr_task;

    while (!STAILQ_EMPTY(head_ptr))
    {
        curr_task = dequeue(head_ptr);
        //printf("Before: Task %s has a priority of %i with %ld alloted time.\n", curr_task.task_name, curr_task.task_priority, curr_task.alloted_time);
        curr_task.task_priority = PRIORITY_1;
        curr_task.alloted_time = ALLOTTED_TIME;
        //printf("After: Task %s has a priority of %i with %ld alloted time.\n", curr_task.task_name, curr_task.task_priority, curr_task.alloted_time);
        enqueue(&temp_head, curr_task);
    }

    while (!STAILQ_EMPTY(&temp_head))
    {
        curr_task = dequeue(&temp_head);
        //printf("Now: Task %s has a priority of %i with %ld alloted time.\n", curr_task.task_name, curr_task.task_priority, curr_task.alloted_time);
        enqueue(head_ptr, curr_task);
    }
}

int main(int argc, char *argv[])
{
    pthread_t scheduler, priority_booster;

    int cpu_threads;
    FILE *task_stream;

    int args_count;
    int delay_time;

    char* line;
    task curr_task;

    if (argc == 4)
    {
        // Initialize the time bookeeper data structure.
        bookeeper.total_turnaround_time = malloc(sizeof(time_t) * TASK_TYPES);
        bookeeper.total_response_time = malloc(sizeof(time_t) * TASK_TYPES);
        bookeeper.task_num = malloc(sizeof(int) * TASK_TYPES);
        bookeeper.total_task_num = 0;
        bookeeper.total_task_done = 0;

        for (int i = 0; i < TASK_TYPES; i++)
        {
            bookeeper.total_turnaround_time[i] = 0;
            bookeeper.total_response_time[i] = 0;
            bookeeper.task_num[i] = 0;
        }

        // Initialize queues and locks for those queues.
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
        pthread_mutex_init(&simulation_bookeeper_lock, NULL);

        printf("Starting MLFQ simulation with %i threads and boost time of %i miliseconds.\n", cpu_threads, boost_time);

        // Launch the scheduler.
        pthread_create(&scheduler, NULL, task_scheduling, &cpu_threads);
        pthread_create(&priority_booster, NULL, priority_boosting, &cpu_threads);

        line = malloc(sizeof(char) * MAX_LINE_LEN);

        while(fgets(line, MAX_LINE_LEN, task_stream) != NULL)
        {
            args_count = space_count(line) + 1;
            
            if (args_count == 4)
            {
                // Add new tasks from a file to priority 1 queue.
                curr_task = parse_task(line);

                pthread_mutex_lock(&simulation_bookeeper_lock);
                bookeeper.total_task_num += 1;
                pthread_mutex_unlock(&simulation_bookeeper_lock);

                pthread_mutex_lock(&queues_lock[TASK_SCHEDULE_QUEUE]);
                //printf("Task %s is added to the task scheduler queue.\n", curr_task.task_name);
                enqueue(&task_queues[TASK_SCHEDULE_QUEUE], curr_task);
                pthread_mutex_unlock(&queues_lock[TASK_SCHEDULE_QUEUE]);
            }
            else if (args_count == 2)
            {
                delay_time = parse_delay_time(line);

                if (delay_time == -1)
                {
                    printf("Invalid format for DELAY task.\n");
                    exit(EXIT_FAILURE);
                }
                printf("Main thread will sleep for %i miliseconds\n", delay_time);
                
                // convert from milisecond to second.
                sleep(delay_time / MILISEC_PER_SEC);
            }
            else
            {
                printf("Invalid task argument format.\n");
                exit(EXIT_FAILURE);
            }
        }
        done_reading = 1;

        pthread_join(scheduler, NULL);
        pthread_join(priority_booster, NULL);

        printf("\nAverage turnaround time per type:\n");
        printf("    Type %i: %li usec\n", TASK_TYPE_1, bookeeper.total_turnaround_time[TASK_TYPE_1] / bookeeper.task_num[TASK_TYPE_1]);
        printf("    Type %i: %li usec\n", TASK_TYPE_2, bookeeper.total_turnaround_time[TASK_TYPE_2] / bookeeper.task_num[TASK_TYPE_2]);
        printf("    Type %i: %li usec\n", TASK_TYPE_3, bookeeper.total_turnaround_time[TASK_TYPE_3] / bookeeper.task_num[TASK_TYPE_3]);
        printf("    Type I/O: %li usec\n", bookeeper.total_turnaround_time[TASK_TYPE_IO] / bookeeper.task_num[TASK_TYPE_IO]);

        printf("\nAverage response time per type:\n");
        printf("    Type %i: %li usec\n", TASK_TYPE_1, bookeeper.total_response_time[TASK_TYPE_1] / bookeeper.task_num[TASK_TYPE_1]);
        printf("    Type %i: %li usec\n", TASK_TYPE_2, bookeeper.total_response_time[TASK_TYPE_2] / bookeeper.task_num[TASK_TYPE_2]);
        printf("    Type %i: %li usec\n", TASK_TYPE_3, bookeeper.total_response_time[TASK_TYPE_3] / bookeeper.task_num[TASK_TYPE_3]);
        printf("    Type I/O: %li usec\n", bookeeper.total_response_time[TASK_TYPE_IO] / bookeeper.task_num[TASK_TYPE_IO]);

        free(line);
        free(task_queues);
        free(queues_lock);
        free(bookeeper.total_turnaround_time);
        free(bookeeper.total_response_time);
        free(bookeeper.task_num);

        printf("\nEnd of simulation.\n");

        exit(EXIT_SUCCESS);
    }
    else
    {
        printf("Insufficient arguments are provided.\n");
        exit(EXIT_FAILURE);
    }
}
