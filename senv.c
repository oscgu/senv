#include "crypto.h"
#include <getopt.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>

/* global variables */

/*

static struct option commands[] = {{"create", required_argument, 0, 'c'},
                                   {"view", required_argument, 0, 'v'},
                                   {"inject", required_argument, 0, 'i'},
                                   {"migrate", required_argument, 0, 'm'},
                                   {0, 0, 0, 0}};
                                   */

/* function prototypes */
int create_encrypted_file(const unsigned char *name, unsigned char *key);
int edit_encrypted_file(const unsigned char *name, unsigned char *key);
int view_encrypted_file(const unsigned char *name, unsigned char *key);

int migrate_plaintext(const unsigned char *name, unsigned char *key);
int inject_env(void);

/* main */
int
main(int argc, char *argv[])
{
        char config_dir[256];
        snprintf(config_dir, sizeof(config_dir), "%s/.config/%s",
                 getenv("HOME"), "senv");
        char *file = "test.bin";

        char file_path[PATH_MAX];
        snprintf(file_path, sizeof(file_path), "%s/%s", config_dir, file);

        char buff[64] = "test it works!!!";

        char pass[] = "test";

        encrypt_file(file_path, buff, 64, "test", sizeof(pass));
        decrypt_file(file_path, pass, sizeof(pass));

        return 0;
}
