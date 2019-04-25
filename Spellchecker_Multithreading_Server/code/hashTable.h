#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>


typedef struct {
    char **table;
    int size;
} HashTable;


HashTable *loadDict(FILE *fp);
void hashTableFree(HashTable *table);
int findKey(HashTable *table, char *key, int keyLen);
int calSize(int numElems);
int getHash(int hashSize, char *key, int keyLen);
int countLine(FILE *fp);
int isPrime(int x);


