//
// Created by CHETAN JOHAL on 5/20/17.
//

#ifndef WEB_SERVER_MTWS_H
#define WEB_SERVER_MTWS_H

struct request
{
    int accept_Id;
    int size;
    char file_name[1024];
    unsigned int ip_addr;
    char time_arr[1024];
    char in_buffer[2048];

}req;

//queue structure
struct node
{
    struct request r;
    struct node *link;
}*new,*temp,*p,*head=NULL,*tail=NULL;
typedef struct node N;

int empty_thread;  //free_thread to empty_thread
sem_t semaphore;
int debugger_count=0;
int scheduler_count=0;
int log_count=0;
char * file=NULL;

void insertion(int,char*, int, unsigned int,char*,char*);
void display();
struct request remove_sjf(int temp);
struct request extract_elem();
void print_help_option();

void *thread_serve();
void *thread_scheduler(void *arg);
void *thread_listen(void *arg);



#endif //WEB_SERVER_MTWS_H
