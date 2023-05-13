#include "crypto.h"
#include "util.h"
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/conf.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <sys/stat.h>
#include <unistd.h>

const char IV_LEN = 16;
const char BLOCK_SIZE = 16;
const char MAX_PASSWORD_LEN = 64;

/* function implementations */
char *
sec_pass_prompt()
{
        char *plain_pass = malloc(MAX_PASSWORD_LEN);

        plain_pass = getpass("Password: ");

        return plain_pass;
}

void
handleErrors(void)
{
        ERR_print_errors_fp(stderr);
        abort();
}

unsigned char *pass_to_key(const char *pass, int pass_len, unsigned char *salt,
                           int salt_len);

unsigned char *
pass_to_key(const char *pass, int pass_len, unsigned char *salt, int salt_len)
{
        int key_len = 32;
        unsigned long long int iterations = 10000;
        unsigned char *buff = malloc(key_len + 1);

        if (1 != PKCS5_PBKDF2_HMAC(pass, pass_len, salt, salt_len, iterations,
                                   EVP_sha256(), key_len, buff))
                handleErrors();

        buff[key_len] = '\0';

        return buff;
}

int
encrypt_file(const char *file_path, const char *plaintext, int plaintext_len,
             const char *plain_pass, int plain_pass_len)
{
        unsigned char text_iv[IV_LEN];
        if (1 != RAND_bytes(text_iv, sizeof(text_iv))) handleErrors();

        unsigned char key_iv[IV_LEN];
        if (1 != RAND_bytes(key_iv, sizeof(key_iv))) handleErrors();

        unsigned char *key =
            pass_to_key(plain_pass, plain_pass_len, key_iv, sizeof(key_iv));
        printf("\nEncrypt key: ");
        for (int i = 0; i < 32; i++) {
                printf("%02X", key[i]);
        }
        printf("\n");

        unsigned char ciphertext[4096];
        int ciphertext_len = encrypt((unsigned char *) plaintext,
                                     plaintext_len, key, text_iv, ciphertext);

        printf("cipherlen: %d\n", ciphertext_len);

        FILE *fp = fopen(file_path, "wo");
        if (fp == NULL) {
                fprintf(stderr, "Error opening file: %s\n", file_path);
                exit(1);
        }

        fwrite(text_iv, sizeof(unsigned char), sizeof(text_iv), fp);
        printf("En textiv: ");
        for (int i = 0; i < IV_LEN; i++) {
                printf("%02X", text_iv[i]);
        }

        printf("\nEn keyiv : ");
        fwrite(key_iv, sizeof(unsigned char), sizeof(key_iv), fp);
        for (int i = 0; i < IV_LEN; i++) {
                printf("%02X", key_iv[i]);
        }

        printf("\nEn text  : ");
        for (int i = 0; ciphertext[i] != '\0'; i++) {
                printf("%02X", ciphertext[i]);
        }

        printf("\n");
        fwrite(ciphertext, sizeof(unsigned char), ciphertext_len, fp);
        printf("Text len: %d\n", ciphertext_len);

        fclose(fp);
        free(key);

        return 0;
}

int
decrypt_file(const char *file_path, const char *plain_pass, int plain_pass_len)
{
        Slice encrypted_file;
        read_file(file_path, &encrypted_file);

        printf("\nDe textiv: ");
        unsigned char text_iv[IV_LEN];
        unsigned int i = 0;
        for (; i < 16; i++) {
                printf("%02X", encrypted_file.ptr[i]);
                text_iv[i] = encrypted_file.ptr[i];
        }

        printf("\nDe keyiv : ");
        unsigned char key_iv[IV_LEN];
        for (; i < 32; i++) {
                printf("%02X", encrypted_file.ptr[i]);
                key_iv[i - 16] = encrypted_file.ptr[i];
        }

        printf("\nDe text  : ");
        for (; encrypted_file.ptr[i] != '\0'; i++) {
                printf("%02X", encrypted_file.ptr[i]);
        }

        unsigned char *key =
            pass_to_key(plain_pass, plain_pass_len, key_iv, sizeof(key_iv));

        printf("\nDecrypt key: ");
        for (int i = 0; i < 32; i++) {
                printf("%02X", key[i]);
        }
        printf("\n");
        printf("\n");
        fflush(stdout);

        unsigned char *ciphertext = &encrypted_file.ptr[32];
        long ciphertext_len = encrypted_file.len - 32;
        printf("Text len: %ld\n", encrypted_file.len - 32);

        unsigned char decryptedtext[256];
        int decryptedtext_len =
            decrypt(ciphertext, ciphertext_len, key, text_iv, decryptedtext);
        printf("decryptedlen: %d\n", decryptedtext_len);

        free(key);
        free(encrypted_file.ptr);

        decryptedtext[decryptedtext_len] = '\0';
        printf("\nRes: %s\n", decryptedtext);

        return 0;
}

int
encrypt(unsigned char *plaintext, int plaintext_len, const unsigned char *key,
        const unsigned char *iv, unsigned char *ciphertext)
{
        EVP_CIPHER_CTX *ctx;
        int len, ciphertext_len;

        if (!(ctx = EVP_CIPHER_CTX_new())) handleErrors();

        if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
                handleErrors();

        if (1 !=
            EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
                handleErrors();
        ciphertext_len = len;

        if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
                handleErrors();
        ciphertext_len += len;

        EVP_CIPHER_CTX_free(ctx);

        return ciphertext_len;
}

int
decrypt(unsigned char *ciphertext, int ciphertext_len,
        const unsigned char *key, const unsigned char *iv,
        unsigned char *plaintext)
{
        EVP_CIPHER_CTX *ctx;
        int len, plaintext_len;

        if (!(ctx = EVP_CIPHER_CTX_new())) handleErrors();

        if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
                handleErrors();

        if (1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext,
                                   ciphertext_len))
                handleErrors();

        plaintext_len = len;

        if (1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
                handleErrors();
        plaintext_len += len;

        EVP_CIPHER_CTX_free(ctx);

        return plaintext_len;
}
