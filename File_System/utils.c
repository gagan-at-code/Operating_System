#include "fs.h"

int fat_init() {
    /* initialize the root directory.
    The root directory will take block 0 on the FAT, and this remains throughout the whole filesystem */
    fs->val[0] = -1;
    fs->isdir[0] = 1;
    sprintf(fs->data[0].dirBlock.filename[0], ".");
    fs->data[0].dirBlock.filenum[0] = 0;
    fs->data[0].dirBlock.size[0] = 0;
    fs->data[0].dirBlock.isdir[0] = 1;
    time_t t;
    time(&t);
    char *create = ctime(&t);
    create[strlen(create) - 1] = 0;
    strcpy(fs->data[0].dirBlock.create_t[0], create);
    strcpy(fs->data[0].dirBlock.mod_t[0], create);

    /* mark all block in FAT as free */
    for (int i = 1; i < nblock; i++) {
        fs->val[i] = -2;
    }

    if (msync(fs, fsize, MS_SYNC) == -1) {
        perror("Could not sync file to disk");
    }

    return EXIT_SUCCESS;
}

int add_dir_entries(char *filename, int filenum, int dirnum, int isdir) {
    /*
    filename: name of file
    filenum: file number
    dirnum: file number of the directory that we put the file in
    isdir: if the file itself is also a directory
    returns: add that filename, file number and metadata into its parent directory, look for a new block
    for the directory if the current block for the directory is full.
    */

    for (int i = 0; i < 4; i++) {
        if (strlen(fs->data[dirnum].dirBlock.filename[i]) == 0) {
            strcat(fs->data[dirnum].dirBlock.filename[i], filename);
            fs->data[dirnum].dirBlock.filenum[i] = filenum;
            fs->data[dirnum].dirBlock.size[i] = 0;
            fs->data[dirnum].dirBlock.isdir[i] = isdir;

            time_t t;
            time(&t);
            char *create = ctime(&t);
            create[strlen(create) - 1] = 0;
            strcpy(fs->data[dirnum].dirBlock.create_t[i], create);
            strcpy(fs->data[dirnum].dirBlock.mod_t[i], create);

            fs->isdir[filenum] = isdir;

            if (isdir) {
                default_dir_entry(filenum, dirnum, i);
            }

            return EXIT_SUCCESS;
        }
    }

    /* if the current block for the directory is full, find a new block */
    for (int i = 1; i < nblock; i++) {
        if (fs->val[i] == -2) {
            fs->val[dirnum] = i; // the next block of directory to this new block
            dirnum = i;
            fs->val[dirnum] = -1;
            fs->isdir[dirnum] = 1;

            strcat(fs->data[dirnum].dirBlock.filename[0], filename);
            fs->data[dirnum].dirBlock.filenum[0] = filenum;
            fs->data[dirnum].dirBlock.size[0] = 0;
            fs->data[dirnum].dirBlock.isdir[0] = isdir;

            time_t t;
            time(&t);
            char *create = ctime(&t);
            create[strlen(create) - 1] = 0;
            strcpy(fs->data[dirnum].dirBlock.create_t[0], create);
            strcpy(fs->data[dirnum].dirBlock.mod_t[0], create);
            fs->isdir[filenum] = isdir;

            if (isdir) {
                default_dir_entry(filenum, dirnum, i);
            }

            return EXIT_SUCCESS;
        }
    }
    return EXIT_FAILURE;
}

int default_dir_entry(int filenum, int parent_num, int index) {
    /*
    filenum: file number of the newly created directory
    parent_num: file number of the parent directory
    add default entry for a directory
    */

    /* information about the directory itself */
    strcpy(fs->data[filenum].dirBlock.filename[0], ".");
    fs->data[filenum].dirBlock.filenum[0] = filenum;
    fs->data[filenum].dirBlock.size[0] = 0;
    fs->data[filenum].dirBlock.isdir[0] = 1;
    strcpy(fs->data[filenum].dirBlock.create_t[0], fs->data[parent_num].dirBlock.create_t[index]);
    strcpy(fs->data[filenum].dirBlock.mod_t[0], fs->data[parent_num].dirBlock.mod_t[index]);

    /* information about the parent directory */
    strcpy(fs->data[filenum].dirBlock.filename[1], "..");
    fs->data[filenum].dirBlock.filenum[1] = fs->data[parent_num].dirBlock.filenum[0];
    fs->data[filenum].dirBlock.size[1] = 0;
    fs->data[filenum].dirBlock.isdir[1] = 1;
    strcpy(fs->data[filenum].dirBlock.create_t[1], fs->data[parent_num].dirBlock.create_t[0]);
    strcpy(fs->data[filenum].dirBlock.mod_t[1], fs->data[parent_num].dirBlock.mod_t[0]);

    return EXIT_SUCCESS;
}

int get_filenum(int dirnum, char *filename) {
    /*
    dirnum: filenumber for the directory
    filename: name of file to look for
    returns: file number of file to look for
    */

    if (fs->isdir[dirnum] == 0) {
        /* if not a directory */
        return -1;
    }
    int i;
    int next = dirnum;
    do {
        dirnum = next;
        i = 0;
        while (fs->data[dirnum].dirBlock.filename[i] != NULL) {
            if (strcmp(fs->data[dirnum].dirBlock.filename[i], filename) == 0) {
                return fs->data[dirnum].dirBlock.filenum[i];
            }
            i++;
            if (i == 4) {
                break;
            }
        }
        next = fs->val[dirnum];
    } while (next > 0);
    return -1;
}

