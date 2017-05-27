# Multi-threaded Web-Server

The objective of this project is to implement a multi-threaded Web server that is capable of processing multiple simultaneous service requests in parallel. 

# DESCRIPTION:

mtws is a basic web server. It ties to a given port on the given address and sits tight to incoming HTTP/1.0 requests. It serves content from the given directory. That is, any demands for reports is settled with respect to this directory (the archive root – by default,the index where the server is running).

# Credits

I have followed the project structure from the online course on Udacity "Introduction to Operating Systems" by Georgia Institute of Technology.

- Socket API (http://pubs.opengroup.org/onlinepubs/7908799/xns/syssocket.h.html)
- pThreads (http://pubs.opengroup.org/onlinepubs/7908799/xsh/pthread.h.html)
- http://kturley.com/simple-multi-threaded-web-server-written-in-c-using-pthreads
- StackOverflow

# OPTIONS:

−d : Enter debugging mode. Without this mode, the web server ought to keep running as a daemon procedure out of sight.
−h : Print a usage summary with all options and exit.
−l file : Log all requests to the given file. 
−p port : Listen on the given port. If not provided, mtws will listen on port 8080.
−r dir : Set the root directory for the http server to dir.
−t time : Set the queuing time to time seconds. The default is 60 seconds.
−n threadnum : Set number of threads waiting ready in the execution thread pool to threadnum. The default should is 4 execution threads.
−s sched : Set the scheduling policy. It can be either FCFS or SJF. The default will be FCFS.

# PROTOCOL:

mtws is a simplified version of HTTP/1.0.

If the request type was a GET, then it will subsequently return the data of the requested file. After serving the request, the connection is terminated. The HEAD request will only return the metadata but not actual content.

If the request was for a directory and the directory does not contain a file named "index.html", then mtws will generate a directory index, listing the contents of the directory in alphanumeric order. Files starting with a "." are ignored.

If the request begins with a ‘~’, then the following string up to the first slash is translated into that user’s mtws directory (ie / home//mtws/).

# LOGGING:

By default, mtws does not do any logging. If explicitly enabled via the −l flag, mtws will log every request in a slight variation of Apache’s so-called "common" format:

’%a %t %t %r %>s %b’
all in a single line per request. That is, it will log:

%a : The remote IP address.
%t : The time the request was received by the queuing thread (in GMT).
%t : The time the request was assigned to an execution thread by the scheduler (in GMT).
%r : The (quoted) first line of the request.
%s : The status of the request.
%b : Size of the response in bytes. i.e, "Content-Length".


The server will consist of 2+n threads. A pool of n threads will be always ready for executing/serving incoming requests (n x execution threads). The number n is given as a parameter when you start the server (using option: -n threadnum). One thread will be responsible for listening to incoming HTTP requests and inserting them in a ready queue (1 x queuing thread). Another thread will be responsible for choosing a request from the ready queue and scheduling it to one of the execution threads (1 x scheduling thread).

# QUEUING

The queuing thread will be continuously listening to the port p for incoming http requests. As soon as a new http request comes to the mtws server, it will be inserted into the ready queue. For the first t seconds after the server is started, there will be no execution and all requests will wait in the ready queue.

# SCHEDULING

The scheduling policy to be used will be set via the [–s sched] option when mtws server is first started. The available policies are First Come First Serve (FCFS) and Shortest Job First (SJF). When SJF scheduling policy is selected, you can use the file size information as the job length for scheduling purposes, assuming serving larger files will take longer. The schedulerthread will choose one of the requests in the ready queue according to the scheduling policyselected. The request will then be assigned to one of the available execution threads for service. There will be no scheduling done during the first t seconds after the myhttpd server is started. This time period will be used to accumulate some requests in the ready queue.

# SYCHNRONIZATION

To make sure that we protect the ready queue and other shared data structures across multiple threads to prevent race conditions. Mutex locks have been implemented for the same.
