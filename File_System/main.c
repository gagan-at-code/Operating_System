#include "fs.h"

int main() {
    disk_init();

    fs_create("files", "/", 1);
    fs_create("test.txt", "/files", 0);
    fs_create("another_files", "/files", 1);
    fs_create("another_test.txt", "/files", 0);
    fs_create("toDel.txt", "/files/another_files", 0);

    int file_fd = fs_open("/files/test.txt", 0);
    fs_write(file_fd, "hello, world! My name is Allen. ", 32);

    int another_fd = fs_open("/files/another_test.txt", 0);
    fs_write(another_fd, "hello, world! I'm an AI. ", 32);

    fs_write(file_fd, "What is AI? ", 16);
    fs_write(another_fd, "AI is machine cognition! ", 32);

    int toDel = fs_open("/files/another_files/toDel.txt", 0);
    fs_write(toDel, "Nothing to write, this is to be deleted!", 64);

    char buf[33];
    fs_read(file_fd, buf, 32);
    printf("read string \'%s\' with length %lu\n", buf, strlen(buf));
    char abuf[21];
    fs_read(file_fd, abuf, 20);
    printf("read string \'%s\' with length %lu\n", abuf, strlen(abuf));

    char buf1[51];
    fs_read(another_fd, buf1, 50);
    printf("read string \'%s\' with length %lu\n", buf1, strlen(buf1));

    fs_close(&file_fd);
    fs_close(&another_fd);

    char dirread[1024];
    int dir_fd = fs_open("/files", 1);
    fs_read(dir_fd, dirread, 1024);
    printf("read string \'%s\' with length %lu\n", dirread, strlen(dirread));
    fs_seek(dir_fd, 0);

    fs_delete("/files/another_files", 1);
    fs_delete("/files/another_files/toDel.txt", 0);
    fs_read(dir_fd, dirread, 1024);
    printf("read string \'%s\' with length %lu\n", dirread, strlen(dirread));
    fs_delete("/files/another_files", 1);
    fs_seek(dir_fd, 0);

    char adirread[256];
    fs_read(dir_fd, adirread, 200);
    printf("read string \'%s\' with length %lu\n", adirread, strlen(adirread));
    fs_close(&dir_fd);

    disk_close();
}