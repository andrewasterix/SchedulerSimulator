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


void print_file(Task_t* Task, FILE* output_file){

    Instruction_t* Temp;

    //FILE* output_file_w = fopen(output_file, "w");
    for(Temp = Task->instr_List->head_Instr; Temp != NULL; Temp = Temp->next)
        fprintf(output_file, "Istr: %d, %d\n", Temp->type_flag, Temp->length);
}

void print_queue_file(Queue_t* Queue, char* output_file){
    
    Task_t* Temp;

    FILE* output_file_w = fopen(output_file, "w");

    for(Temp = Queue->first_in; Temp != NULL; Temp = Temp->next){
        fprintf(output_file_w, "Task: %d, %d\n", Temp->id_task, Temp->arrival_time_task);
        print_file(Temp, output_file_w);
    }
}

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
        scheduling(Queue, output_pree, false); 
    }else{
        scheduling(Queue, output_no_pree, true);
    }

    if(Pid != 0){
        waitpid(Pid, NULL, 0);
    }
    
    free(Queue);
    
    return 0;
}