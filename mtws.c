//
// Created by CHETAN JOHAL on 5/21/17.
//
#include<fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/stat.h>
#include <semaphore.h>
#include "mtws.h"

#define OK_IMAGE    "HTTP/1.0 200 OK\nContent-Type:image/gif\n\n"
#define OK_TEXT     "HTTP/1.0 200 OK\nContent-Type:text/html\n\n"
#define NOTOK_404   "HTTP/1.0 404 Not Found\nContent-Type:text/html\n\n"
#define MESS_404    "<html><body><h1>FILE NOT FOUND</h1></body></html>"
#define BUF_SIZE 2048


void display()
{
    if(head==NULL)
        printf("\nempty queue");
    else
    {
        int a;

        temp=head;
        while(temp!=NULL)
        {
            a=(temp->r.accept_Id);
            printf("\n accept_Id is %d, file name is %s, file size is %d , ip addr is %u, request is %s,time is %s",a,temp->r.file_name,temp->r.size,temp->r.ip_addr,temp->r.in_buffer,temp->r.time_arr);
            temp=temp->link;
        }
    }
}


void insertion(int xyz,char *f,int size,unsigned int ip, char * time_arr,char * in_buffer)
{


    new=(N*)malloc(sizeof(N));
   // int n;
    char x[1024];
    char y[1024];
    char z[1024];
    strcpy(x,f);
    strcpy(y,time_arr);
    strcpy(z,in_buffer);
    new->r.accept_Id=xyz;
    strcpy(new->r.file_name,x);
    new->r.ip_addr=ip;
    strcpy(new->r.time_arr,y);
    strcpy(new->r.in_buffer,z);

    new->r.size=size;
    new->link=NULL;
    if(head==NULL)
        head=new;
    else
        tail->link=new;
    tail=new;
    display();

}

// queue functions

struct request remove_sjf(int number)
{
    printf("\nentered remove_sjf");
    if(head==NULL)
    {
        printf("\n\nempty list");
    }
    else
    {
        struct node *old,*temp;
        temp=head;
        while(temp!=NULL)
        {
            if(temp->r.accept_Id==number)
            {
                if(temp==head)
                    head=temp->link;
                else
                    old->link=temp->link;
                return(temp->r);

            }
            else
            {
                old=temp;
                temp=temp->link;
            }
        }
    }
}


struct request extract_elem()
{

    if(head==NULL)
        printf("\nempty queue");
    else
    {
        struct request req1;
        p=head;
        head=head->link;
        req1.accept_Id=p->r.accept_Id;
        strcpy(req1.file_name,p->r.file_name);
        req1.size=p->r.size;
        free(p);
        return(req1);
    }
}

// end of queue functions

void print_help_option()
{
    printf("\n−d : Enter debugging mode. That is, do not daemonize, only accept one connection at a "
                   "\ntime and enable logging to stdout. Without this option, the web server should run "
                   "as a daemon process in the background. \n−h : Print a usage summary with all options "
                   "and exit. \n−l file : Log all requests to the given file. See LOGGING for details."
                   "\n−p port : Listen on the given port. If not provided, myhttpd will listen on port 8080."
                   " \n−r dir : Set the root directory for the http server to dir. \n−t time : Set the queuing"
                   " time to time seconds. The default should be 60 seconds. \n−n threadnum: Set number of threads"
                   " waiting ready in the execution thread pool to threadnum. \nThe default should be 4 execution threads."
                   " \n−s sched: Set the scheduling policy. It can be either FCFS or SJF. The default will be FCFS.");
}


// thread pool function

pthread_mutex_t s_thread = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t q_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER;

