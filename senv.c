#include "crypto.h"
#include "util.h"
#include <getopt.h>
#include <linux/limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

char config_dir[256];

enum FLAG {
    CREATE,
    EDIT,
    VIEW,
    MIGRATE,
    INJECT
};

/* function prototypes */
int create_encrypted_file(const char *file_path, const char *plain_text);
int edit_encrypted_file(const char *file_path);
int view_encrypted_file(const char *file_path);

int migrate_plain_file(const char *migration_file, const char *out_file);
int inject_env(void);

int view_encrypted_file(const char *file_path) {
    Slice slice;
    read_file(file_path, &slice);

    char *pass = sec_pass_prompt();

    decrypt_file(file_path, pass, 64);

    free(pass);
    return 0;
}

int migrate_plain_file(const char *migration_file, const char *out_file) {
    Slice slice;
    read_file(migration_file,&slice);

    char *pass = sec_pass_prompt();

    encrypt_file(out_file, (char *)slice.ptr, slice.len, pass, strlen(pass));

    free(pass);
    free(slice.ptr);

    return 0;
}

void
smkdir(char *dir) {
    if (access(dir, F_OK) == -1 &&
        0 != mkdir(dir, S_IRUSR | S_IWUSR)) {
            exit(2);
    }
}

void
setup() {
    snprintf(config_dir, sizeof(config_dir), "%s/.config/%s",
             getenv("HOME"), "senv");
    smkdir(config_dir);
}

void
print_help()
{
        printf(
                "                                             \n"
                "    /$$$$$$$  /$$$$$$  /$$$$$$$  /$$    /$$  \n"
                "   /$$_____/ /$$__  $$| $$__  $$|  $$  /$$/  \n"
                "  |  $$$$$$ | $$$$$$$$| $$  \\ $$ \\  $$/$$/ \n"
                "   \\____  $$| $$_____/| $$  | $$  \\  $$$/  \n"
                "   /$$$$$$$/|  $$$$$$$| $$  | $$   \\  $/    \n"
                "  |_______/  \\_______/|__/  |__/    \\_/    \n"
                "                                             \n"

                "Usage:                         \n"
                "senv [flag] [project] [label]  \n"
                "                               \n"

                "Flags:                                                                       \n"
                "   -c                          Create new senv file                          \n"
                "   -e                          Edit file                                     \n"
                "   -v                          View file                                     \n"
                "   -m [file]                   Migrate an unencrypted file                   \n"
                "   -i                          Inject senv vars into your current environment\n"
        );
}

void
open_in_nvim(char *prefill, char *file_path) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "nvim %s", file_path);
    if (system(cmd) == -1) {
        fprintf(stderr, "Could not open nvim\n");
        exit(1);
    }
}

int
mktmp() {
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


/* main */
int
main(int argc, char *argv[])
{
        setup();
        /*
            char *file = "test.bin";
            snprintf(file_path, sizeof(file_path), "%s/%s", config_dir, file);


            char buff[64] = "test it works!!! ??";
            char pass[] = "test123";

            encrypt_file(file_path, buff, sizeof(buff), pass, sizeof(pass));
            decrypt_file(file_path, pass, sizeof(pass));
            */

        int c;
        int flag_count;
        enum FLAG flag;

        char *migration_file;
        extern char *optarg; extern int optind;

        while ((c = getopt(argc, argv, "cevm:i")) != -1) {
                switch (c) {
                case 'c':
                        flag_count++;
                        flag = CREATE;
                        break;
                case 'e':
                        flag_count++;
                        flag = EDIT;
                        break;
                case 'v':
                        flag_count++;
                        flag = VIEW;
                        break;
                case 'm':
                        flag_count++;
                        migration_file = optarg;
                        flag = MIGRATE;
                        break;
                case 'i':
                        flag_count++;
                        flag = INJECT;
                        break;
                default:
                        print_help();
                        exit(2);
                }
        }

        if (flag_count == 0) {
            print_help();
            exit(2);
        }

        if (flag_count > 1) {
                fprintf(stderr, "Only one flag allowed\n");
                print_help();
                exit(2);
        }

        
        if (argc - optind != 2) {
            fprintf(stderr, "Missing either project or name");
            exit(2);
        }

        char *project = argv[optind++];
        char *name = argv[optind++];

        char file_path[PATH_MAX];
        snprintf(file_path, sizeof(file_path), "%s/%s/%s.bin", config_dir, project, name);

        smkdir((char *)(&file_path - (sizeof(name) + 4)));

        switch (flag) {
            case EDIT:
                break;
            case CREATE:
                break;
            case VIEW:
                view_encrypted_file(file_path);
                break;
            case INJECT:
                break;
            case MIGRATE:
                migrate_plain_file(migration_file, file_path);
                break;
        }

        return 0;
}
