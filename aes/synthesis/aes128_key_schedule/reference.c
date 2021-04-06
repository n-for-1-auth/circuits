#include <stdio.h>
#include <mbedtls/aes.h>

int main(){
	mbedtls_aes_context ctx;

	mbedtls_aes_init(&ctx);

	// set the key
	unsigned char key[16];
	for(int i = 0; i < 16; i++) {
		key[i] = 0;
	}
	mbedtls_aes_setkey_enc(&ctx, key, 128);

	// print out the key schedule
	printf("key schedule:\n");
	for(int i = 0; i < 11; i++){
		for(int j = 0; j < 4; j++){
			printf("%.8x ", ctx.rk[i * 4 + j]);
		}
		printf("\n");
	}

	// encrypt the test plaintext
	unsigned char plaintext[16];
	plaintext[0] = 243;
	plaintext[1] = 68;
	plaintext[2] = 129;
	plaintext[3] = 236;
	plaintext[4] = 60;
	plaintext[5] = 198;
	plaintext[6] = 39;
	plaintext[7] = 186;
	plaintext[8] = 205;
	plaintext[9] = 93;
	plaintext[10] = 195;
	plaintext[11] = 251;
	plaintext[12] = 8;
	plaintext[13] = 242;
	plaintext[14] = 115;
	plaintext[15] = 230;
	
	unsigned char ciphertext[16];
	mbedtls_aes_crypt_ecb(&ctx, MBEDTLS_AES_ENCRYPT, plaintext, ciphertext);

	// print out the key schedule
	printf("ciphertext:\n");
	for(int i = 0; i < 4; i++){
		for(int j = 0; j < 4; j++) {
			printf("%.2x", ciphertext[i * 4 + j]);
		}
		printf(" ");
	}
	printf("\n");

	mbedtls_aes_free(&ctx);

	return 0;
}
