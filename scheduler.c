#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "list_data.c"
#include "error.c"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

typedef struct Scheduler_Info Scheduler_Info_t;

struct Scheduler_Info{
    bool Sched_type; // 0 Preemptive - 1 NO_Preemptive
    int core_number;
    char* output_file;
    Queue_t* Task_list;
};

Scheduler_Info_t* new_scheduler_info(Queue_t* Queue, char* file_output, bool type, int core){

    Scheduler_Info_t* new_Scheduler = (Scheduler_Info_t*)malloc(sizeof(Scheduler_Info_t));

    new_Scheduler->core_number = core;
    new_Scheduler->output_file = file_output;
    new_Scheduler->Task_list = Queue;
    new_Scheduler->Sched_type = type;

    return new_Scheduler;    
}

void make_Tasks_ready(Queue_t* ready_tasks,Task_t* Task) {
    
    push_queue(ready_tasks,Task);

}

void make_Tasks_Blocked(Queue_t* Blocked, Task_t* Task){
    
    push_queue(Blocked,Task);
}

void change_state(Task_t* Task, int core, int clock, char* output_file){
    
    FILE* output_file_w = fopen(output_file, "a");

    switch(Task->ProcessState){
        case 1: // NEW
            fprintf(output_file_w, "core%d, %d, %d, %s\n", core, clock, Task->id_task, "new");
        break;
        case 2: // READY
            fprintf(output_file_w, "core%d, %d, %d, %s\n", core, clock, Task->id_task, "ready");
        break;
        case 3: // RUNNING
            fprintf(output_file_w, "core%d, %d, %d, %s\n", core, clock, Task->id_task, "runing");
        break;
        case 4: // BLOCKED
            fprintf(output_file_w, "core%d, %d, %d, %s\n", core, clock, Task->id_task, "blocked");
        break;
        case 5: // EXIT
            fprintf(output_file_w, "core%d, %d, %d, %s\n", core, clock, Task->id_task, "exit");
        break;
        default:
        break;
    }
    fclose(output_file_w);
}

unsigned int calculate_quantum(Queue_t* Queue){

    unsigned int quantum = 0;
    unsigned int somma = 0;
    int number_instructions = 0;

    Task_t* Temp_Task;
    Instruction_t* Temp_Istr;
    
    for(Temp_Task = Queue->first_in; Temp_Task != NULL; Temp_Task = Temp_Task->next){
        for(Temp_Istr = Temp_Task->instr_List->head_Instr; Temp_Istr != NULL; Temp_Istr = Temp_Istr->next){
            somma += Temp_Istr->length;
            number_instructions++;
        }
    }

    if (number_instructions == 0){
        fprintf(stderr, "%s\n", Queue_Empty);
        return 0;
    }

    quantum = ceil(somma*0.8/number_instructions);
    return quantum;
}

