#include "fs.h"

int diskfd;
char *path = "/home/TU/tug15645/File_System/file.txt";

int disk_init() {
    /* Initialize virtual disk */
    if ((diskfd = open(path, O_RDWR | O_CREAT)) == -1) {
        perror("error initialize disk");
        return EXIT_FAILURE;
    }

    if ((ftruncate(diskfd, fsize)) == -1) {
        perror("error set disk size");
        return EXIT_FAILURE;
    }

    if ((fstat(diskfd, &stat_buff)) == -1) {
        perror("error deteriming size\n");
        return EXIT_FAILURE;
    }

    if ((fs = mmap(NULL, stat_buff.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, diskfd, 0)) == MAP_FAILED) {
        perror("error mapping file to memory");
        return EXIT_FAILURE;
    }

    fat_init();

    table = calloc(1, sizeof(OpenFileTable));
    table->size = 0;
    return EXIT_SUCCESS;
}

int fs_create(char *filename, char *filepath, int isdir) {
    /*
    filename: name of the file to be created
    filepath: path to the file
    isdir: if the file is a directory
    returns: create a new file
    */
    int dirnum = get_dir(filepath); /* get the nearest parent directory's file number */

    /* if file is already created, just return success */
    if (get_filenum(dirnum, filename) > 0) {
        return EXIT_SUCCESS;
    }

    /* if file isn't created */
    int i;
    for (i = 1; i < nblock; i++) {
        if (fs->val[i] == -2) {
            /* when find an empty block, create a new entry for that file in its
             * nearest parent directory */
            fs->val[i] = -1;
            add_dir_entries(filename, i, dirnum, isdir);
            break;
        }
    }

    if (i == nblock) {
        puts("Disk is full");
        return EXIT_FAILURE;
    }

    if (msync(fs, fsize, MS_SYNC) == -1) {
        perror("Could not sync file to disk");
    }

    return EXIT_SUCCESS;
}

int fs_open(char *filepath, int isdir) {
    /*
    filepath: path to file
    isdir: openning a directory?
    returns: index into open file table
    */

    int filenum = -1;

    if (isdir) {
        filenum = get_dir(filepath);
    } else {
        char *filename = get_filename(filepath);
        int dirnum = get_dir(filepath);
        if (dirnum == -1) {
            return -1;
        }

        filenum = get_filenum(dirnum, filename);
        free(filename);
    }

    if (table->size == 0) {
        /* this is the first file we open */
        table->open[0].filenum = filenum;
        table->open[0].offset = 0;
        table->size += 1;
        /* return the first index into the table */
        return 0;
    } else {
        /* check if the file is in the table */
        int i = 0;
        for (; i < table->size; i++) {
            if (table->open[i].filenum == filenum) {
                /* return this index*/
                return i;
            }
        }
        if (i == table->size) {
            /* file isn't in the table, create a new entry */
            table->open[i].filenum = filenum;
            table->open[i].offset = 0;
            table->size += 1;
        }
        /* return the index of the newly created file */
        return i;
    }
}

int fs_write(int fd, char *buf, size_t count) {
    /*
    fd: index into open file table
    buf: a constant string to write to the file
    count: number of bytes desired to write to the file
    return: actual number of bytes written to file, perhaps if the disk becomes
    full, return -1 on error
    */

    /* get file number */
    int filenum = table->open[fd].filenum;

    /* checking */
    if (filenum < 0) {
        puts("file descriptor invalid");
        return -1;
    }

    if (fs->isdir[filenum]) {
        puts("cannot write to directory");
        return -1;
    }
    size_t r_count = MIN(count, strlen(buf));

    int index = filenum;
    int temp;

    /* get to the end of the file */
    while ((temp = fs->val[index]) > 0) {
        index = temp;
    }
    int write = 0;

    /* if the last block is full, find a new free block */
    int i = 1;
    while (write < r_count) {
        if (strlen(fs->data[index].fileBlock.content) == bsize) {
            for (; i < nblock; i++) {
                if (fs->val[i] == -2) {
                    fs->val[index] = i;
                    index = i;
                    fs->val[index] = -1;
                    fs->isdir[index] = 0;
                    break;
                }
            }
        }

        if (i == nblock) {
            puts("disk is full");
            return write;
        }

        /* start writing */
        int len = strlen(fs->data[index].fileBlock.content);
        for (; write < r_count; write++) {
            /* write character by character */
            fs->data[index].fileBlock.content[len + write] = buf[write];
            if ((len + write) == bsize - 1) {
                /* when the block is full */
                break;
            }
        }
        fs->data[index].fileBlock.content[len + write + 1] = 0;
    }

    if (msync(fs, fsize, MS_SYNC) == -1) {
        perror("Could not sync file to disk");
    }

    /* update size and modification time */
    update(filenum, write);

    return write;
}

