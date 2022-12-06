// usage
// gcc keygen.c -o keygen -lcrypto -lssl



#include <stdio.h>
#include <cpuid.h>
#include <openssl/md5.h>

int main() {
        unsigned int local_34;
        unsigned int local_30;
        unsigned int local_2c;
        unsigned int local_28;
        unsigned int local_18;
        unsigned int local_14;

        __get_cpuid(1, &local_34, &local_30, &local_2c, &local_28);


        local_18 = local_34 << 0x18 | local_34 >> 0x18 | (local_34 & 0xff00) << 8 | local_34 >> 8 & 0xff00;
        local_14 = local_28 << 0x18 | local_28 >> 0x18 | (local_28 & 0xff00) << 8 | local_28 >> 8 & 0xff00;

        printf("Your HWID is %08X%08X.\n", local_18, local_14);

        unsigned char buffer[17];
        snprintf(buffer, 17, "%08X%08X", local_18, local_14);


        MD5_CTX md5ctx;
        unsigned char digest[MD5_DIGEST_LENGTH];

        MD5_Init(&md5ctx);
        MD5_Update(&md5ctx, buffer, 16);
        MD5_Final(digest, &md5ctx);

        printf("Your license is ");
        for (unsigned int local_20 = 0; local_20 < 16; local_20 = local_20 + 1) {
                printf("%02x", digest[0xf - local_20]);
        }
        printf("\n");

        return 0;
}