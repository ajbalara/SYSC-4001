#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>

#define ECG_FIFO "ECG"
#define ECG_FIFO2 "ECG2"
#define IMAGE_FIFO "image"
#define IMAGE_FIFO2 "image2"
#define BUFFER_SIZE PIPE_BUF
#define TEN_MEG (1024 * 1024 * 10)
