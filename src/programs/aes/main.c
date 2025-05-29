#include <openssl/evp.h>
#include <openssl/rand.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define BLOCK_SIZE 16384  // 测试的块大小

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <N>\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    if (N <= 0) {
        fprintf(stderr, "N must be a positive integer\n");
        return 1;
    }

    // 初始化随机数生成器
    RAND_poll();

    // 生成随机密钥和IV
    unsigned char key[16], iv[16];
    if (!RAND_bytes(key, sizeof(key)) || !RAND_bytes(iv, sizeof(iv))) {
        fprintf(stderr, "Error generating key/IV\n");
        return 1;
    }

    // 准备测试数据
    unsigned char *plaintext = malloc(BLOCK_SIZE);
    unsigned char *ciphertext = malloc(BLOCK_SIZE + EVP_MAX_BLOCK_LENGTH);
    if (!plaintext || !ciphertext) {
        perror("malloc");
        free(plaintext);
        free(ciphertext);
        return 1;
    }
    RAND_bytes(plaintext, BLOCK_SIZE);  // 填充随机明文

    struct timeval start, end;
    gettimeofday(&start, NULL);

    // 执行N次加密操作
    for (int i = 0; i < N; i++) {
        EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            fprintf(stderr, "Context creation failed\n");
            break;
        }

        // 初始化加密上下文（AES-128-CBC）
        if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv)) {
            fprintf(stderr, "Encryption init failed\n");
            EVP_CIPHER_CTX_free(ctx);
            break;
        }

        int len, ciphertext_len = 0;
        // 执行加密
        if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, BLOCK_SIZE)) {
            fprintf(stderr, "Encryption failed\n");
            EVP_CIPHER_CTX_free(ctx);
            break;
        }
        ciphertext_len += len;

        // 处理填充
        if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + ciphertext_len, &len)) {
            fprintf(stderr, "Finalization failed\n");
            EVP_CIPHER_CTX_free(ctx);
            break;
        }
        ciphertext_len += len;

        EVP_CIPHER_CTX_free(ctx);  // 释放上下文
    }

    gettimeofday(&end, NULL);

    // 计算耗时
    double time_used = (end.tv_sec - start.tv_sec) + 
                      (end.tv_usec - start.tv_usec) / 1000000.0;

    printf("执行次数: %d\n", N);
    printf("总耗时: %.6f 秒\n", time_used);
    printf("平均每块耗时: %.6f 微秒\n", (time_used * 1e6) / N);
    printf("吞吐量: %.2f MB/s\n", 
          (BLOCK_SIZE) / (time_used * 1024 * 1024) * N);

    free(plaintext);
    free(ciphertext);
    return 0;
}