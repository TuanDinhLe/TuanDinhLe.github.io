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

                int i = 0;
                int len = 0;
                while (msg[i++] != '\0') {
                    len+=1;
                }
                msg[len-1] = '\0';

                while (!physical_layer.physical->L2RReady) {
                    pthread_cond_wait(& physical_layer.physical->L2RTxSignal, & physical_layer.physical->L2RLock);
                }

                printf("Send data from the Left to the Right\n");
                printf("Message to be sent is %s with size %d\n", msg, len);
                transmitFrame(L2R, (unsigned char const * const) msg, (const int) len);

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

                int i = 0;
                int len = 0;
                while (msg[i++] != '\0') {
                    len+=1;
                }
                msg[len-1] = '\0';

                while (!physical_layer.physical->R2LReady) {
                    pthread_cond_wait(& physical_layer.physical->R2LTxSignal, & physical_layer.physical->R2LLock);
                }

                printf("Send data from the Right to the Left\n");
                printf("Message to be sent is %s with size %d\n", msg, len);
                transmitFrame(R2L, (unsigned char const * const) msg, (const int) len);

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

    if (physical_layer.direction == L2R) {
        while (1) {
            output_file = fopen(receive_signal.filename, "a");
            pthread_mutex_lock(& physical_layer.physical->L2RLock);

            while (physical_layer.physical->L2RReady) {
                printf("Waiting on the Right\n");
                pthread_cond_wait(& physical_layer.physical->L2RRxSignal, & physical_layer.physical->L2RLock);
            }
            printf("Incoming data from the Left to the Right\n");

            while ((data = receiveByte(L2R))) {
                fprintf(output_file, "%c", data);
            }     
            fprintf(output_file, "\n");
            
            pthread_mutex_unlock(& physical_layer.physical->L2RLock);
            pthread_cond_signal(& physical_layer.physical->L2RTxSignal);
            fclose(output_file);
        }
    }
    else if (physical_layer.direction == R2L) {
        while (1) {
            output_file = fopen(receive_signal.filename, "a");
            pthread_mutex_lock(& physical_layer.physical->R2LLock);

            while (physical_layer.physical->R2LReady) {
                printf("Waiting on the Left\n");
                pthread_cond_wait(& physical_layer.physical->R2LRxSignal, & physical_layer.physical->R2LLock);
            }

            printf("Incoming data from the Right to the Left\n");
            while ((data = receiveByte(R2L))) {
                fprintf(output_file, "%c", data);
            }   
            fprintf(output_file, "\n");
            
            pthread_mutex_unlock(& physical_layer.physical->R2LLock);
            pthread_cond_signal(& physical_layer.physical->R2LTxSignal);
            fclose(output_file);
        }
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
        pthread_create(&right, NULL, receiveData, (void *) &receive_signal_L2R);
        pthread_create(&right, NULL, sendData, (void *) &trasmit_signal_R2L);
        pthread_create(&left, NULL, receiveData, (void *) &receive_signal_R2L);
        
        
        pthread_exit(NULL);
        cleanPhysical();
    }
    return 0;
}