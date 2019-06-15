# File System

## Introduction

In this assignment, we have to design a file system that can store (sub)directories and files.  

Requirements are: 

* Files can be up to 65536 bytes in size; data blocks are each 512 bytes
* Directory can be extensible
* Metadata for every entry in the directory includes a file name, file size, creation date/time, last modification date/time 
* Metadata for directory includes size, last modfication date/time, path to the directory
* Functionality includes: create a file, delete a file, open a file, close a file.
* All I/O in the file system should be performed using "memory=mapped file" functionality, where your file system is mapped into memory and manipulation of the filesystem is directly through memory.

**Premiliary**: to achieve our goal, first we need to create a virtual disk. I will create a file.txt file with size 1MB by typing the following command:

```
$ dd if=/dev/zero of=output.dat  bs=1M  count=1
```

Then, we need to make use of mmap() system call and facility. The memory mapped file facility lets us address locations in the file and read and write the locations as if you are directly addressing memory locations:

```C
int fd;
char *buff;
struct stat stat_buff; 

if ((fd=open(path, O_RDONLY)) == -1) {
    perror("error opening %s for reading", path);
    return FILE_OPEN_FAILURE;
} 

if ((fstat(fd, &stat_buff)) == -1) {
    perror("error determining the size of %s", path);
    return STAT_FAILURE;
}

if ((buff=mmap(NULL, stat_buff.st_size, PROT_READ, MAP_PRIVATE, fd, 0))==MAP_FAILURE) {
    perror("error mapping input");
    return MMAP_FAILURE;
}

```

## Design 

I choose to implement FAT as a design for my file system. To implement FAT, we need an array of integers and an array of structs or two arrays of structs. First, we need to create the data structures and divide the data blocks so that each is 512 bytes.

Then, in **case 1: an array of integers and an array of structs**. I use an array of integers to keep track of the FAT and an array of structs to keep track of the data block. Create 1024 blocks. For each block, let -2 denotes that the block is free, -1 denotes that the block is the ending block of a file, and any nonnegative number to be the next block of the file that that block belongs.

For example, let:
```
FAT = {-2, -2, -2, -2, -2};
```

Now, when we create the root directory, it will take the first block in the FAT, so, the FAT array becomes 
```
FAT = {-1, -2, -2, -2, -2}; 
```

In the first data block, because the there is no data right now, so the data will be empty, but we will store metadata for the directory. It will look something like: 

```
". 0 metadata"
```

Now suppose we create a file, file1, inside the root directory, and the file number for this file is the index of the FAT block, then the FAT will look like:
```
FAT = {-1, -1, -2, -2, -2};
```

In the data blocks, the block for the the directory looks like a string that:
```
". 0 metadata
file1 1 metadata"
```

The block for file1 store the data of whatever we write into it. Suppose now, we write something in file1 that is bigger than 512 bytes, then, the FAT looks like:
```
FAT = {-1, 2, -1, -2, -2};
```

In the data blocks, the block for the directory looks like a string that:
```
". 0 metadata
file1 1 metadata"
```

or in **case 2: 2 arrays of structs**. Instead of using an array to store the FAT, we can use an array of structs, where the struct contains 2 integers: one for the index and one for the next block of the file. 

Because the maximum size of a file is 65536 bytes, then we need to limit a file to has at most 128 data blocks. With this design and mmap, we are guaranteed that directory and files (maximum for file is 65536) can have variable length and directly write to the memory location.

## Functionality
Functions that we need to implement are:

```
int k_creat(char *path);
int k_open(char *path, int mode);
int k_delete(char *path);
int k_close(char *path);
```

Notice that: k_creat(path) just "k_open(path, O_WRONLY|O_CREAT|O_TRUNC)". First, I implement k_creat, then k_open because we cannot open a file if we don't have one, then, implement k_close and k_delete. When we create a file, we need to find a block in the FAT that is free, then allocate the corresponding data block to that file. After getting the file number, we put the file name, file number and metadata into the directory's data. Whenever we open a file and modify it, we need to keep track of the last modification time as well as the pointer to the location of the file from the previous access. When we delete a file, we have to loop through the data block in the directory and wipe out that file's metadata and then mark the data block(s) of the file to be -2 (free). 

Because all I/O in the file system should be performed using "memory=mapped file", we also need to write a k_write and k_read functions that directly read and write to the memory. Functions such as k_read, k_write, k_delete can directly modify the data in the data block, the FAT, and the file's metadata stored in its directory's data block.

## Testing

To test the program, we create each feature and test it before adding a new feature in:

* First, we create a virtual disk, create the data structures, divide the memory into 1024 data blocks, each with 512 bytes. Then we create the root directory, which is the first block in the FAT.
* Then, we write a function, k_creat(char *path) and use this functionality to create a file in the root directory and see if it is working. 
* Since deleting a file is also relatively easy, so we write a k_delete function to delete our file and test it.
* Then, we write a function, k_open(char *path, int mode) and use this function to open our created file.
* Then, we write a k_write function to write to the file and test it.
* Then, we write a k_read function to read from the file and test it.
* Lastly, we write a k_close function to close our opened file and test it.