#include "hashTable.h"

HashTable *loadDict(FILE *fp) {
    /* arguments:
    fp: file pointer to a dictionary file
    hash word in the dictionary into a hashtable
    returns: a pointer to the hash table
    */

    HashTable *hashTable = (HashTable *)malloc(sizeof(HashTable));
    if (hashTable == NULL) {
        printf("ERROR: Could not allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    hashTable->size = calSize(countLine(fp));
    hashTable->table = calloc(hashTable->size, sizeof(char *));

    if (hashTable->table == NULL) {
        printf("ERROR: Could not allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    char buffer[64];
    int index;
    int col;
    while (!feof(fp)) {
        fscanf(fp, "%s%*[^\n]", buffer);
        index = getHash(hashTable->size, buffer, strlen(buffer));

        while (hashTable->table[index] != NULL) {
            index = (index + 1) % hashTable->size;
            col++;
        }
        /* if we find an empty slot, fill it in with the new element */
        hashTable->table[index] = calloc((strlen(buffer) + 1), sizeof(char));

        if (hashTable->table[index] == NULL) {
            printf("ERROR: Could not allocate memory.\n");
            exit(EXIT_FAILURE);
        }
        strcpy(hashTable->table[index], buffer);
    }
    printf("Total collison: %d\n", col);
    return hashTable;
}

void hashTableFree(HashTable *hashTable) {
    int i;
    for (i = 0; i < hashTable->size; i++) {
        free(hashTable->table[i]);
    }
    free(hashTable->table);
    free(hashTable);
}

int findKey(HashTable *hashTable, char *key, int keyLen) {
    /* arguments:
    hashTable: pointer to the hash table
    key: word to look for
    keyLen: length of the key
    returns : the address of key if it exists and -1 otherwise
    */

    int addr = getHash(hashTable->size, key, keyLen);
    int count = 0;
    while (hashTable->table[addr] != NULL && count < hashTable->size) {
        if (strcmp(hashTable->table[addr], key) == 0) {
            return addr;
        }
        addr = (addr + 1) % hashTable->size;
        count++;
    }
    return -1;
}

int getHash(int hashSize, char *key, int keylen) {
    /* arguments:
    hashSize: size of the hash table
    key: word to be hashed
    keyLen: length of the key
    returns: the hashed value
    */

    const int constant = 41;
    int addr = 0;
    for (int i = 0; i < keylen; i++) {
        if (islower(key[i])) {
            addr = (addr * constant + ((unsigned char)key[i] - 96)) % hashSize;
        } else if (isupper(key[i])) {
            addr = (addr * constant + ((unsigned char)key[i] - 64)) % hashSize;
        } else {
            addr = (addr * constant + (unsigned char)key[i]) % hashSize;
        }
    }

    return addr;
}

int calSize(int numElems) {
    /* find the nearest prime that is larger than 2 * numElems */
    int M = numElems * 2;
    if (!isPrime(M))
        M++;
    return M;
}

int isPrime(int x) {
    for (int i = 2; i <= x / 2; i++) {
        if (x % i == 0) {
            return 0;
        }
    }
    return 1;
}

int countLine(FILE *fp) {
    int count = 0;
    long pos = ftell(fp);
    /* set the pointer to be at the beginning of the file */
    fseek(fp, 0, SEEK_SET);
    while (!feof(fp)) {
        if (fgetc(fp) == '\n')
            count++;
    }
    fseek(fp, pos, SEEK_SET);
    return count;
}