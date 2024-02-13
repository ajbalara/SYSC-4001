#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <sys/time.h>
#include <sys/types.h>
#include <string.h>
#include <semaphore.h>
#include <time.h>

#define NUM_CONSUMER_THREADS 4
#define BONUS 5
#define min(X, Y) ((X) < (Y) ? (X) : (Y))
#define max(X, Y) ((X) > (Y) ? (X) : (Y))
#define min4(X, Y, Z, A) (min(X, Y) < min(Z, A) ? min(X, Y) : min(Z,A))
#define ITERATION_DIVIDER 1

#define FILENAME "values.csv"
#define NUM_PROCESSES 20

#define MAX_SLEEP_AVG 10 * 1000

// Enum for scheduling types
enum SchedulingType {
    NORMAL,
    RR,
    FIFO
};

// Define the struct to hold the data read from csv
struct ProcessData {
    int pid;
    enum SchedulingType policy;
    int priority;
    int execution_time;
    int cpu_affinity;
};

// Define the struct to hold all the task information
struct task_struct {
	int pid;
	enum SchedulingType policy;
	int static_priority; 
	int dynamic_priority; 
	int remain_time;
    	int time_slice;
    	int accu_time_slice;
    	int cpu_affinity;
    	int sleep_avg;
    	struct timeval last_time_run;
};

// Function to convert a string to the corresponding enum value
enum SchedulingType get_scheduling_type(const char *policy_str) {
    if (strcmp(policy_str, "NORMAL") == 0) {
        return NORMAL;
    } else if (strcmp(policy_str, "RR") == 0) {
        return RR;
    } else if (strcmp(policy_str, "FIFO") == 0) {
        return FIFO;
    } else {
        // Default to NORMAL if the string doesn't match any known type
        return NORMAL;
    }
}

// Function to parse a CSV line and store the data in the struct
int parse_csv_line(FILE *file, struct ProcessData *process_data) {
    char policy_str[7]; // Temporary buffer for the policy string

    int result = fscanf(file, "%d,%6[^,],%d,%d,%d",
                        &process_data->pid,
                        policy_str,
                        &process_data->priority,
                        &process_data->execution_time,
                        &process_data->cpu_affinity);

    if (result == 0) {
        // fscanf returns 0 when it fails to match any items
        return 0;
    } else if (result != 5) {
        // Parsing error
        return -1;
    }

    // Convert the policy string to the corresponding enum value
    process_data->policy = get_scheduling_type(policy_str);

    return 1; // Success
}

void *thread_function_p(void *arg);
void *thread_function_c(void *arg);

// all the CPU queues with their sizes defined
struct task_struct c0rq0[NUM_PROCESSES]; int c0rq0_size = 0;
struct task_struct c0rq1[NUM_PROCESSES]; int c0rq1_size = 0;
struct task_struct c0rq2[NUM_PROCESSES]; int c0rq2_size = 0;

struct task_struct c1rq0[NUM_PROCESSES]; int c1rq0_size = 0;
struct task_struct c1rq1[NUM_PROCESSES]; int c1rq1_size = 0;
struct task_struct c1rq2[NUM_PROCESSES]; int c1rq2_size = 0;

struct task_struct c2rq0[NUM_PROCESSES]; int c2rq0_size = 0;
struct task_struct c2rq1[NUM_PROCESSES]; int c2rq1_size = 0;
struct task_struct c2rq2[NUM_PROCESSES]; int c2rq2_size = 0;

struct task_struct c3rq0[NUM_PROCESSES]; int c3rq0_size = 0;
struct task_struct c3rq1[NUM_PROCESSES]; int c3rq1_size = 0;
struct task_struct c3rq2[NUM_PROCESSES]; int c3rq2_size = 0;

int producer_still_going = 1;
pthread_mutex_t work_mutex;

