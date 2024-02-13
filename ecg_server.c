#include "CPS.h"

int main()
{
	int pipe_fd;
    	int res;
    	int open_mode = O_RDONLY;
    	char buffer[BUFFER_SIZE + 1];
    	int bytes_read = 0;

    	memset(buffer, '\0', sizeof(buffer));
    	
    	if (access(ECG_FIFO, F_OK) == -1) {
        res = mkfifo(ECG_FIFO, 0777);
        	if (res != 0) {
            	fprintf(stderr, "Could not create fifo %s\n", ECG_FIFO);
            	exit(EXIT_FAILURE);
        	}
    	}
    	
    	printf("Process %d opening FIFO O_RDONLY\n", getpid());
    	pipe_fd = open(ECG_FIFO, open_mode);
    	printf("Process %d result %d\n", getpid(), pipe_fd);

    	if (pipe_fd != -1) {
        	do {
            		res = read(pipe_fd, buffer, BUFFER_SIZE);
            		bytes_read += res;
        	} while (res > 0);
        	(void)close(pipe_fd);
    	}
    	else {
        	exit(EXIT_FAILURE);
    	}

    	printf("ECG data recieved, %d bytes read\n", bytes_read);
    	
    	open_mode = O_WRONLY;
    	
    	res = mkfifo(ECG_FIFO2, 0777);
        if (res != 0) {
        	fprintf(stderr, "Could not create fifo for %s\n", ECG_FIFO2);
            	exit(EXIT_FAILURE);
        }
    	pipe_fd = open(ECG_FIFO2, open_mode);
    	if (pipe_fd != 1){
    		write(pipe_fd, &bytes_read, sizeof(int));
    		close(pipe_fd);
    	}
    	unlink(ECG_FIFO2);
    	exit(EXIT_SUCCESS);
}