int fs_read(int fd, char *buf, size_t count) {
    /*
    filenum: file number of file to open
    buf: a string to read data into
    count: number of characters want to read
    return: number of characters actually read, -1 on error
    */

    int filenum = table->open[fd].filenum;
    int offset = table->open[fd].offset;

    if (filenum < 0) {
        puts("file descriptor invalid");
        return -1;
    }

    int read = 0;
    if (fs->isdir[filenum] == 1) {
        /* read from directory */
        char *strDir = struct2str(filenum);
        int len = strlen(strDir);
        int r_count = MIN((len - offset), count);

        for (int i = offset; i < (r_count + offset); i++) {
            buf[read++] = strDir[i];
        }
        buf[read] = 0;
        table->open[fd].offset += read;
        free(strDir);

        return read;
    } else {
        /* read from file */
        /* locate position to start reading */
        int next = filenum;
        int startpos = offset;
        int len = strlen(fs->data[next].fileBlock.content);
        while (len > 0 && len - 1 < startpos) {
            next = fs->val[next];
            len = strlen(fs->data[next].fileBlock.content);
            startpos -= (len - 1);
        }
        // Now, either startpos < len - 1 or len == 0
        len -= (startpos + 1); // len is the length of the string from startpos in the datablock
        while (read < count) {
            /* start reading */
            int r_count = MIN(len, (count - read));
            for (int i = 0; i < r_count; i++) {
                buf[read++] = fs->data[next].fileBlock.content[i + startpos];
            }
            if (fs->val[next] < 0) {
                /* if the last block */
                buf[read] = 0;
                table->open[fd].offset += read;
                return read;
            }
            next = fs->val[next];
            len = strlen(fs->data[next].fileBlock.content);
            startpos = 0; // set startpos to 0 again
        }
        buf[read] = 0;
        table->open[fd].offset += read;
    }
    return read;
}

int fs_seek(int fd, size_t off) {
    /*
    fd: index into open file table
    off: offset to set
    */

    table->open[fd].offset = off;
    return EXIT_SUCCESS;
}

int fs_delete(char *filepath, int isdir) {
    /*
    filepath: path to file
    isdir: 0 for normal file and 1 for directory
    delete a file from the system
    */

    char *filename = get_filename(filepath);
    int len = strlen(filepath);
    int namelen = strlen(filename);
    char *path = calloc((len - namelen), sizeof(char));
    strncpy(path, filepath, len - namelen - 1);
    int dirnum = get_dir(path);
    int filenum = get_filenum(dirnum, filename);
    int next;

    if (isdir) {
        /* check if the directory is the root */
        if (filenum == 0) {
            puts("cannot delete root directory!");
            return EXIT_FAILURE;
        }

        /* check if the directory is empty */
        next = filenum;
        do {
            for (int i = 0; i < 4; i++) {
                if (strlen(fs->data[next].dirBlock.filename[i]) > 0 &&
                    strcmp(fs->data[next].dirBlock.filename[i], ".") != 0 &&
                    strcmp(fs->data[next].dirBlock.filename[i], "..") != 0) {
                    puts("directory not empty");
                    return EXIT_FAILURE;
                }
            }
            next = fs->val[next];
        } while (next > 0);
    }

    /* go to the parent directory and copy the last directory entry to replace */
    char *lastentry = get_last_entry(dirnum);
    char *entryptr;

    char *token = strtok_r(lastentry, ",", &entryptr);
    next = dirnum;
    do {
        for (int i = 0; i < 4; i++) {
            if (fs->data[next].dirBlock.filenum[i] == filenum) {
                strcpy(fs->data[next].dirBlock.filename[i], token);

                token = strtok_r(NULL, ",", &entryptr);
                fs->data[next].dirBlock.filenum[i] = atoi(token);
                token = strtok_r(NULL, ",", &entryptr);
                fs->data[next].dirBlock.size[i] = atoi(token);
                token = strtok_r(NULL, ",", &entryptr);
                fs->data[next].dirBlock.isdir[i] = atoi(token);
                token = strtok_r(NULL, ",", &entryptr);
                strcpy(fs->data[next].dirBlock.create_t[i], token);
                token = strtok_r(NULL, ",", &entryptr);
                strcpy(fs->data[next].dirBlock.mod_t[i], token);
            }
        }
        next = fs->val[next];
    } while (next > 0);
    fs->val[filenum] = -2;
    free(lastentry);
    free(filename);
    free(path);
    return EXIT_SUCCESS;
}

void fs_close(int *fd) { *fd = -1; }

int disk_close() {
    /* free all calloc */
    free(table);
    close(diskfd);
    munmap(fs, fsize);
    return EXIT_SUCCESS;
}
