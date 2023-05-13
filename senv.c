#include "crypto.h"
#include "util.h"
#include <getopt.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

char config_dir[256];

/* function prototypes */
int create_encrypted_file(const char *file_path, const char *plain_text);
int edit_encrypted_file(const char *file_path);
int view_encrypted_file(const char *file_path);

int migrate_plain_file(const char *migration_file, const char *out_file);
int inject_env(void);

int
view_encrypted_file(const char *file_path)
{
        Slice slice;
        read_file(file_path, &slice);

        char *pass = sec_pass_prompt();

        decrypt_file(file_path, pass, 64);

        free(pass);
        return 0;
}

// migrate a plain text file
int
migrate_plain_file(const char *migration_file, const char *out_file)
{
        Slice slice;
        read_file(migration_file, &slice);

        char *pass = sec_pass_prompt();

        encrypt_file(out_file, (char *) slice.ptr, slice.len, pass,
                     strlen(pass));

        free(pass);
        free(slice.ptr);

        return 0;
}

void
smkdir(char *dir)
{
        if (access(dir, F_OK) == -1 && 0 != mkdir(dir, 0700)) { 
            fprintf(stderr, "Could not create dir: %s\n", dir);
            exit(2); 
        }
}

void
setup()
{
        snprintf(config_dir, sizeof(config_dir), "%s/.config/%s",
                 getenv("HOME"), "senv");
        smkdir(config_dir);
}

void
print_help()
{
        printf("\n"
               "Usage:\n"
               "senv [operation] [project] [name]\n"
               "\n"

               "Operations:\n"
               "   create                        Create new senv file\n"
               "   edit                          Edit file\n"
               "   view                          View file\n"
               "   migrate -i [File]             Migrate an unencrypted file\n"
               "   inject                        Inject senv vars into your \n"
               "                                 current environment\n"
               "Example usage:\n"
               "    senv create test prod\n");
}

void
open_in_nvim(char *prefill, char *file_path)
{
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "nvim %s", file_path);
        if (system(cmd) == -1) {
                fprintf(stderr, "Could not open nvim\n");
                exit(1);
        }
}

int
mktmp()
{
        char filename[256] = "/tmp/senv-buff-XXXXXX";
        if (mkstemp(filename) == -1) {
                fprintf(stderr, "Failed to create tmp file\n");
                return 1;
        }

        Slice tmp;
        read_file(filename, &tmp);
        free(tmp.ptr);

        if (unlink(filename) == -1) {
                fprintf(stderr, "Failed to delete tmp file\n");
                return 1;
        }

        return 0;
}

typedef enum {
        UNKNOWN
} PARSE_RES;

typedef enum {
        VIEW,
        EDIT,
        CREATE,
        INJECT,
        MIGRATE
} OPERATION;
char operations[5][8] = {"view\0", "edit\0", "create\0", "inject\0",
                         "migrate\0"};

typedef enum {
        IN,
} FLAG;
char flags[1][3] = {"-i\0"};

int
flag_index(char **args, int len, char *flag)
{
        for (int i = 0; i < len; i++) {
                if (!strcmp(args[i], flag)) { return i; }
        }

        return -1;
}

int
string_to_enum(char table[5][8], int len, char *str)
{
        for (int i = 0; i < len; i++) {
                if (!strcmp(table[i], str)) { return i; }
        }

        return UNKNOWN;
}

void
parse_args(char **args, int arglen)
{
        OPERATION op;
        char *migration_file;
        int flag_i = -1;

        switch (arglen) {
        case 0: /* FALLTHROUGH */
        case 1:
        case 2:
        case 3:
                fprintf(stderr, "Not enough args provided\n");
                print_help();
                exit(1);
        case 6: /* FALLTHROUGH */
                flag_i = flag_index(args, arglen, "-i");
                if (flag_i > 0) { migration_file = *(args + flag_i + 1); }
        default:
                op = string_to_enum(operations, arglen, args[1]);
                if (op == MIGRATE && flag_i == -1) {
                    fprintf(stderr, "Missing file to migrate");
                    exit(1);
                }
                break;
        }

        /*
         * 1: op
         * 2: project
         * 3: name
         */
        char *cli_args[3];
        for (int i = 1, j = 0; i < arglen; i++) {
                if (i == flag_i || i == (flag_i + 1)) continue;
                cli_args[j++] = args[i];
                printf("%s\n", args[i]);
        }
        char *project = cli_args[1];
        char *name = cli_args[2];

        char file_path[strlen(config_dir) + strlen(project) + strlen(name) +
                       3 + 6];
        snprintf(file_path, sizeof(file_path), "%s/%s/%s.bin", config_dir,
                 project, name);

        char dir[256];
        int len = strlen(file_path) - (strlen(name) + 4);
        for (int i = 0; i < len; i++) {
                dir[i] = file_path[i];
        }
        dir[len] = '\0';


        smkdir(dir);


        switch (op) {
        case VIEW:
                view_encrypted_file(file_path);
                break;
        case EDIT:
        case CREATE:
        case INJECT:
        case MIGRATE:
                migrate_plain_file(migration_file, file_path);
                break;
        default:
                break;
        }
}

/* main */
int
main(int argc, char *argv[])
{
        setup();
        parse_args(argv, argc);
        return 0;
}