void* schedule(void* Sched_Info){

    Scheduler_Info_t* Sched_info = new_scheduler_info(((Scheduler_Info_t*) Sched_Info)->Task_list, 
        ((Scheduler_Info_t*) Sched_Info)->output_file, ((Scheduler_Info_t*) Sched_Info)->Sched_type, 
            ((Scheduler_Info_t*) Sched_Info)->core_number);
    
    
    unsigned int clock = 0;
   
    bool doThing = false;
    
    pthread_mutex_lock(&mutex);
    while(Sched_info->Task_list->first_in->arrival_time_task != clock){ // Until clock != Task Arrival time
        clock++;
    } 
    pthread_mutex_unlock(&mutex);
    
    if(Sched_info->Sched_type){ //FCFS
        
        Queue_t* blocked_tasks = new_Queue_node();
        
        while(!doThing){
            Queue_t* ready_tasks = new_Queue_node();
    
            pthread_mutex_lock(&mutex);
            if(!IsEmpty(Sched_info->Task_list)){
                
                push_queue(ready_tasks,Sched_info->Task_list->first_in); // Sched_queue -> Ready_queue
                change_state(ready_tasks->first_in,Sched_info->core_number,clock,Sched_info->output_file);// NEW
                
                ready_tasks->first_in->ProcessState = 2;
                change_state(ready_tasks->first_in,Sched_info->core_number,clock,Sched_info->output_file);// in Ready_queue: NEW->READY
           
                Sched_info->Task_list->first_in = Sched_info->Task_list->first_in->next; // Prepare new Task To execute
                  
            }else if(!IsEmpty(blocked_tasks)){
                
                if(blocked_tasks->first_in->BLOCKING_TIME < clock || blocked_tasks->first_in->BLOCKING_TIME == clock){
                    push_queue(ready_tasks,blocked_tasks->first_in);
                    pop_at_Queue(blocked_tasks, blocked_tasks->first_in);
                           
                    ready_tasks->first_in->ProcessState = 2;
                    ready_tasks->first_in->instr_List->head_Instr->type_flag = false;
                    change_state(ready_tasks->first_in,Sched_info->core_number,clock,Sched_info->output_file);
                }
            }
            
            pthread_mutex_unlock(&mutex);
           

            if(!IsEmpty(ready_tasks)){

                pthread_mutex_lock(&mutex);       
                Instruction_t* tmp = ready_tasks->first_in->instr_List->head_Instr;
                pthread_mutex_unlock(&mutex);

                while (tmp != NULL){

                    if (!tmp->type_flag){ //NO BLOCKING INSTRUCTION
                       
                        ready_tasks->first_in->ProcessState = 3;
                        change_state(ready_tasks->first_in, Sched_info->core_number, clock, Sched_info->output_file); // READY->RUNNING
                    
                        int EXEC_TIME = (tmp->length + clock);

                        while(clock != EXEC_TIME){
                            clock++;
                        }   
                    
                        if (tmp->next == NULL){ // NO BLOCKING INSTRUCTION - END 
                            ready_tasks->first_in->ProcessState = 5;
                            change_state(ready_tasks->first_in, Sched_info->core_number, clock, Sched_info->output_file); // RUNNING->EXIT
                    }   

                    }else{ //BLOCKING INSTRUCTION
                        ready_tasks->first_in->ProcessState = 4;
                        change_state(ready_tasks->first_in, Sched_info->core_number, clock, Sched_info->output_file); // READY->BLOCKED
                        ready_tasks->first_in->BLOCKING_TIME = clock + ready_tasks->first_in->instr_List->head_Instr->length;
                        make_Tasks_Blocked(blocked_tasks, ready_tasks->first_in); // INSERT IN BLOCKED QUEUE
                        
                        break;
                    }

                    pop_at_tasks(ready_tasks->first_in,ready_tasks->first_in->instr_List->head_Instr);
                    
                    tmp = tmp->next;         
                }
                
            }
                
            clock++;
                       
            pthread_mutex_lock(&mutex);
            if(IsEmpty(ready_tasks) && IsEmpty(blocked_tasks) && IsEmpty(Sched_info->Task_list)){
                doThing = true;
            }
            pthread_mutex_unlock(&mutex);
        }
    }else if(!Sched_info->Sched_type){ //RR

        int quantum =  calculate_quantum(Sched_info->Task_list);

        Queue_t* ready_queue = new_Queue_node();
        Queue_t* blocked_queue = new_Queue_node();
        
        Queue_t* TMP = new_Queue_node();

        while(!doThing){
            
            Task_t* Temp = NULL;

            pthread_mutex_lock(&mutex);
            if(!IsEmpty(Sched_info->Task_list)){
                
                push_queue(ready_queue, Sched_info->Task_list->first_in);

                if(ready_queue->first_in->ProcessState == 1){ // NEW
                    change_state(ready_queue->first_in, Sched_info->core_number, clock, Sched_info->output_file);
                    ready_queue->first_in->ProcessState = 2; // NEW->READY
                }

                Sched_info->Task_list->first_in = Sched_info->Task_list->first_in->next;
                
            }else if(!IsEmpty(blocked_queue)){
                
                if(blocked_queue->first_in->BLOCKING_TIME <= clock){
                    
                    blocked_queue->first_in->ProcessState = 2;
                    blocked_queue->first_in->instr_List->head_Instr->type_flag = false;
                
                    push_queue(ready_queue, blocked_queue->first_in);
                    pop_at_Queue(blocked_queue, blocked_queue->first_in);
                    
                }
                
            }
            pthread_mutex_unlock(&mutex);

            pthread_mutex_lock(&mutex2);
            if(!IsEmpty(TMP)){
                
                push_queue(ready_queue, TMP->first_in);
                TMP->first_in = NULL;
               
            }
            pthread_mutex_unlock(&mutex2);
         
            if(!IsEmpty(ready_queue)){
                
                pthread_mutex_lock(&mutex);
                if(ready_queue->first_in->ProcessState == 1){ // NEW
                    change_state(ready_queue->first_in, Sched_info->core_number, clock, Sched_info->output_file);
                }

                ready_queue->first_in->ProcessState = 2; // NEW->READY
                change_state(ready_queue->first_in, Sched_info->core_number, clock, Sched_info->output_file);
                
                Instruction_t* tmp = ready_queue->first_in->instr_List->head_Instr;
                pthread_mutex_unlock(&mutex);

                if(!tmp->type_flag){
                    ready_queue->first_in->ProcessState = 3;
                    change_state(ready_queue->first_in,Sched_info->core_number,clock,Sched_info->output_file);

                    int EXEC_TIME = tmp->length + clock;
                    int EXEC_QUANTUM = clock + quantum;

                    while(clock != EXEC_TIME){
                        clock++;

                        if(clock == EXEC_QUANTUM){
                            ready_queue->first_in->instr_List->head_Instr->length -= quantum;
                            break;
                        }
                    }

                    if(EXEC_TIME <= EXEC_QUANTUM){
                        pop_at_tasks(ready_queue->first_in, ready_queue->first_in->instr_List->head_Instr);      
                        if(tmp->next == NULL){
                            ready_queue->first_in->ProcessState = 5;
                            change_state(ready_queue->first_in,Sched_info->core_number,clock,Sched_info->output_file);
                            pop_at_Queue(ready_queue, ready_queue->first_in);
                        }
                    }

                    Temp = ready_queue->first_in;
                    if(Temp != NULL){
                        int i = 1;
                        while(i == 1){
                            push_queue(TMP, Temp);
                            TMP->first_in->ProcessState = 2;
                            i++;
                        }
                    }
                    pop_at_Queue(ready_queue, ready_queue->first_in);  
                                        
                    
                }else{
                    ready_queue->first_in->ProcessState = 4;
                    change_state(ready_queue->first_in,Sched_info->core_number,clock,Sched_info->output_file);

                    ready_queue->first_in->BLOCKING_TIME = clock + tmp->length;
                    
                    push_queue(blocked_queue, ready_queue->first_in);
                    pop_at_Queue(ready_queue, ready_queue->first_in);
                    
                }
                
            }
           
            clock++;

            pthread_mutex_lock(&mutex);        
            if(IsEmpty(Sched_info->Task_list) && IsEmpty(blocked_queue) && IsEmpty(ready_queue) && IsEmpty(TMP)){
                doThing = true;
            }
            pthread_mutex_unlock(&mutex);
        }
       
    }else{
        fprintf(stderr, "%s\n", Generic_Error);
        exit(-1);
    }

    return NULL;
}

void scheduling(Queue_t* Queue, char* file_name, bool type_sched){

    int core = 2;

    pthread_t thread[core];
    
    Scheduler_Info_t Scheduler[core];

    for(int i = 0; i < core; i++){
        Scheduler[i].output_file = file_name;
        Scheduler[i].Sched_type = type_sched;
        Scheduler[i].Task_list = Queue;
        Scheduler[i].core_number = i;
      
        
        if (pthread_create(&thread[i], NULL, &schedule, &Scheduler[i]) != 0) {
            fprintf(stderr,"%s\n", Thread_Error);
            exit(1);
        }
    }
    
    for(int i = 0; i < core; i++){
        pthread_join(thread[i], NULL);
    }

}