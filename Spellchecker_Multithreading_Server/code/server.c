#define _GNU_SOURCE
#include "hashTable.h"
#include "queue_t.h"
#include "wrappers.h"
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>

#define NUM_WORKERS 5
#define BUFFSIZE 20
#define MAXLINE 64
#define DEFAULT_PORT "5555"
#define DEFAULT_FILE "/vagrant/Spellchecker_Multithreading_Server/dict.txt" // change to the directory you store the file

void *serveClient();
void spellChecker(int connfd);
ssize_t readLine(int fd, char *buffer, size_t n);

queue_t buffer;
HashTable *dictTable;

int main(int argc, char *argv[]) {
    /* read in the dictionary */

    FILE *dictFile;
    const char *fileName;
    const char *portNum;
    portNum = argc > 1 ? argv[1] : DEFAULT_PORT;
    fileName = argc > 2 ? argv[2] : DEFAULT_FILE;
    
    if ((dictFile = fopen(fileName, "r")) == 0) {
        printf("ERROR: Openning dictionary file\n");
        exit(EXIT_FAILURE);
    }

    dictTable = loadDict(dictFile);
    fclose(dictFile);
    printf("Hash Size: %d\n", dictTable->size);

    int listenfd, connfd, port;
    socklen_t clientlen = sizeof(struct sockaddr_in);
    struct sockaddr_in clientaddr;
    pthread_t tid;

    port = atoi(portNum);
    queue_init(&buffer, BUFFSIZE);
    listenfd = open_listenfd(port);

    for (int i = 0; i < NUM_WORKERS; i++) {
        pthread_create(&tid, NULL, serveClient, (void *)&i);
    }

    while (1) {
        connfd = Accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
        enqueue(&buffer, connfd);
    }
}

void *serveClient() {
    Pthread_detach(pthread_self());
    /*
    in MacOS
    uint64_t tid;
    pthread_threadid_np(NULL, &tid);
    */
    pid_t tid = syscall(__NR_gettid);
    while (1) {
        int connfd = dequeue(&buffer);
        printf("Thread id: %d\n", tid);
        /* spell check goes here */
        spellChecker(connfd);
        Close(connfd);
    }
}

void spellChecker(int connfd) {
    char buf[MAXLINE];
    ssize_t bytes_read;
    int correct = 0;
    char *res;
    while ((bytes_read = readLine(connfd, buf, MAXLINE - 1)) > 0) {
        correct = findKey(dictTable, buf, strlen(buf));
        if (correct < 0) {
            res = " wrong\n";
            strcat(buf, res);
            write(connfd, buf, strlen(buf));
        } else {
            res = " OK\n";
            strcat(buf, res);
            write(connfd, buf, strlen(buf));
        }
    }
}

ssize_t readLine(int fd, char *buffer, size_t n) {
    ssize_t numRead; /* # of bytes fetched by last read() */
    size_t totRead;  /* Total bytes read so far */
    char *buf;
    char ch;

    if (n <= 0 || buffer == NULL) {
        errno = EINVAL;
        return -1;
    }

    buf = buffer; /* No pointer arithmetic on "void *" */

    totRead = 0;
    for (;;) {
        numRead = read(fd, &ch, 1);
        if (numRead == -1) {
            if (errno == EINTR) /* Interrupted --> restart read() */
                continue;
            else
                return -1; /* Some other error */

        } else if (numRead == 0) { /* EOF */
            if (totRead == 0)      /* No bytes read; return 0 */
                return 0;
            else /* Some bytes read; add '\0' */
                break;

        } else {                                                 /* 'numRead' must be 1 if we get here */
            if (totRead < n - 1 && !isspace(ch) && ch != '\n') { /* Discard > (n - 1) bytes */
                totRead++;
                *buf++ = ch;
            }

            if (ch == '\n') {
                break;
            }
        }
    }
    *buf = '\0';
    return totRead;
}
