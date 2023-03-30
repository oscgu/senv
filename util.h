/* structs */
typedef struct {
        unsigned char *ptr;
        long len;
} Slice;

/* functions */
void sec_free(char *mem, int size);
int read_file(const char *file_path, Slice *out);
