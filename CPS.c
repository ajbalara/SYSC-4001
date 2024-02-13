// Header files
#include <sys/time.h>
#include <signal.h>
#include "CPS.h"

static int signals = 0;

void success(int sig)
{
	if (sig == SIGUSR1){
		signals++;
		printf("%d out of 2 uploads succeeded\n", signals);
	}
	else{
		printf("Upload failed\n");
	}
}

int main()
{
	pid_t pid1, pid2;
	
	printf("Launching CPS system PID - %d\n", getpid());
	
	pid1 = fork();
	switch(pid1)
	{
	case -1:
		perror("fork1 failed");
		exit(1);
	case 0:
		printf("Launching ECG upload system PID - %d\n", getpid());
		// ECG code
		int res;
		int open_mode = O_WRONLY;
		int pipe_fd;
		int bytes_sent = 0;
		int bytes_sent_check;
    		char buffer[BUFFER_SIZE + 1];
    		if (access(ECG_FIFO, F_OK) == -1) {
        		res = mkfifo(ECG_FIFO, 0777);
        		if (res != 0) {
            			fprintf(stderr, "Could not create fifo for %s\n", ECG_FIFO);
            			exit(EXIT_FAILURE);
        		}
    		}
    		printf("ECG process opening FIFO\n");
    		pipe_fd = open(ECG_FIFO, open_mode);
    		printf("ECG process result %d\n", pipe_fd);
    		if (pipe_fd != -1) {
        		while(bytes_sent < TEN_MEG) {
            			res = write(pipe_fd, buffer, BUFFER_SIZE);
            			usleep(50);
            			if (res == -1) {
                			fprintf(stderr, "Write error on pipe\n");
                			char *error_msg = strerror(errno);
                			printf("errno: %d msg: %s\n", errno, error_msg);
                			exit(EXIT_FAILURE);
            			}
            			bytes_sent += res;
        		}
        		(void)close(pipe_fd); 
    		}
    		else {
        		exit(EXIT_FAILURE);        
    		}
    		unlink(ECG_FIFO);
    		printf("%d bytes of ECG data written\n", bytes_sent);
    		
    		open_mode = O_RDONLY;
    		
    		pipe_fd = open(ECG_FIFO2, open_mode);
    		if (pipe_fd != 1){
    			read(pipe_fd, &bytes_sent_check, sizeof(int));
    			close(pipe_fd);
    		}
    		if(bytes_sent == bytes_sent_check){
    			printf("Confirmation recieved that %d bytes of ECG data were written to server\n", bytes_sent_check);
    			kill(getppid(), SIGUSR1);
    		}
    		else{
    			kill(getppid(), SIGALRM);
    		}
    		exit(EXIT_SUCCESS);
		break;
	default:
		pid2 = fork();
		switch(pid2)
		{
		case -1:
			perror("fork2 failed");
			exit(1);
		case 0:
			printf("Launching image upload system PID  - %d\n", getpid());
			// image code
			int res;
			int open_mode = O_WRONLY;
			int pipe_fd;
			int bytes_sent = 0;
    			char buffer[BUFFER_SIZE + 1];
    			if (access(IMAGE_FIFO, F_OK) == -1) {
        			res = mkfifo(IMAGE_FIFO, 0777);
        			if (res != 0) {
            				fprintf(stderr, "Could not create fifo for %s\n", IMAGE_FIFO);
            				exit(EXIT_FAILURE);
        			}
    			}
    			printf("Image process opening FIFO\n");
    			pipe_fd = open(IMAGE_FIFO, open_mode);
    			printf("Image process result %d\n", pipe_fd);
    			if (pipe_fd != -1) {
        			while(bytes_sent < TEN_MEG) {
            				res = write(pipe_fd, buffer, BUFFER_SIZE);
            				usleep(50);
            				if (res == -1) {
                				fprintf(stderr, "Write error on pipe\n");
                				char *error_msg = strerror(errno);
                				printf("errno: %d msg: %s\n", errno, error_msg);
                				exit(EXIT_FAILURE);
            				}
            				bytes_sent += res;
        			}
        			(void)close(pipe_fd); 
    			}
    			else {
        			exit(EXIT_FAILURE);        
    			}
    			unlink(IMAGE_FIFO);
    			printf("%d bytes of image data written\n", bytes_sent);
    		
    			open_mode = O_RDONLY;
    		
    			pipe_fd = open(IMAGE_FIFO2, open_mode);
    			if (pipe_fd != 1){
    				read(pipe_fd, &bytes_sent_check, sizeof(int));
    				close(pipe_fd);
    			}
    			if(bytes_sent == bytes_sent_check){
    				printf("Confirmation recieved that %d bytes of image data were written to server\n", bytes_sent_check);
    				kill(getppid(), SIGUSR1);
    			}
    			else{
    				kill(getppid(), SIGALRM);
    			}
    			exit(EXIT_SUCCESS);
			break;
		default:
			// parent code
			struct sigaction act;
    			act.sa_handler = success;
    			sigemptyset(&act.sa_mask);
    			act.sa_flags = 0;
    			sigaction(SIGUSR1, &act, 0);
    			pause();
    			sigemptyset(&act.sa_mask);
    			act.sa_flags = 0;
    			sigaction(SIGUSR1, &act, 0);
    			pause();
			printf("CPS upload completed\n");
			break;
		}
		break;
	}
	exit(0);
}
