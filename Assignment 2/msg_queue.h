#define QUEUE_ID 2345
#define MAX_TEXT 512
#define TO_SERVER 1
#define TO_PRODUCER 2
#define TO_CONSUMER 3

struct my_msg_st {
    long int my_msg_type;
    char some_text[MAX_TEXT];
};