void *thread_serve()
{
    while(1)
    {

        pthread_mutex_lock(&s_thread);
        pthread_cond_wait(&condition_var,&s_thread);
        struct request r=req;
        pthread_mutex_unlock(&s_thread);
        time_t now;					// getting the time the job has been assigned to the serving thread
        time(&now);
        struct tm * ct=localtime(&now); //getting localtime
        int ch[128], time_serve[128];
        struct timeval tv;
        strftime(ch, sizeof ch, "[%d/%b/%Y : %H:%M:%S %z]", ct); //format of the timestamp string we need
        snprintf(time_serve, sizeof time_serve, ch, tv.tv_usec); //printing the needed timestamp string


        unsigned int ip=r.ip_addr;

        /* code adapted from stackoverflow.com */

        unsigned char bytes[4];
        bytes[0] = ip & 0xFF;
        bytes[1] = (ip >> 8) & 0xFF;
        bytes[2] = (ip >> 16) & 0xFF;
        bytes[3] = (ip >> 24) & 0xFF;

        //struct request r= *((struct request *)arg);
        if(debugger_count==0&& log_count==1)
        {

            FILE * file_des=fopen(file,"a");
            //printf("\n in  serving thread \n");
            fprintf(file_des,"%d.%d.%d.%d\t-\t ", bytes[0], bytes[1], bytes[2], bytes[3]);
            fprintf(file_des,"%s\t %s\t %s \t status\t %d\n",r.time_arr,time_serve,r.in_buffer,r.size);
            fclose(file_des);
        }

        else if(debugger_count==1)
        {

            printf("\n%d.%d.%d.%d\t-\t %s\t %s\t %s \t status\t %d\n", bytes[0], bytes[1], bytes[2], bytes[3],r.time_arr,time_serve,r.in_buffer,r.size);

        }


        //printf("\n in  serving thread copied structure\n");

        char           in_buffer[BUF_SIZE];
        char           out_buf[BUF_SIZE];
        char           *file_name;
        file_name=malloc(sizeof(char *));
        int accept_Id;
        unsigned int   fd1;
        unsigned int   buffer_length;
        unsigned int   retcode;
        int m;

        //printf("\n in  serving thread before copying variables\n");
        accept_Id=r.accept_Id;
        file_name=r.file_name;


        {


            /* This part of code adopted from http://kturley.com/simple-multi-threaded-web-server-written-in-c-using-pthreads/ */

            fd1 = open(&file_name[1], O_RDONLY, S_IREAD | S_IWRITE);

            memset(out_buf, 0, sizeof(out_buf));


            if (fd1 == -1)
            {
                printf("File %s not found - sending an HTTP 404 \n", &file_name[1]);
                strcpy(out_buf, NOTOK_404);
                send(accept_Id, out_buf, strlen(out_buf), 0);
                strcpy(out_buf, MESS_404);
                send(accept_Id, out_buf, strlen(out_buf), 0);
            }
            else
            {
                printf("File %s is being sent \n", &file_name[1]);
                if ((strstr(file_name, ".jpg") != NULL)||(strstr(file_name, ".gif") != NULL))
                { strcpy(out_buf, OK_IMAGE); }

                else
                { strcpy(out_buf, OK_TEXT); }
                send(accept_Id, out_buf, strlen(out_buf), 0);

                buffer_length = 1;
                while (buffer_length > 0)
                {
                    buffer_length = read(fd1, out_buf, BUF_SIZE);
                    if (buffer_length > 0)
                    {
                        send(accept_Id, out_buf, buffer_length, 0);

                        //printf("\nin serving thread after sending file\n");

                        //pthread_mutex_lock(&s_thread);

                        sem_post(&semaphore);


                    }
                }
            }
        }




    }

}

//scheduler thread
void *thread_scheduler(void *arg)
{
    unsigned int schedalg=*((unsigned int*)arg);
    int accept_Id,n;
    if(schedalg==0)
    {
        while(1)
        {


            if(head!=NULL)
            {

                sem_wait(&semaphore);
                //printf("\nin sched thread before extracting element\n");
                //printf("\nscheduler locking mutex\n");
                pthread_mutex_lock(&s_thread);
                pthread_mutex_lock(&q_mutex);
                req=extract_elem();
                //printf("\n popped element in scheduler thread");
                pthread_mutex_unlock(&q_mutex);

                //printf("\nscheduler unlocked mutex\n");
                // call serving thread from thread pool

                //printf("\nin sched thread before sending to serving thread\n");

                pthread_cond_signal(&condition_var);


                empty_thread--;
                pthread_mutex_unlock(&s_thread);

            }

            else
            {

                continue;
            }
        }
    }
    else
    {
        //code for SJF scheduling algorithm
        int shortestjob_fd=0;
        int min;
        int a,b;
        while(1)
        {	pthread_mutex_lock(&q_mutex);
            temp=head;
            if (temp==NULL)
            {

                continue;
            }
            else if(temp->link==NULL)
            {
                shortestjob_fd=temp->r.accept_Id;
            }
            else

            {
                min=temp->r.size;
                while(temp->link!=NULL)
                {

                    b=temp->link->r.size;
                    if(min<=b)
                    {
                        shortestjob_fd=temp->r.accept_Id;
                    }
                    else if(min>b)
                    {
                        min=temp->link->r.size;
                        shortestjob_fd=temp->link->r.accept_Id;
                    }
                    printf("\n %d",a);
                    temp=temp->link;
                }
            }
            pthread_mutex_lock(&s_thread);

            req=remove_sjf(shortestjob_fd);
            pthread_cond_signal(&condition_var);
            pthread_mutex_unlock(&s_thread);

            pthread_mutex_unlock(&q_mutex);
        }

    }

}

// Listening and queueing thread

