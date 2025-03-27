/* Include files */
#include "fcfs.h"

// keep the record of time quantum if put back to the head
/*
q: destination queue;
p: source queue;
q_priority: expected priority of q if q is valid;
time: the time when the function is queued;
*/
PcbPtr enqueueList(PcbPtr q, PcbPtr p, int q_priority, int time)
{
    PcbPtr h = q;   // record the head of the destination queue
    PcbPtr m = p;   // keep a copy of the source queue for traversal

    if(!p) return q;
     
    if (q) {    // if destination is not empty or NULL
        while (q->next) q = q->next;
        p->quantum_record = 0;
        q->next = p;
        while (m){  //set the priority of the enqueued process and its previous enqueued time
            m->priority = q_priority;
            m->previous_enq = time;
            m = m->next;
        }
        return h;
    }
    // if destination is NULL or empty
    while (m){  //set the priority of the enqueued process and its previous enqueued time
        m->priority = q_priority;
        m->previous_enq = time;
        m = m->next;
    }
    return p;
}

int main (int argc, char *argv[])
{
    /*** Main function variable declarations ***/

    FILE * input_list_stream = NULL;
    PcbPtr dispatch_queue = NULL;
    PcbPtr queue_0 = NULL;
    PcbPtr queue_1 = NULL;
    PcbPtr queue_2 = NULL;
    PcbPtr current_process = NULL;
    PcbPtr process = NULL;
    int timer = 0;
    int quantum_counter = 0;    // timer for current running process to check if quantum exceeded

    int turnaround_time, response_time;
    double av_turnaround_time = 0.0, av_wait_time = 0.0, av_response_time = 0.0;
    int n = 0;

    int t0, t1, t2, w = 0;

// ask for inputs
    printf("Enter t0: ");
    scanf("%d", &t0);
    printf("Enter t1: ");
    scanf("%d", &t1);
    printf("Enter t2: ");
    scanf("%d", &t2);
    printf("Enter W: ");
    scanf("%d", &w);

//  Populate the dispatch queue

    if (argc <= 0)
    {
        fprintf(stderr, "FATAL: Bad arguments array\n");
        exit(EXIT_FAILURE);
    }
    else if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <TESTFILE>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (!(input_list_stream = fopen(argv[1], "r")))
    {
        fprintf(stderr, "ERROR: Could not open \"%s\"\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    while (!feof(input_list_stream)) {  // put processes into dispatch_queue
        process = createnullPcb();
        if (fscanf(input_list_stream,"%d, %d, %d",
             &(process->arrival_time), 
             &(process->service_time),
             &(process->priority)) != 3) {
            free(process);
            continue;
        }
        process->remaining_cpu_time = process->service_time;
        process->status = PCB_INITIALIZED;
        dispatch_queue = enqPcb(dispatch_queue, process);
        n++;
    }
    fclose(input_list_stream);
//  Whenever there is a running process or the queues are not empty:
    while (current_process || queue_0 || queue_1 || queue_2 || dispatch_queue)
    {
        while(dispatch_queue && dispatch_queue->arrival_time <= timer){
            // decide the queue to enqueue to and change process status to ready;
            dispatch_queue->status = PCB_READY;
            dispatch_queue->previous_enq = timer;
            switch (dispatch_queue->priority)
            {
            case 0:
                queue_0 = enqPcb(queue_0, deqPcb(&dispatch_queue));
                break;
            case 1:
                queue_1 = enqPcb(queue_1, deqPcb(&dispatch_queue));
                break;
            case 2:
                queue_2 = enqPcb(queue_2, deqPcb(&dispatch_queue));
                break;

            default:
                break;
            }
        }

        // decrement the remaining cpu time of the current running process and terminate if time runs out
        if(current_process){
            current_process->remaining_cpu_time -= 1;
            if (current_process->remaining_cpu_time <= 0)
            {
//              Terminate the process;
                terminatePcb(current_process);
                quantum_counter = 0;

//		        calculate and acumulate turnaround time and wait time
                turnaround_time = timer - current_process->arrival_time;
                av_turnaround_time += turnaround_time;
                av_wait_time += turnaround_time - current_process->service_time;
                
//              Deallocate the PCB (process control block)'s memory
                free(current_process);
                current_process = NULL;
            }
        }

//      case current priority = 0;
        if (current_process && current_process->priority == 0)
        {
            if(quantum_counter >= t0){  // if runs out of time quantum
                PcbPtr suspended = suspendPcb(current_process);
                suspended->priority = 1;
                suspended->previous_enq = timer;
                queue_1 = enqPcb(queue_1, suspended);   //downgrade
                current_process = NULL;
                quantum_counter = 0;
            }

//      case current priority = 1;
        }else if(current_process && current_process->priority == 1){
           
            if(quantum_counter >= t1){  // if runs out of time quantum
                PcbPtr suspended = suspendPcb(current_process);
                suspended->quantum_record = 0;
                suspended->priority = 2;
                suspended->previous_enq = timer;
                queue_2 = enqPcb(queue_2, suspended);       //downgrade
                current_process = NULL;
                quantum_counter = 0;
            }else if(queue_0){  // if there is a higher priority process ready to run
                PcbPtr suspended = suspendPcb(current_process);
                suspended->quantum_record = quantum_counter;    //freezing
                suspended->next = queue_1;
                queue_1 = suspended;
                quantum_counter = 0;
                current_process = NULL;
            }
            
//      case current priority = 2;
        }else if(current_process && current_process->priority == 2){
            if(quantum_counter >= t2){  // if runs out of time quantum
                PcbPtr suspended = suspendPcb(current_process);
                suspended->quantum_record = 0;
                suspended->previous_enq = timer;
                queue_2 = enqPcb(queue_2, suspended);       // put to end
                current_process = NULL;
                quantum_counter = 0;
            }else if(queue_0 || queue_1){  // if there is a higher priority process ready to run
                PcbPtr suspended = suspendPcb(current_process);
                suspended->quantum_record = quantum_counter;    // freezing
                suspended->next = queue_2;
                queue_2 = suspended;
                quantum_counter = 0;
                current_process = NULL;
            }
        }

//      fairness checking
        if(queue_1){    // if queue 1 not empty
            int q1time = timer - queue_1->previous_enq - queue_1->quantum_record;   // calculate the time passed since last enqueued
            if(q1time >= w){    // if time exceeds w
                queue_0 = enqueueList(queue_0, queue_1, 0, timer);
                queue_0 = enqueueList(queue_0, queue_2, 0, timer);
                queue_1 = NULL;
                queue_2 = NULL;
            }
        }
        if(queue_2){    // if queue 2 not empty
            int q2time = timer - queue_2->previous_enq - queue_2->quantum_record;   // calculate the time passed since last enqueued
            if(q2time >= w){    // if time exceeds w
                queue_0 = enqueueList(queue_0, queue_2, 0, timer);
                queue_2 = NULL;
            }
        }

//      second suspention checking in case promotion occurs
        if(current_process){
            if(current_process->priority == 1){
                if(queue_0 && current_process->remaining_cpu_time > 0){
                    PcbPtr suspended = suspendPcb(current_process);
                    suspended->quantum_record = quantum_counter;    //freezing
                    suspended->next = queue_1;
                    queue_1 = suspended;
                    quantum_counter = 0;    //reset the timer for 
                    current_process = NULL;
                }
            }else if(current_process->priority == 2){
                if((queue_0 || queue_1) && current_process->remaining_cpu_time > 0){
                    PcbPtr suspended = suspendPcb(current_process);
                    suspended->quantum_record = quantum_counter;    //freezing
                    suspended->next = queue_2;
                    queue_2 = suspended;
                    quantum_counter = 0;
                    current_process = NULL;
                }
            }
        }


//      If there is no running process and there is a process in q0 ready to run:
        if (!current_process && queue_0)
        {
//          Dequeue the process at the head of the queue, set it as currently running and start it
            current_process = deqPcb(&queue_0);
            quantum_counter = 0;
            if(current_process->status == PCB_READY){
                response_time = timer - current_process->arrival_time;
                av_response_time += response_time;
            }
            startPcb(current_process);

//      If there is no running process and there is a process in q1 ready to run:
        }else if (!current_process && queue_1)
        {
//          Dequeue the process at the head of the queue, set it as currently running and start it
            current_process = deqPcb(&queue_1);
            quantum_counter = current_process->quantum_record;  // retrieve amount of time run
            if(current_process->status == PCB_READY){
                response_time = timer - current_process->arrival_time;
                av_response_time += response_time;
            }
            startPcb(current_process);

//      If there is no running process and there is a process in q2 ready to run:
        }else if (!current_process && queue_2)
        {
//          Dequeue the process at the head of the queue, set it as currently running and start it
            current_process = deqPcb(&queue_2);
            quantum_counter = current_process->quantum_record;  // retrieve amount of time run
            if(current_process->status == PCB_READY){
                response_time = timer - current_process->arrival_time;
                av_response_time += response_time;
            }
            startPcb(current_process);
        }
        
        // increment the timer if the program has not ended yet;
        if(current_process || queue_0 || queue_1 || queue_2 || dispatch_queue){
            timer += 1;
            quantum_counter += 1;
            sleep(1);
        }
        
    }

//  print out average turnaround time and average wait time
    av_turnaround_time = av_turnaround_time / n;
    av_wait_time = av_wait_time / n;
    av_response_time = av_response_time / n;
    printf("Average turnaround time = %f\n", av_turnaround_time);
    printf("Average wait time = %f\n", av_wait_time);
    printf("Average response time = %f\n", av_response_time);
    
//  Terminate the RR dispatcher
    exit(EXIT_SUCCESS);
}