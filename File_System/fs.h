#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define fsize 1572864
#define bsize 512
#define nblock 2048
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

/* data structures */

typedef struct FileBlock {
    char content[bsize]; // 512 bytes to store the data
} FileBlock;

typedef struct DirBlock {
    char filename[4][52]; // 208 bytes
    int filenum[4];       // 16 bytes
    int size[4];          // 16 bytes
    int isdir[4];         // 16 bytes
    char create_t[4][32]; // 128 bytes
    char mod_t[4][32];    // 128 bytes
} DirBlock;

typedef union Block {
    FileBlock fileBlock;
    DirBlock dirBlock;
} Block;

typedef struct FAT {
    Block data[nblock];
    int isdir[nblock];
    int val[nblock]; // 4 bytes to store the next index, -1 if is the final block, -2 if free
} FAT;

typedef struct open_file {
    int filenum;
    int offset;
} OpenFileData;

typedef struct OpenFileTable {
    int size;
    OpenFileData open[nblock];
} OpenFileTable;

FAT *fs;

OpenFileTable *table;

struct stat stat_buff;

/* functionality */
int disk_init();
int fs_create(char *filename, char *filepath, int isdir);
int fs_open(char *filepath, int isdir);
int fs_write(int filenum, char *buf, size_t count);
int fs_read(int filenum, char *buf, size_t count);
int fs_seek(int filenum, size_t off);
int fs_delete(char *filepath, int isdir);
void fs_close(int *fd);
int disk_close();

/* helpers */
int fat_init();
int add_dir_entries(char *filename, int filenum, int dirnum, int isdir);
int default_dir_entry(int filenum, int parent_num, int index);
int get_filenum(int dirnum, char *filename);
int get_dir(char *filepath);
char *get_filename(char *filepath);
int update(int filenum, int write);
char *struct2str(int dirnum);
char *get_last_entry(int dirnum);