int get_dir(char *filepath) {
    /*
    filepath: path to file
    return: file number of the last parent directory
    */

    char *pathptr = NULL;
    char temp[strlen(filepath)];
    strcpy(temp, filepath);
    char *dir = strtok_r(temp, "/", &pathptr);

    int parent_dir = 0;
    int next = 0;

    while (dir != NULL) {
        if ((next = get_filenum(parent_dir, dir)) < 0) {
            puts("file not found in directory");
            return -1; // not found;
        }
        if (fs->isdir[next] == 0) {
            return parent_dir;
        }

        parent_dir = next;
        dir = strtok_r(NULL, "/", &pathptr);
    }
    return parent_dir;
}

char *get_filename(char *filepath) {
    /*
    filepath: path to a file
    return: file name
    */
    int len = strlen(filepath);
    char temp[len];
    strcpy(temp, filepath);
    char *ptr = strrchr(temp, '/');

    char *filename = calloc(strlen(ptr) - 1, sizeof(char));
    strcpy(filename, ptr + 1);
    return filename;
}

int update(int filenum, int write) {
    /*
    filenum: file number for the file just get written to
    write: number of bytes written to the file
    returns: add written to
    */
    int dirnum = 0;
    int index = 0;

    for (int i = 0; i < nblock; i++) {
        if (fs->isdir[i]) {
            for (int j = 0; j < 4; j++) {
                if (fs->data[i].dirBlock.filenum[j] == filenum) {
                    dirnum = i;
                    index = j;
                    break;
                }
            }
        }
    }

    /* update size and modification time */
    fs->data[dirnum].dirBlock.size[index] += write;
    time_t t;
    time(&t);
    char *mod = ctime(&t);
    mod[strlen(mod) - 1] = 0;
    strcpy(fs->data[dirnum].dirBlock.mod_t[index], mod);

    return EXIT_SUCCESS;
}

char *struct2str(int dirnum) {
    /* represent data in directory block as a string */
    char *strDir = calloc(1, 512 * sizeof(char));
    DirBlock dir;
    char *temp[4];
    int j = 1;
    int next = dirnum;
    do {
        strDir = realloc(strDir, 512 * j * sizeof(char));
        /* get length of string required to hold struct values */
        dir = fs->data[next].dirBlock;
        size_t len[4] = {0};
        for (int i = 0; i < 4; i++) {
            if (strlen(dir.filename[i]) == 0) {
                continue;
            }
            len[i] = snprintf(NULL, len[i], "%s,%d,%d,%d,%s,%s\n", dir.filename[i], dir.filenum[i], dir.size[i],
                              dir.isdir[i], dir.create_t[i], dir.mod_t[i]);
            temp[i] = calloc(1, sizeof(char) * len[i]);
        }
        for (int i = 0; i < 4; i++) {
            if (strlen(dir.filename[i]) == 0) {
                continue;
            }
            snprintf(temp[i], len[i] + 1, "%s,%d,%d,%d,%s,%s\n", dir.filename[i], dir.filenum[i], dir.size[i],
                     dir.isdir[i], dir.create_t[i], dir.mod_t[i]);
            strcat(strDir, temp[i]);
        }
        j++;
        next = fs->val[next];
    } while (next > 0);

    return strDir;
}

char *get_last_entry(int dirnum) {
    /*
    get the last entry for a directory with filenumber dirnum
    */
    int next = dirnum;
    int parent = dirnum;
    int temp = dirnum;

    while (temp > 0) {
        parent = next;
        next = temp;
        temp = fs->val[temp];
    }
    int last = 0;

    int i;
    for (i = 0; i < 4; i++) {
        if (strlen(fs->data[next].dirBlock.filename[i]) > 0) {
            last = i;
        }
    }

    int len = 0;
    len = snprintf(NULL, len, "%s,%d,%d,%d,%s,%s", fs->data[next].dirBlock.filename[last],
                   fs->data[next].dirBlock.filenum[last], fs->data[next].dirBlock.size[last],
                   fs->data[next].dirBlock.isdir[last], fs->data[next].dirBlock.create_t[last],
                   fs->data[next].dirBlock.mod_t[last]);

    char *strDir = calloc(len + 1, sizeof(*strDir));
    snprintf(strDir, len + 1, "%s,%d,%d,%d,%s,%s", fs->data[next].dirBlock.filename[last],
             fs->data[next].dirBlock.filenum[last], fs->data[next].dirBlock.size[last],
             fs->data[next].dirBlock.isdir[last], fs->data[next].dirBlock.create_t[last],
             fs->data[next].dirBlock.mod_t[last]);

    /* delete this entry */
    memset(fs->data[next].dirBlock.filename[last], 0, strlen(fs->data[next].dirBlock.filename[last]));
    fs->data[next].dirBlock.filenum[last] = 0;
    fs->data[next].dirBlock.size[last] = 0;
    fs->data[next].dirBlock.isdir[last] = 0;
    memset(fs->data[next].dirBlock.create_t[last], 0, strlen(fs->data[next].dirBlock.create_t[last]));
    memset(fs->data[next].dirBlock.mod_t[last], 0, strlen(fs->data[next].dirBlock.mod_t[last]));

    if (i == 0 && next != parent) {
        fs->val[parent] = -1;
    }

    return strDir;
}
