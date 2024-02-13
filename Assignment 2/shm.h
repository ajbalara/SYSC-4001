#define FILE_SIZE_MSG 4
#define NUM_BUFFERS 10
#define BUFFER_SIZE 1024
#define INPUT_FILENAME "input_file"
#define OUTPUT_FILENAME "output_file"
#define END_OF_FILE_CONSTANT "Done file"

struct shared_use_st {
    char some_text[BUFFER_SIZE];
    int offset;
    int actual_size;
};

#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
    /* union semun is defined by including <sys/sem.h> */
#else
    /* according to X/OPEN we have to define it ourselves */
    union semun {
        int val;                    /* value for SETVAL */
        struct semid_ds *buf;       /* buffer for IPC_STAT, IPC_SET */
        unsigned short int *array;  /* array for GETALL, SETALL */
        struct seminfo *__buf;      /* buffer for IPC_INFO */
    };
#endif