int main() {
    int res;
    int thread_num;
    pthread_t p_thread;
    pthread_t a_thread[NUM_CONSUMER_THREADS];
    void *thread_result;
   
    res = pthread_mutex_init(&work_mutex, NULL);
    
    res = pthread_create(&p_thread, NULL, thread_function_p, NULL);
    
	
    for(thread_num = 0; thread_num < NUM_CONSUMER_THREADS; thread_num++) {
        res = pthread_create(&(a_thread[thread_num]), NULL, thread_function_c, (void *)&thread_num);
        sleep(1);
    }
    
    printf("Waiting for consumer threads to finish...\n");
    // call pthread_join () here with a loop
    res = pthread_join(p_thread, &thread_result);
    for(int lots_of_threads = NUM_CONSUMER_THREADS - 1; lots_of_threads >= 0; lots_of_threads--) {
        res = pthread_join(a_thread[lots_of_threads], &thread_result);
    }
    
    printf("All done\n");
    
    pthread_mutex_destroy(&work_mutex);

    exit(EXIT_SUCCESS);
}

void *thread_function_c(void *arg) {
    int my_number = *(int *)arg;
    printf("\nCPU %d\n", my_number);  
    while (producer_still_going){}
    struct task_struct generated_info;
    
    struct timeval start, end;
    
    struct task_struct *rq0, *rq1, *rq2; // pointers to access to queues
    int *rq0_size, *rq1_size, *rq2_size; // pointers to access size of queues
    
    // based on which cpu number it is, this will set the pointers to point to the correct queues so that they can be accessed
    switch(my_number){
    	case 0:
    		rq0 = c0rq0;
    		rq1 = c0rq1;
    		rq2 = c0rq2;
    		
    		rq0_size = &c0rq0_size;
    		rq1_size = &c0rq1_size;
    		rq2_size = &c0rq2_size;
    		break;
    	case 1:
    		rq0 = c1rq0;
    		rq1 = c1rq1;
    		rq2 = c1rq2;
    		
    		rq0_size = &c1rq0_size;
    		rq1_size = &c1rq1_size;
    		rq2_size = &c1rq2_size;
    		break;
    	case 2:
    		rq0 = c2rq0;
    		rq1 = c2rq1;
    		rq2 = c2rq2;
    		
    		rq0_size = &c2rq0_size;
    		rq1_size = &c2rq1_size;
    		rq2_size = &c2rq2_size;
    		break;
    	case 3:
    		rq0 = c3rq0;
    		rq1 = c3rq1;
    		rq2 = c3rq2;
    		
    		rq0_size = &c3rq0_size;
    		rq1_size = &c3rq1_size;
    		rq2_size = &c3rq2_size;
    		break;
    }
    struct task_struct *rq; // pointer to access the specific rq that we need
    int *size; // pointer to access its size
    while(((*rq0_size) > 0)||((*rq1_size) > 0)||((*rq2_size) > 0)){
	    // get the process from the rq
	    if((*rq0_size) > 0){
	    	rq = rq0;
	    	pthread_mutex_lock(&work_mutex);
	    	generated_info = rq0[0];
	    	pthread_mutex_unlock(&work_mutex);
	    	size = rq0_size;
	    }
	    else if ((*rq1_size) > 0){
	    	rq = rq1;
	    	pthread_mutex_lock(&work_mutex);
	    	generated_info = rq1[0];
	    	pthread_mutex_unlock(&work_mutex);
	    	size = rq1_size;
	    }
	    else if ((*rq2_size) > 0){
	    	rq = rq2;
	    	pthread_mutex_lock(&work_mutex);
	    	generated_info = rq2[0];
	    	pthread_mutex_unlock(&work_mutex);
	    	size = rq2_size;
	    }
	    else{
	    	break;
	    }
	    
	    
	    printf("\nCPU %d recieved: \n", my_number);

	    printf("pid: %d, \n", generated_info.pid);
	    printf("SP: %d, \n", generated_info.static_priority);
	    printf("DP: %d, \n", generated_info.dynamic_priority);
	    printf("RT: %d, \n", generated_info.remain_time);
	    printf("TS: %d, \n", generated_info.time_slice);
	    printf("ATS: %d, \n", generated_info.accu_time_slice);
	    printf("CA: %d, \n", generated_info.cpu_affinity);
	    
	    // if the process is FIFO it will run to completion
	    if (generated_info.policy == FIFO){
	    	generated_info.time_slice = generated_info.remain_time;
	    }
	    else{
	    	// Calculate time slice 
	    	generated_info.time_slice = ((140 - generated_info.static_priority) * 20);
	    }
	    
	    gettimeofday(&start, NULL);
	    
	    if (generated_info.accu_time_slice == 0){
	    	generated_info.last_time_run = start;
	    }
	 
	    int deltaT = (int) ((start.tv_sec * 1000000 + start.tv_usec) - (generated_info.last_time_run.tv_sec * 1000000 + generated_info.last_time_run.tv_usec));
	    
	    int ticks = deltaT / ITERATION_DIVIDER;
	    
	    generated_info.sleep_avg += ticks;
	    
	    if (generated_info.sleep_avg > MAX_SLEEP_AVG){
	    	generated_info.sleep_avg = MAX_SLEEP_AVG;
	    }
	    
	    printf("SA: %d, \n", generated_info.sleep_avg/1000); // since this is in microseconds and we want to display in miliseconds
	    
	    // Decrease remaining time
	    generated_info.remain_time -= generated_info.time_slice;
	    if (generated_info.remain_time < 0){
	    	generated_info.remain_time = 0;
	    }
	    
	    // Adjust priority
	    if (generated_info.policy == NORMAL){
	    	generated_info.dynamic_priority = max(100, min(generated_info.static_priority - generated_info.sleep_avg + 5, 139));
	    }
	    else{
		generated_info.dynamic_priority = max(100, min(generated_info.static_priority - BONUS + 5, 139));
	    }
	    
	    // Increase accumulated time slice
	    generated_info.accu_time_slice += generated_info.time_slice;
	    
	    // Set current thread ID
	    generated_info.cpu_affinity = my_number;
	    
	    usleep(generated_info.time_slice);
	    
	    printf("\nCPU %d returned: \n", my_number);
		
	    printf("pid: %d, \n", generated_info.pid);
	    printf("SP: %d, \n", generated_info.static_priority);
	    printf("DP: %d, \n", generated_info.dynamic_priority);
	    printf("RT: %d, \n", generated_info.remain_time);
	    printf("TS: %d, \n", generated_info.time_slice);
	    printf("ATS: %d, \n", generated_info.accu_time_slice);
	    printf("CA: %d, \n", generated_info.cpu_affinity);
	    
	    gettimeofday(&end, NULL);
	    
	    deltaT = (int) ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec));
	    ticks = deltaT / ITERATION_DIVIDER;
	    generated_info.sleep_avg -= ticks;
	    if (generated_info.sleep_avg < 0){
	    	generated_info.sleep_avg = 0;
	    }
	    printf("SA: %d, \n", generated_info.sleep_avg/1000);
	    generated_info.last_time_run = end;
	    
	    for (int i = 0; i < ((*size) - 1); i++) {
            	pthread_mutex_lock(&work_mutex);
            	rq[i] = rq[i + 1];
            	pthread_mutex_unlock(&work_mutex);
            }
            if (generated_info.remain_time > 0){
            	pthread_mutex_lock(&work_mutex);
            	rq[(*size) - 1] = generated_info;
            	pthread_mutex_unlock(&work_mutex);
            }
            else{
            	pthread_mutex_lock(&work_mutex);
            	(*size) -= 1;
            	pthread_mutex_unlock(&work_mutex);
            	printf("Done process pid %d\n", generated_info.pid);
            }
    }
    
    pthread_exit(NULL);
}


