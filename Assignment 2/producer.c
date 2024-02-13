#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <sys/msg.h>

#include <sys/stat.h>

#include <sys/sem.h>

#include <sys/shm.h>

#include "msg_queue.h"
#include "shm.h"

static int set_semvalue(int id, int num, int set_val);
static void del_semvalue(int id);
static int semaphore_p(int id, int num);
static int semaphore_v(int id, int num);

static int sem_id;

int main()
{
    // for getting keys from server
    struct my_msg_st send_data, recieve_data;
    int msgid;
    char buffer[BUFSIZ];
    int sem_key, shm_key;
    char response[MAX_TEXT];
    
    // for sending file size to consumer
    struct stat buf;
    
    // for accessing shared memory
    struct shared_use_st* shared_stuff;
    void *shared_memory = (void *)0;
    int shmid;
    
    // for reading from input file
    char char_buffer[MAX_TEXT];
    FILE *fptr;



    msgid = msgget((key_t)QUEUE_ID, 0666 | IPC_CREAT);

    if (msgid == -1) {
        fprintf(stderr, "msgget failed with error: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    sprintf(buffer, "REQUEST;sem");
    send_data.my_msg_type = TO_SERVER;
    strcpy(send_data.some_text, buffer);

    if (msgsnd(msgid, (void *)&send_data, MAX_TEXT, 0) == -1) {
        fprintf(stderr, "msgsnd failed\n");
        exit(EXIT_FAILURE);
    }
        
    if (msgrcv(msgid, (void *)&recieve_data, BUFSIZ, TO_PRODUCER, 0) == -1) {
        fprintf(stderr, "msgrcv failed with error: %d\n", errno);
        exit(EXIT_FAILURE);
    }
        
    sscanf(recieve_data.some_text, "RESPONSE;%s", response);
    sscanf(response, "sem;%d", &sem_key);
    printf("Recieved semaphore key: %d\n", sem_key);
        
    sprintf(buffer, "REQUEST;shm");
    send_data.my_msg_type = TO_SERVER;
    strcpy(send_data.some_text, buffer);
        
    if (msgsnd(msgid, (void *)&send_data, MAX_TEXT, 0) == -1) {
        fprintf(stderr, "msgsnd failed\n");
        exit(EXIT_FAILURE);
    }
        
    if (msgrcv(msgid, (void *)&recieve_data, BUFSIZ, TO_PRODUCER, 0) == -1) {
        fprintf(stderr, "msgrcv failed with error: %d\n", errno);
        exit(EXIT_FAILURE);
    }
  
    sscanf(recieve_data.some_text, "RESPONSE;%s", response);
    sscanf(response, "shm;%d", &shm_key);
    printf("Recieved shared memory key: %d\n", shm_key);
    
    sleep(5);
    
    stat("input_file", &buf);
    
    int file_size = buf.st_size;
    
    sprintf(send_data.some_text, "FILE_SIZE;%d", file_size);
    send_data.my_msg_type = FILE_SIZE_MSG;
    
    if (msgsnd(msgid, (void *)&send_data, MAX_TEXT, 0) == -1) {
        fprintf(stderr, "msgsnd failed\n");
        exit(EXIT_FAILURE);
    }
    
    printf("File size is %d - sent to consumer\n", file_size);
    
    sem_id = semget((key_t)sem_key, 3, 0666 | IPC_CREAT);
    
    set_semvalue(sem_id, 0, 1);
    set_semvalue(sem_id, 1, 0);
    set_semvalue(sem_id, 2, 10);
    
    shmid = shmget((key_t)shm_key, NUM_BUFFERS * sizeof(struct shared_use_st), 0666 | IPC_CREAT);

    if (shmid == -1) {
        fprintf(stderr, "shmget failed\n");
        exit(EXIT_FAILURE);
    }

    shared_memory = shmat(shmid, (void *)0, 0);
    if (shared_memory == (void *)-1) {
	fprintf(stderr, "shmat failed\n");
	exit(EXIT_FAILURE);
    }
    printf("Memory attached\n");

    fptr = fopen(INPUT_FILENAME, "r");
    
    printf("Opened file\n");
    
    int i = 0;
    int index;
    
    char *fgets_return;
    int char_size;
    int bytes_read = 0;
    
    int keep_going = 1;
    
    while(keep_going){
    	index = i % NUM_BUFFERS;
    	shared_stuff = (struct shared_use_st *) (shared_memory + index * sizeof(struct shared_use_st));
    	
    	fgets_return = fgets(char_buffer, MAX_TEXT, fptr);
    	if (fgets_return == NULL){
    	    printf("Reached end of file\n");
    	    keep_going = 0;
    	    strcpy(char_buffer, END_OF_FILE_CONSTANT);
    	}
    	char_size = strlen(char_buffer);
    	if(!semaphore_p(sem_id, 2)) exit(EXIT_FAILURE);
    	if(!semaphore_p(sem_id, 0)) exit(EXIT_FAILURE);
    	
    	// Critical Section - write to shared memory
    	strcpy(shared_stuff->some_text, char_buffer);
    	shared_stuff->offset = bytes_read;
    	shared_stuff->actual_size = char_size;
    	
    	if(!semaphore_v(sem_id, 0)) exit(EXIT_FAILURE);
    	if(!semaphore_v(sem_id, 1)) exit(EXIT_FAILURE);
    	bytes_read += char_size;
    	if (keep_going){
    		printf("Wrote line %d: %s", i + 1, char_buffer);
    	}
    	i++;
    }
    
    fclose(fptr);
      
    if (shmdt(shared_memory) == -1) {
        fprintf(stderr, "shmdt failed\n");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}

/* The function set_semvalue initializes the semaphore using the SETVAL command in a
 semctl call. We need to do this before we can use the semaphore. */

static int set_semvalue(int id, int num, int set_val)
{
    union semun sem_union;

    sem_union.val = set_val;
    if (semctl(id, num, SETVAL, sem_union) == -1) return(0);
    return(1);
}

/* The del_semvalue function has almost the same form, except the call to semctl uses
 the command IPC_RMID to remove the semaphore's ID. */

static void del_semvalue(int id)
{
    union semun sem_union;
    
    if (semctl(id, 0, IPC_RMID, sem_union) == -1)
        fprintf(stderr, "Failed to delete semaphore\n");
}

/* semaphore_p changes the semaphore by -1 (waiting). */

static int semaphore_p(int id, int num)
{
    struct sembuf sem_b;
    
    sem_b.sem_num = num;
    sem_b.sem_op = -1; /* P() */
    sem_b.sem_flg = 0;
    if (semop(id, &sem_b, 1) == -1) {
        fprintf(stderr, "semaphore_p failed\n");
        return(0);
    }
    return(1);
}

/* semaphore_v is similar except for setting the sem_op part of the sembuf structure to 1,
 so that the semaphore becomes available. */

static int semaphore_v(int id, int num)
{
    struct sembuf sem_b;
    
    sem_b.sem_num = num;
    sem_b.sem_op = 1; /* V() */
    sem_b.sem_flg = 0;
    if (semop(id, &sem_b, 1) == -1) {
        fprintf(stderr, "semaphore_v failed\n");
        return(0);
    }
    return(1);
}
