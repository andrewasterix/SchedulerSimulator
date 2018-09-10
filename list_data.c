#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>

#include "error.c"

#ifndef LIST_DATA
#define LIST_DATA

typedef struct Instruction Instruction_t;
typedef struct Task Task_t;
typedef struct List_Instruction List_Instruction_t;
typedef struct Queue Queue_t;


struct Instruction{
    bool type_flag;
    int length;
    Instruction_t* next;
};

struct List_Instruction{
    Instruction_t* head_Instr;
};

struct Task{
    int arrival_time_task;
    int id_task;
    int ProcessState; // Stato di avanzamento del Task Da 1 a 5;
    int BLOCKING_TIME;
    List_Instruction_t* instr_List;
    Instruction_t* Program_Counter;
    Task_t* next;
};

struct Queue{
    Task_t* first_in;
};

List_Instruction_t* new_List_node (){

    List_Instruction_t* new_Instruction_list = (List_Instruction_t*)malloc(sizeof(List_Instruction_t));

    if (new_Instruction_list == NULL){
        fprintf(stderr, "%s\n", List_Instruction_Error);
    }

    new_Instruction_list->head_Instr = NULL;

    return new_Instruction_list;
}

Instruction_t* new_Instruction_node (bool type_flag, int length){

    Instruction_t* new_node_Instruction = (Instruction_t*)malloc(sizeof(Instruction_t));

    if (new_node_Instruction == NULL){
        fprintf(stderr, "%s\n", Instruction_Error);
    }

    new_node_Instruction->type_flag = type_flag;

    if(type_flag)
        new_node_Instruction->length = random() % length + 1;
    else 
        new_node_Instruction->length = length;
   
    new_node_Instruction->next = NULL;

    return new_node_Instruction;
}

Task_t* new_Task_node (int id_task, int arrival_time_task){
    
    Task_t* new_node_Task = (Task_t*)malloc(sizeof(Task_t));
    
    if (new_node_Task == NULL){
        fprintf(stderr, "%s\n", Task_Error);
        exit(-1);
    }

    new_node_Task->id_task = id_task;
    new_node_Task->arrival_time_task = arrival_time_task;
    new_node_Task->ProcessState = 1;
    new_node_Task->BLOCKING_TIME = 0;
    new_node_Task->Program_Counter = NULL;
    new_node_Task->next = NULL;
    List_Instruction_t* new_node_list = new_List_node();

    new_node_Task->instr_List = new_node_list;
    
    //free(new_node_list); VALGRIND ERROR MEMORY: SEGMENTATION FAULT CORE DUMP
    return new_node_Task;
}

Queue_t* new_Queue_node (){

    Queue_t* new_Queue = (Queue_t*)malloc(sizeof(Queue_t));

    if (new_Queue == NULL){
        fprintf(stderr, "%s\n", Queue_Error);
    }

    new_Queue ->first_in = NULL;
    
    return new_Queue;
}

void push_task (Task_t* Task, Instruction_t* Instruction){
    
    Instruction_t* Tail;
    Instruction_t* Temp = (Instruction_t*)malloc(sizeof(Instruction_t));
        
        Temp->type_flag = Instruction->type_flag;
        Temp->length = Instruction->length;
        
    
    if (Task->instr_List->head_Instr == NULL){
        
        Task->instr_List->head_Instr = Temp;
        
        Task->Program_Counter = Temp;

    }else{
        
        Tail = Task->instr_List->head_Instr;
        while(Tail->next){
            Tail = Tail->next;
        }
        Tail->next = Temp;
    }
}

void push_queue (Queue_t* Queue, Task_t* Task){
    
    Task_t* Tail;
    Task_t* Temp = (Task_t*)malloc(sizeof(Task_t));
        
        Temp->arrival_time_task = Task->arrival_time_task;
        Temp->id_task = Task->id_task;
        Temp->BLOCKING_TIME = Task->BLOCKING_TIME;
        Temp->instr_List = Task ->instr_List;
        Temp->ProcessState = Task->ProcessState;
        Temp->Program_Counter = Task->Program_Counter;
        Temp->next = NULL;

    if (Queue->first_in == NULL){
        
        Queue->first_in = Temp;
        
    }else{
        
        Tail = Queue->first_in;
        while(Tail->next!= NULL){
            Tail = Tail->next;
        }
        Tail->next = Temp;
    }
   
}

void print(Task_t* Task){

    Instruction_t* Temp;

    for(Temp = Task->instr_List->head_Instr; Temp != NULL; Temp = Temp->next)
        printf("Istr: %d, %d\n", Temp->type_flag, Temp->length);
}

void print_queue(Queue_t* Queue){
    
    Task_t* Temp;
    
    for(Temp = Queue->first_in; Temp != NULL; Temp = Temp->next){
        printf("Task: %d, %d\n", Temp->id_task, Temp->arrival_time_task);
            print(Temp);
    }
}

bool IsEmpty(Queue_t* queue){
    if (queue->first_in ==  NULL)
        return true;
    else 
        return false;
}

void pop_at_Queue(Queue_t* Queue,Task_t* Task){
  
    if (Queue->first_in == NULL){
        return;
    }

    Task_t* Temp_task_1 = (Task_t*)malloc(sizeof(Task_t));
    Task_t* Temp_task_2 = (Task_t*)malloc(sizeof(Task_t));
    
    Temp_task_1 = Queue->first_in;

    if (Temp_task_1->next == NULL){
        if (Temp_task_1->id_task == Task->id_task){
            Queue->first_in = NULL;
            free(Temp_task_1);
        }
    }else{
        if (Temp_task_1 == Task){
            Queue->first_in = Temp_task_1->next;
            free(Temp_task_1);
        }else{
            while (Temp_task_1->next != NULL){
                Temp_task_2 = Temp_task_1;
                Temp_task_1 = Temp_task_2->next;
                if (Temp_task_1 ==Task){
                    if (Temp_task_1->next == NULL){
                        Temp_task_2->next = NULL;
                    }else{
                        Temp_task_2->next = Temp_task_1->next;
                    }
                    
                    free(Temp_task_1);
                    //printf("%s\n", Node_Erase);
                    break;
                }
            }
        } 
    }
}

void pop_at_tasks(Task_t* Task, Instruction_t* Instruction){
    
    Instruction_t* Temp_Instruction_1 =(Instruction_t*)malloc(sizeof(Instruction_t));
    
    Temp_Instruction_1 = Task->instr_List->head_Instr;
    
    Instruction_t* Temp_Instruction_2 = (Instruction_t*)malloc(sizeof(Instruction_t));;
    
    if (Temp_Instruction_1->next == NULL){
        if (Temp_Instruction_1 == Instruction){
            Task->instr_List->head_Instr= NULL;
            free(Temp_Instruction_1);
        }
    }else   {
        if (Temp_Instruction_1 == Instruction){
            Task->instr_List->head_Instr = Temp_Instruction_1->next;
            free(Temp_Instruction_1);
        }else{
            while (Temp_Instruction_1->next != NULL){
                Temp_Instruction_2 = Temp_Instruction_1;
                Temp_Instruction_1 = Temp_Instruction_2->next;
                
                if (Temp_Instruction_1 == Instruction){
                    if (Temp_Instruction_1->next == NULL){
                        Temp_Instruction_2->next = NULL;
                    }else{
                        Temp_Instruction_2->next = Temp_Instruction_1->next;
                    }

                //printf("%s\n", Node_Erase);
                free(Temp_Instruction_1);
                free(Temp_Instruction_2);            
                break;
                }
            }
        }
    }
}

#endif      