void *thread_function_p(void *arg) {
    printf("This is the producer\n");
    
    sleep(5);
    
    struct task_struct generated_info;
    
    FILE *file = fopen("values.csv", "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(1);
    }
    
    struct ProcessData process_data;
    
    int assigned_cpu;
    
    for (int buff_index = 0; buff_index < NUM_PROCESSES; buff_index++){
    	int result = parse_csv_line(file, &process_data);
    	
    	if (result == 1) {
            // Successfully parsed a line, you can use process_data here
            printf("PID: %d, Policy: %d, Priority: %d, Execution Time: %d, CPU Affinity: %d\n",
                   process_data.pid, process_data.policy, process_data.priority,
                   process_data.execution_time, process_data.cpu_affinity);
            generated_info.pid = process_data.pid;
            generated_info.policy = process_data.policy;
            generated_info.static_priority = process_data.priority;
            generated_info.remain_time = process_data.execution_time;
            generated_info.cpu_affinity = process_data.cpu_affinity;
        } else if (result == 0) {
            // End of file or invalid input line
            break;
        } else {
            // Parsing error
            fprintf(stderr, "Error parsing CSV line\n");
            break;
        }

    	// Store struct in appropriate queue
    	pthread_mutex_lock(&work_mutex);
    	if (generated_info.static_priority < 100){
    		switch(generated_info.cpu_affinity){
    			case -1:
    				// if the cpu affinity is -1, then it will go to the smallest queue
	    			int min_val = min4(c0rq0_size, c1rq0_size, c2rq0_size, c3rq0_size);
		    		if (c0rq0_size == min_val){
		    			c0rq0[c0rq0_size] = generated_info;
		    			c0rq0_size++;
		    			assigned_cpu = 0;
		    		}
		    		else if (c1rq0_size == min_val){
		    			c1rq0[c1rq0_size] = generated_info;
		    			c1rq0_size++;
		    			assigned_cpu = 1;
		    		}
		    		else if (c2rq0_size == min_val){
		    			c2rq0[c2rq0_size] = generated_info;
		    			c2rq0_size++;
		    			assigned_cpu = 2;
		    		}
		    		else if (c3rq0_size == min_val){
		    			c3rq0[c3rq0_size] = generated_info;
		    			c3rq0_size++;
		    			assigned_cpu = 3;
		    		}
		    		break;
	    		case 0:
	    			c0rq0[c0rq0_size] = generated_info;
	    			c0rq0_size++;
	    			assigned_cpu = 0;
	    			break;
	    		case 1:
	    			c1rq0[c1rq0_size] = generated_info;
	    			c1rq0_size++;
	    			assigned_cpu = 1;
	    			break;
	    		case 2:
	    			c2rq0[c2rq0_size] = generated_info;
	    			c2rq0_size++;
	    			assigned_cpu = 2;
	    			break;
	    		case 3:
	    			c3rq0[c3rq0_size] = generated_info;
	    			c3rq0_size++;
	    			assigned_cpu = 3;
	    			break;
	    		
    		}
    	}
    	else{
    		if (generated_info.static_priority < 130){
	    		switch(generated_info.cpu_affinity){
	    			case -1:
		    			int min_val = min4(c0rq1_size, c1rq1_size, c2rq1_size, c3rq1_size);
			    		if (c0rq1_size == min_val){
			    			c0rq1[c0rq1_size] = generated_info;
			    			c0rq1_size++;
			    			assigned_cpu = 0;
			    		}
			    		else if (c1rq1_size == min_val){
			    			c1rq1[c1rq1_size] = generated_info;
			    			c1rq1_size++;
			    			assigned_cpu = 1;
			    		}
			    		else if (c2rq1_size == min_val){
			    			c2rq1[c2rq1_size] = generated_info;
			    			c2rq1_size++;
			    			assigned_cpu = 2;
			    		}
			    		else if (c3rq1_size == min_val){
			    			c3rq1[c3rq1_size] = generated_info;
			    			c3rq1_size++;
			    			assigned_cpu = 3;
			    		}
			    		break;
		    		case 0:
		    			c0rq1[c0rq1_size] = generated_info;
		    			c0rq1_size++;
		    			assigned_cpu = 0;
		    			break;
		    		case 1:
		    			c1rq1[c1rq1_size] = generated_info;
		    			c1rq1_size++;
		    			assigned_cpu = 1;
		    			break;
		    		case 2:
		    			c2rq1[c2rq1_size] = generated_info;
		    			c2rq1_size++;
		    			assigned_cpu = 2;
		    			break;
		    		case 3:
		    			c3rq1[c3rq1_size] = generated_info;
		    			c3rq1_size++;
		    			assigned_cpu = 3;
		    			break;
		    		
	    		}
	    	}
	    	else{
	    		switch(generated_info.cpu_affinity){
	    			case -1:
		    			int min_val = min4(c0rq2_size, c1rq2_size, c2rq2_size, c3rq2_size);
			    		if (c0rq2_size == min_val){
			    			c0rq2[c0rq2_size] = generated_info;
			    			c0rq2_size++;
			    			assigned_cpu = 0;
			    		}
			    		else if (c1rq2_size == min_val){
			    			c1rq2[c1rq2_size] = generated_info;
			    			c1rq2_size++;
			    			assigned_cpu = 1;
			    		}
			    		else if (c2rq2_size == min_val){
			    			c2rq2[c2rq2_size] = generated_info;
			    			c2rq2_size++;
			    			assigned_cpu = 2;
			    		}
			    		else if (c3rq2_size == min_val){
			    			c3rq2[c3rq2_size] = generated_info;
			    			c3rq2_size++;
			    			assigned_cpu = 3;
			    		}
			    		break;
		    		case 0:
		    			c0rq2[c0rq2_size] = generated_info;
		    			c0rq2_size++;
		    			assigned_cpu = 0;
		    			break;
		    		case 1:
		    			c1rq2[c1rq2_size] = generated_info;
		    			c1rq2_size++;
		    			assigned_cpu = 1;
		    			break;
		    		case 2:
		    			c2rq2[c2rq2_size] = generated_info;
		    			c2rq2_size++;
		    			assigned_cpu = 2;
		    			break;
		    		case 3:
		    			c3rq2[c3rq2_size] = generated_info;
		    			c3rq2_size++;
		    			assigned_cpu = 3;
		    			break;
		    		
	    		}
	    	}
    	}
    	pthread_mutex_unlock(&work_mutex);
    	
    	printf("Assigned process pid %d to CPU %d\n", generated_info.pid, assigned_cpu);
    }
    
    struct task_struct *rq0, *rq1, *rq2;
    int *rq0_size, *rq1_size, *rq2_size;
    pthread_mutex_lock(&work_mutex);
    // outputs which process is in what queue
    for (int i = 0; i < 4; i++){
    	switch(i){
	    	case 0:
	    		rq0 = c0rq0;
	    		rq1 = c0rq1;
	    		rq2 = c0rq2;
	    		
	    		rq0_size = &c0rq0_size;
	    		rq1_size = &c0rq1_size;
	    		rq2_size = &c0rq2_size;
	    		break;
	    	case 1:
	    		rq0 = c1rq0;
	    		rq1 = c1rq1;
	    		rq2 = c1rq2;
	    		
	    		rq0_size = &c1rq0_size;
	    		rq1_size = &c1rq1_size;
	    		rq2_size = &c1rq2_size;
	    		break;
	    	case 2:
	    		rq0 = c2rq0;
	    		rq1 = c2rq1;
	    		rq2 = c2rq2;
	    		
	    		rq0_size = &c2rq0_size;
	    		rq1_size = &c2rq1_size;
	    		rq2_size = &c2rq2_size;
	    		break;
	    	case 3:
	    		rq0 = c3rq0;
	    		rq1 = c3rq1;
	    		rq2 = c3rq2;
	    		
	    		rq0_size = &c3rq0_size;
	    		rq1_size = &c3rq1_size;
	    		rq2_size = &c3rq2_size;
	    		break;
    	}
    	for (int j = 0; j < (*rq0_size); j++){
    		printf("CPU %d rq0 has process pid %d\n", i, rq0[j].pid);
    	}
    	for (int j = 0; j < (*rq1_size); j++){
    		printf("CPU %d rq1 has process pid %d\n", i, rq1[j].pid);
    	}
    	for (int j = 0; j < (*rq2_size); j++){
    		printf("CPU %d rq2 has process pid %d\n", i, rq2[j].pid);
    	}
    }
    pthread_mutex_unlock(&work_mutex);
    sleep(5);
    producer_still_going = 0;
    pthread_exit(NULL);
}
