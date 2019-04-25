## Description of the Problem 

In this project, we need to create a spell-checker server that is able to handle multiple connections or services in parallel. 

First, to implement a spell-checker, we need to store the dictionary into a data structure that can be easily accessed, i.e., searching for a element in such data structure should cost as low as possible. One simple option is to implement a HashTable, where we can put all words into. The implementation of such hashtable is in hashTable.h and hashTable.c

Second, to implement a multithreading server, we're going to implement a prethreaded concurrent server. The server consists of a main thread and a set of worker threads. The main thread repeatedly accepts connection request from clients and places the resulting connected descriptors in a bounded queue. Each worker thread repeatedly removes a descriptor from the buffer, services the client, and then waits for the next descriptor. The implementation of the work queue is in queue_t.c and queue_t.h, and we also provide wrappers for socket interface in wrappers.c and wrappers.h. As a side note, because MacOS doesn't implement unamed semaphore, so I have to use sem_open and sem_unlink instead of sem_init

