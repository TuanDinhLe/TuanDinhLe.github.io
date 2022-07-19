#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#define MAX_LEN 100

char *main_buffer;
char *lower_buffer;
int lower_ready = 0;
int upper_ready = 0;
int done = 0;
pthread_mutex_t main_buffer_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lower_buffer_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t main_and_lower;
pthread_cond_t lower_and_upper;

void *lower_case_processing(void *args);
void *upper_case_processing(void *args);

void *upper_case_processing(void *args)
{
    (void) args;

    while (1)
    {
        pthread_mutex_lock(&lower_buffer_lock);

        // If upper_case thread start first, it will yield the lock
        // and wait for signal to wake up.
        while(!upper_ready)
        {
            pthread_cond_wait(&lower_and_upper, &lower_buffer_lock);

            // Terminate thread.
            if (done == 1)
            {
                pthread_mutex_unlock(&lower_buffer_lock);
                return NULL;
            }
        }

        upper_ready = 0;

        printf("Upper case characters are: ");
        for (int i = 0; i < (int) strlen(lower_buffer); i++)
        {
            if (isupper(lower_buffer[i]))
            {
                printf("%c ", lower_buffer[i]);
            }
        }
        printf("\n");
        pthread_mutex_unlock(&lower_buffer_lock);
    }
    return NULL;
}

void *lower_case_processing(void *args)
{
    (void) args;

    while (1)
    {
        pthread_mutex_lock(&main_buffer_lock);

        // If lower_case thread start first, it will yield the lock
        // and wait for signal to wake up.
        while (!lower_ready)
        {
            pthread_cond_wait(&main_and_lower, &main_buffer_lock);

            // Terminate thread and signal to upper_case thread to
            // terminate as well.
            if (done == 1)
            {
                upper_ready = 1;
                pthread_cond_signal(&lower_and_upper);
                pthread_mutex_unlock(&main_buffer_lock);
                return NULL;
            }
        }
        lower_ready = 0;

        printf("Lower case characters are: ");
        for (int i = 0; i < (int) strlen(main_buffer); i++)
        {
            if (islower(main_buffer[i]))
            {
                printf("%c ", main_buffer[i]);
            }
        }
        printf("\n");

        pthread_mutex_lock(&lower_buffer_lock);
        strcpy(lower_buffer, main_buffer);
        upper_ready = 1;
        pthread_cond_signal(&lower_and_upper);
        pthread_mutex_unlock(&lower_buffer_lock);

        pthread_mutex_unlock(&main_buffer_lock);
    }
    return NULL;
}

int main( void )
{
    pthread_t lower_case, upper_case;
    char *line = malloc(sizeof(char) * MAX_LEN);;
    main_buffer = malloc(sizeof(char) * MAX_LEN);
    lower_buffer = malloc(sizeof(char) * MAX_LEN);

    pthread_cond_init(&main_and_lower, NULL);
    pthread_cond_init(&lower_and_upper, NULL);

    pthread_create(&lower_case, NULL, lower_case_processing, NULL);
    pthread_create(&upper_case, NULL, upper_case_processing, NULL);

    while (fgets(line, MAX_LEN, stdin) != NULL) 
    {
        pthread_mutex_lock(&main_buffer_lock);

        strcpy(main_buffer, line);

        // Tell the lower_case thread to proceed after wait(). 
        lower_ready = 1;

        pthread_cond_signal(&main_and_lower);
        pthread_mutex_unlock(&main_buffer_lock);
    }
    // condition for lower_case and upper_case thread to exit.
    done = 1;

    lower_ready = 1;
    pthread_cond_signal(&main_and_lower);

    pthread_join(lower_case, NULL);
    pthread_join(upper_case, NULL);

    free(line);
    free(main_buffer);
    free(lower_buffer);

    printf("End of file\n");

    return EXIT_SUCCESS;
}