void *thread_listen(void *arg) {
    unsigned int sockfd = *((unsigned int *) arg);
    int i, size;
    unsigned int accept_Id, ids2;
    socklen_t clilen;
    int newsockfd[10], c;
    int n;
    char buffer[256];
    pthread_t t_serve[10];
    struct sockaddr_in cli_addr;
    clilen = sizeof(cli_addr);
    unsigned int retval;
    char request_buffer[1024];


    int retcode;
    off_t file_size;
    char in_buffer[BUF_SIZE];

    char *fname = malloc(sizeof(char *));
    struct stat st;
    int k, j;

    int l;


    listen(sockfd, 5);

    while (1) {

        //printf("\nin listening thread before accept\n");
        accept_Id = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (accept_Id < 0)
            perror("error in accepting");

        unsigned int ip = cli_addr.sin_addr.s_addr;


        time_t now;
        time(&now);
        struct tm *ct = localtime(&now); //localtime
        int ch[128], time_arr[128];
        struct timeval tv;
        strftime(ch, sizeof ch, "[%d/%b/%Y : %H:%M:%S %z]", ct); //timestamp string we need
        snprintf(time_arr, sizeof time_arr, ch, tv.tv_usec); //printing timestamp string

char *file_name = malloc(sizeof(char *));

        memset(in_buffer, 0, sizeof(in_buffer));
        retcode = recv(accept_Id, in_buffer, BUF_SIZE, 0);


        if (retcode < 0) {
            printf("recv error detected ...\n");
        } else {

            strtok(in_buffer, " ");
            file_name = strtok(NULL, " ");
        }

        if (file_name != NULL) {


            k = 1, j = 0;
            while (k < strlen(file_name)) {
                fname[j] = file_name[k];
                k++;
                j++;
            }


            if (stat(fname, &st) == 0)
                file_size = st.st_size;

            insertion(accept_Id, file_name, file_size, ip, time_arr, in_buffer);
        } else {
            continue;
        }


    }
}


int main(int argc, char *args[]) {
    int thread_status[10];
    pthread_t t_listener, t_scheduler, t_serve[10];
    int sockfd, ids;
    char *dir;
    file = malloc(sizeof(char *));
    dir = malloc(sizeof(char *));


    int portnum = 8080, threadnum = 4, sleep_time = 60;
    int i;
    int help_flag = 0, dir_flag = 0, time_flag, threadnum_flag = 0;

    // Parser code
    for (i = 0; i < argc; i++) {

        if (strcmp(args[i], "-h") == 0) {
            help_flag = 1;

        } else if (strcmp(args[i], "-n") == 0) {
            threadnum = atoi(args[i + 1]);
        } else if (strcmp(args[i], "-d") == 0) {
            debugger_count = 1;
            threadnum = 1;

        } else if (strcmp(args[i], "-l") == 0) {
            log_count = 1;
            file = args[i + 1];
        } else if (strcmp(args[i], "-p") == 0) {
            portnum = atoi(args[i + 1]);
        } else if (strcmp(args[i], "-r") == 0) {
            dir_flag = 1;
            dir = args[i + 1];
        } else if (strcmp(args[i], "-t") == 0) {
            time_flag = 1;
            sleep_time = atoi(args[i + 1]);
        } else if (strcmp(args[i], "-s") == 0) {
            if (strcmp(args[i + 1], "FCFS") == 0)
                scheduler_count = 0;
            else if (args[i + 1], "SJF")
                scheduler_count = 1;
            else
                printf("Please enter a proper scheduling algorithm");
        }

    }

    //empty_thread=threadnum;
    sem_init(&semaphore, 0, threadnum);

    //Parser code ends

    if (help_flag == 1)            // printing help options and exit if -h option is specified
    {
        print_help_option();
        exit(1);
    } else if (dir_flag == 1)            //changing directory if -d option is specified
    {
        if (chdir(dir) < 0) {
            perror("\ndirectory doesnt exist");
            exit(1);
        }
    }

    struct sockaddr_in serv_addr;
    //printf("before socket creation");
    sockfd = socket(AF_INET, SOCK_STREAM, 0);            //creation of socket
    //printf("\n after socket creation socket id is %d", sockfd);
    if (sockfd < 0)
        perror("error creating socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portnum);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)        //binding socket
        perror("binding error");

    int w;
    for (w = 0; w < threadnum; w++) {
        pthread_create(&t_serve[w], NULL, &thread_serve, NULL);
    }

    ids = sockfd;
    pthread_create(&t_listener, NULL, &thread_listen, &ids);            // listener thread
    sleep(sleep_time);                            // putting scheduler to sleep
    pthread_create(&t_scheduler, NULL, &thread_scheduler, &scheduler_count);    //scheduler thread
    pthread_join(t_listener, NULL);
    pthread_join(t_scheduler, NULL);
    display();
    close(sockfd);
    return 0;
}


