int encrypt_file(const char *file_path, const char *plaintext,
                 int plaintext_len, const char *plain_pass,
                 int plain_pass_len);
int decrypt_file(const char *file_path, const char *plain_pass,
                 int plain_pass_len);
int encrypt(unsigned char *plaintext, int plaintext_len,
            const unsigned char *key, const unsigned char *iv,
            unsigned char *ciphertext);
int decrypt(unsigned char *ciphertext, int ciphertext_len,
            const unsigned char *key, const unsigned char *iv,
            unsigned char *plaintext);

void handleErrors(void);
char *sec_pass_prompt();
