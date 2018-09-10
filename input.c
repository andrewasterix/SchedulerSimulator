#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "list_data.c"
#include "error.c"

void parsing_input(Queue_t* Queue, char* file_name){

    FILE* input_file = fopen(file_name, "r");

    if(input_file == NULL){
        perror(loading_error);
        exit(-1);
    } 

    char type;
    int element_1, element_2;
    Task_t* Task = NULL;

    while(fscanf(input_file, "%c, %d, %d\n", &type, &element_1, &element_2) != EOF){
        switch(type){
            case 't':
                Task = new_Task_node(element_1, element_2);
                push_queue(Queue, Task);
                break;
            case 'i':
                if(Task->Program_Counter == NULL){
                    
                    Instruction_t* head_Instruction;

                    if(element_1 == 0)
                        head_Instruction = new_Instruction_node(false, element_2);
                    else
                        head_Instruction = new_Instruction_node(true, element_2);

                    push_task(Task, head_Instruction);

                }else{
                    Instruction_t* next_Instruction;
                    if(element_1 == 0)
                        next_Instruction = new_Instruction_node(false, element_2);
                    else
                        next_Instruction = new_Instruction_node(true, element_2);
                    
                    push_task(Task, next_Instruction);
                }
                break;
            default:
                perror(reading_error);
                exit(1);
        }
    }
    fclose(input_file);
}