/* This server sends the key values for the producer to write to the shared memory*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <sys/msg.h>

#include "msg_queue.h"

int main()
{
    struct my_msg_st recieve_data, send_data_prod, send_data_cons;
    int msgid;
    char msg[MAX_TEXT];
    char request[MAX_TEXT];
    int num;

    msgid = msgget((key_t)QUEUE_ID, 0666 | IPC_CREAT);

    if (msgid == -1) {
        fprintf(stderr, "msgget failed with error: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    
    if (msgrcv(msgid, (void *)&recieve_data, BUFSIZ, TO_SERVER, 0) == -1) {
        fprintf(stderr, "msgrcv failed with error: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    printf("Request Recieved: %s\n", recieve_data.some_text);
    
    srand((unsigned int)getpid());
        
    num = rand() % (9999 - 1000 + 1) + 1000;
        
    sprintf(send_data_prod.some_text, "RESPONSE;sem;%d", num);
    sprintf(send_data_cons.some_text, "RESPONSE;sem;%d", num);
        
    printf("Data sent: %s\n", send_data_prod.some_text);
        
    send_data_prod.my_msg_type = TO_PRODUCER;
        
    if (msgsnd(msgid, (void *)&send_data_prod, MAX_TEXT, 0) == -1) {
        fprintf(stderr, "msgsnd failed\n");
        exit(EXIT_FAILURE);
    }
    
    send_data_cons.my_msg_type = TO_CONSUMER;
    
    if (msgsnd(msgid, (void *)&send_data_cons, MAX_TEXT, 0) == -1) {
        fprintf(stderr, "msgsnd failed\n");
        exit(EXIT_FAILURE);
    }
        
    if (msgrcv(msgid, (void *)&recieve_data, BUFSIZ, TO_SERVER, 0) == -1) {
        fprintf(stderr, "msgrcv failed with error: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    printf("Request Recieved: %s\n", recieve_data.some_text);
        
    num = rand() % (9999 - 1000 + 1) + 1000;
        
    sprintf(send_data_prod.some_text, "RESPONSE;shm;%d", num);
    sprintf(send_data_cons.some_text, "RESPONSE;shm;%d", num);
        
    printf("Data sent: %s\n", send_data_prod.some_text);
        
    send_data_prod.my_msg_type = TO_PRODUCER;
    send_data_cons.my_msg_type = TO_CONSUMER;
        
    if (msgsnd(msgid, (void *)&send_data_prod, MAX_TEXT, 0) == -1) {
        fprintf(stderr, "msgsnd failed\n");
        exit(EXIT_FAILURE);
    }
    if (msgsnd(msgid, (void *)&send_data_cons, MAX_TEXT, 0) == -1) {
        fprintf(stderr, "msgsnd failed\n");
        exit(EXIT_FAILURE);
    }
        
    sleep(5);

    

    exit(EXIT_SUCCESS);
}
