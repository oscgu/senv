#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "util.h"

void
sec_free(char *mem, int size)
{
        memset(mem, 0, size);
        free(mem);
}


int
read_file(const char *file_path, Slice *out)
{
        FILE *fp = fopen(file_path, "rb");
        if (fp == NULL) {
                fprintf(stderr, "Error opening file: %s\n", file_path);
                exit(1);
        }

        struct stat st;
        stat(file_path, &st);

        int file_size = st.st_size;

        unsigned char *buff = malloc(file_size);
        fread(buff, sizeof(unsigned char), file_size, fp);
        fclose(fp);

        out->len = file_size;
        out->ptr = buff;

        return 0;
}
