#include <stdio.h>
#include <openssl/evp.h>
#include <string.h>

//#define EVP_MAX_MD_SIZE 36

int main(int argc, char *argv[])
{
    EVP_MD_CTX mdctx;
    const EVP_MD *md;
    char mess1[] = "Test Message\n";
    char mess2[] = "Hello World\n";
    char mess3[] = "10.0.1.7";
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len;
    int i;

    OpenSSL_add_all_digests();

    if(!argv[1]) {
               printf("Usage: mdtest digestname\n");
                      exit(1);
    }

    md = EVP_get_digestbyname(argv[1]);

    if(!md) {
               printf("Unknown message digest %s\n", argv[1]);
                      exit(1);
    }

    EVP_MD_CTX_init(&mdctx);
    EVP_DigestInit_ex(&mdctx, md, NULL);
    //EVP_DigestUpdate(&mdctx, mess1, strlen(mess1));
    //EVP_DigestUpdate(&mdctx, mess1, strlen(mess1));
    EVP_DigestUpdate(&mdctx, mess3, strlen(mess3));
    EVP_DigestFinal_ex(&mdctx, md_value, &md_len);
    EVP_MD_CTX_cleanup(&mdctx);

    printf("Digest Length is: %d\n", md_len);
    printf("Digest is: ");
    for(i = 0; i < md_len; i++) printf("%02x", md_value[i]);
    printf("\n");

    return 0;
}
