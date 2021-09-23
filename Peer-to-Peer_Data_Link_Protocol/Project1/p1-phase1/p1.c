#include "physical.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>

struct PHYSICAL_LAYER {
    Direction direction;
    PhysicalState* physical;
};

typedef struct PHYSICAL_LAYER Physical_Layer;

struct SIGNAL {
    Physical_Layer physical_layer;
    char* filename;
};

typedef struct SIGNAL Signal;

void* sendData(void* args) {
    Signal transmit_signal = * (Signal *) args;
    Physical_Layer physical_layer = transmit_signal.physical_layer;
    FILE* input_file;
    char* msg;
    size_t line_size = 0;

    if (physical_layer.direction == L2R) {
        input_file = fopen(transmit_signal.filename, "r");
        if (input_file != NULL) {
            while (getline(&msg, &line_size, input_file) != -1) {
                pthread_mutex_lock(& physical_layer.physical->L2RLock);

                while (!physical_layer.physical->L2RReady) {
                    pthread_cond_wait(& physical_layer.physical->L2RTxSignal, & physical_layer.physical->L2RLock);
                }

                printf("Send data from the Left\n");
                printf("Message to be sent is %s with size %zu\n", msg, strlen(msg));
                transmitFrame(L2R, (unsigned char const * const) msg, (const int) strlen(msg));

                pthread_cond_signal(& physical_layer.physical->L2RRxSignal);
                pthread_mutex_unlock(& physical_layer.physical->L2RLock);

                
            }
            fclose(input_file);  
        }    
    }

    else if (physical_layer.direction == R2L) {
        input_file = fopen(transmit_signal.filename, "r");
        if (input_file != NULL) {
            while (getline(&msg, &line_size, input_file) != -1) {
                pthread_mutex_lock(& physical_layer.physical->R2LLock);

                while (!physical_layer.physical->R2LReady) {
                    pthread_cond_wait(& physical_layer.physical->R2LTxSignal, & physical_layer.physical->R2LLock);
                }

                printf("Send data from the Right\n");
                printf("Message to be sent is %s with size %zu\n", msg, strlen(msg));
                transmitFrame(R2L, (unsigned char const * const) msg, (const int) strlen(msg));

                pthread_cond_signal(& physical_layer.physical->R2LRxSignal);
                pthread_mutex_unlock(& physical_layer.physical->R2LLock);
            }
            fclose(input_file);  
        }
    }  

    return NULL;
}

void* receiveData(void* args) {
    Signal receive_signal = * (Signal *) args;
    Physical_Layer physical_layer = receive_signal.physical_layer;
    FILE* output_file;
    unsigned char data; 

    while (1) {
        output_file = fopen(receive_signal.filename, "a");
    
        if (physical_layer.direction == L2R) { 
            pthread_mutex_lock(& physical_layer.physical->L2RLock);

            while (physical_layer.physical->L2RReady) {
                printf("Waiting on the Right\n");
                pthread_cond_wait(& physical_layer.physical->L2RRxSignal, & physical_layer.physical->L2RLock);
            }
            printf("Receive data on the Righ\n");

            while ((data = receiveByte(L2R))) {
                fprintf(output_file, "%c", data);
            }     
            
            pthread_mutex_unlock(& physical_layer.physical->L2RLock);
            pthread_cond_signal(& physical_layer.physical->L2RTxSignal);
        }
        else if (physical_layer.direction == R2L) {
            pthread_mutex_lock(& physical_layer.physical->R2LLock);

            while (physical_layer.physical->R2LReady) {
                printf("Waiting on the Left\n");
                pthread_cond_wait(& physical_layer.physical->R2LRxSignal, & physical_layer.physical->R2LLock);
            }

            printf("Receive data on the Left\n");
            while ((data = receiveByte(R2L))) {
                fprintf(output_file, "%c", data);
            }   
            
            pthread_mutex_unlock(& physical_layer.physical->R2LLock);
            pthread_cond_signal(& physical_layer.physical->R2LTxSignal);
        }
    fclose(output_file);
    }
    return NULL;

}

int main(int argc, char *argv[]) {
    if (argc == 3) {
        char* input_file = argv[1];
        char* output_file = argv[2];
        pthread_t left, right;
        PhysicalState* physical_ptr = initPhysical(); 

        Physical_Layer args_L2R = {L2R, physical_ptr};
        Signal trasmit_signal_L2R = {args_L2R, input_file};
        Signal receive_signal_L2R = {args_L2R, output_file};
        

        Physical_Layer args_R2L = {R2L, physical_ptr};
        Signal trasmit_signal_R2L = {args_R2L, input_file};
        Signal receive_signal_R2L = {args_R2L, output_file};

        pthread_create(&left, NULL, sendData, (void *) &trasmit_signal_L2R);
        pthread_create(&right, NULL, sendData, (void *) &trasmit_signal_R2L);
        pthread_create(&right, NULL, receiveData, (void *) &receive_signal_L2R);
        pthread_create(&left, NULL, receiveData, (void *) &receive_signal_R2L);
        
        pthread_exit(NULL);
        pthread_join(left, NULL);
        pthread_join(right, NULL);
           
        cleanPhysical();
    }
    return 0;
}