#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>

//Function
#include "list_data.c"
#include "input.c"
#include "option.c"
#include "scheduler.c"

int main(int argc, char* argv[])
{   
    pid_t Pid = fork();

    if(Pid == -1){
        fprintf(stderr, "Failed Fork\n");
        exit(-1);
    }
    
    #ifdef DEBUG
    if (Pid == 0)
        printf("Child Process\n");
    else
        printf("Parent Process\n");
    #endif 
    
    char* input_file = NULL;
    char* output_pree = NULL;
    char* output_no_pree = NULL;

    Queue_t* Queue = new_Queue_node();

    parse_option(argv, argc, &input_file, &output_pree, &output_no_pree);
    
    parsing_input(Queue, input_file); 

    if(Pid == 0){
        scheduling(Queue, output_pree, false); // PREEMPRIVE SIMULATION
    }else{
        scheduling(Queue, output_no_pree, true); // NO-PREEMPTIVE SIMULATION
    }

    if(Pid != 0){
        waitpid(Pid, NULL, 0); // WAIT UNTIL ALL PROCESS END
    }
    
    free(Queue);
    
    return 0;
}