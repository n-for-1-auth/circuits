#include "emp-tool/emp-tool.h"
#include <cstring>

using namespace emp;

typedef unsigned int word32;

void reverse_bytes(block *data, int num_bits) {
	auto data2 = (block*) malloc(sizeof(block) * num_bits);
	memcpy(data2, data, sizeof(block) * num_bits);

	for(int i = 0; i < num_bits; i++) {
		data[i] = data2[num_bits - 1 - i];
	}

	free(data2);
}

void change_endian(block *input, block *output, int input_len) {
	if (input_len % 8 != 0) {
		error("The circuit synthesizer can only convert the endianness for bytes.");
	}

	int num_bytes = input_len / 8;
	for (int i = 0; i < num_bytes; i++) {
		for (int j = 0; j < 8; j++) {
			output[i * 8 + j] = input[i * 8 + (7 - j)];
		}
	}
}

void print_hash(block *output) {
	unsigned char digest_char[32];
	memset(digest_char, 0, 32);

	bool output_bool[256];
	ProtocolExecution::prot_exec->reveal(output_bool, PUBLIC, (block *) output, 256);

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 4; j++) {
			int w = 1;
			for (int k = 0; k < 8; k++) {
				digest_char[i * 4 + j] += output_bool[i * 32 + 8 * j + k] * w;
				w <<= 1;
			}
		}
	}

	for (int i = 0; i < 32; i++) {
		printf("%02X ", digest_char[i]);
	}
	printf("\n");
}

void print_many_bytes(block *output, int num_bytes) {
	unsigned char digest_char[num_bytes];
	memset(digest_char, 0, num_bytes);

	bool output_bool[num_bytes * 8];
	ProtocolExecution::prot_exec->reveal(output_bool, PUBLIC, (block *) output, num_bytes * 8);

	for (int i = 0; i < num_bytes; i++) {
		int w = 1;
		for (int j = 0; j < 8; j++) {
			digest_char[i] += output_bool[i * 8 + j] * w;
			w <<= 1;
		}
	}

	for (int i = 0; i < num_bytes; i++) {
		printf("%02X ", digest_char[i]);
	}
	printf("\n");
}

int get_padded_len(int L) {
	// find K such that L + 1 + K + 64 is a multiple of 512
	int K = 512 - ((L + 1 + 64) % 512);
	K %= 512;    // If L + 1 + 64 is already a multiple of 512, K = 0

	return L + 1 + K + 64;
}

void padding(block *input, block *output, int input_len) {
	block one = CircuitExecution::circ_exec->public_label(true);
	block zero = CircuitExecution::circ_exec->public_label(false);

	for (int i = 0; i < input_len; i++) {
		output[i] = input[i];
	}

	int offset = input_len;

	// add one bit "1"
	output[offset++] = one;

	// find K such that L + 1 + K + 64 is a multiple of 512
	int K = 512 - ((input_len + 1 + 64) % 512);
	K %= 512;    // If L + 1 + 64 is already a multiple of 512, K = 0

	// add K bits "0"
	for (int i = 0; i < K; i++) {
		output[offset++] = zero;
	}

	if (input_len > 8191) {
		error("The circuit synthesizer assumes that input_len is small (< 8192 bits).");
	}

	// add the length of L
	// for simplicity, assume that the higher 48 bits are zero---since our input is going to be small anyway
	// the remaining 16 bits give you 2^15-1 bits to spend, about 8KB
	for (int i = 0; i < 48; i++) {
		output[offset++] = zero;
	}

	for (int i = 0; i < 16; i++) {
		int bool_test = (input_len & (1 << (16 - 1 - i))) != 0;
		output[offset++] = bool_test ? one : zero;
	}
}

void sha256(block *input, block *output, int input_len) {
	// new input
	auto input_new = new block[input_len];

	// reverse the bits
	change_endian(input, input_new, input_len);

	// first, do the padding
	int padded_len = get_padded_len(input_len);

	// allocate the padding
	block *padded_input = new block[padded_len];

	// pad
	padding(input_new, padded_input, input_len);

	delete[] input_new;

	// number of blocks
	int num_blocks = padded_len / 512;

	// start the hashing
	// first block
	word32 digest[8];
	digest[0] = 0x6A09E667L;
	digest[1] = 0xBB67AE85L;
	digest[2] = 0x3C6EF372L;
	digest[3] = 0xA54FF53AL;
	digest[4] = 0x510E527FL;
	digest[5] = 0x9B05688CL;
	digest[6] = 0x1F83D9ABL;
	digest[7] = 0x5BE0CD19L;

	block one = CircuitExecution::circ_exec->public_label(true);
	block zero = CircuitExecution::circ_exec->public_label(false);

	auto input_to_sha256_circuit = new block[768];
	block output_from_sha256_circuit[256];

	block digest_bits[256];
	for (int i = 0; i < 8; i++) {
		word32 tmp = digest[i];
		for (int j = 0; j < 32; j++) {
			digest_bits[i * 32 + j] = (tmp & 1) != 0 ? one : zero;
			tmp >>= 1;
		}
	}

	for (int b = 0; b < num_blocks; b++) {
		printf("-- sha256\n");
		// the first 512 bits -> the padded data
		// the rest of the 256 bits -> the 8 * 32 bits of the digest values

		for (int i = 0; i < 512; i++) {
			input_to_sha256_circuit[i] = padded_input[b * 512 + i];
		}

		for (int i = 0; i < 256; i++) {
			input_to_sha256_circuit[512 + i] = digest_bits[i];
		}

		BristolFormat bf("./emp-tool/circuits/files/bristol_format/sha-256-multiblock-aligned.txt");
		bf.compute(output_from_sha256_circuit, input_to_sha256_circuit, input_to_sha256_circuit);

		for (int i = 0; i < 256; i++) {
			digest_bits[i] = output_from_sha256_circuit[i];
		}
	}

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < 8; k++) {
				output[i * 32 + j * 8 + k] = output_from_sha256_circuit[i * 32 + 8 * (3 - j) + k];
			}
		}
	}

	delete[] padded_input;
	delete[] input_to_sha256_circuit;
}

void sha256_test() {
	printf("SHA256 test:\n");
	block output[256];
	block input[2048];

	block one = CircuitExecution::circ_exec->public_label(true);
	block zero = CircuitExecution::circ_exec->public_label(false);

	for (int i = 0; i < 2048; i++) {
		input[i] = zero;
	}

	// empty sha256
	sha256(input, output, 0);
	print_hash(output);

	// hash of 256 bits "1"
	for (int i = 0; i < 256; i++) {
		input[i] = one;
	}
	sha256(input, output, 256);
	print_hash(output);

	// hash of 512 bits "1"
	// needs another block
	for (int i = 0; i < 512; i++) {
		input[i] = one;
	}
	sha256(input, output, 512);
	print_hash(output);

	// hash of 1024 bits "1"
	// needs three blocks
	for (int i = 0; i < 1024; i++) {
		input[i] = one;
	}
	sha256(input, output, 1024);
	print_hash(output);
}

void hmac(block *key, int key_len, block *data, int data_len, block *output) {
	// reject key that is too long
	if (key_len > 512) {
		error("The circuit synthesizer only supports key that is shorter or equal to 512 bits.");
	}

	// create the ipad
	unsigned char ipad_bytes[512 / 8];
	for (int i = 0; i < 64; i++) {
		ipad_bytes[i] = 0x36;
	}

	block one = CircuitExecution::circ_exec->public_label(true);
	block zero = CircuitExecution::circ_exec->public_label(false);

	// convert ipad into bits
	block ipad[512];
	for (int i = 0; i < 64; i++) {
		unsigned char tmp = ipad_bytes[i];
		for (int j = 0; j < 8; j++) {
			ipad[i * 8 + j] = (tmp & 1) != 0 ? one : zero;
			tmp >>= 1;
		}
	}

	// assemble the hash function input
	block input_to_hash_function[512 + data_len];
	for (int i = 0; i < 512; i++) {
		input_to_hash_function[i] = ipad[i];
	}
	for (int i = 0; i < key_len; i++) {
		input_to_hash_function[i] = CircuitExecution::circ_exec->xor_gate(input_to_hash_function[i], key[i]);
	}
	for (int i = 0; i < data_len; i++) {
		input_to_hash_function[512 + i] = data[i];
	}

	// allocate the hash function output
	block output_from_hash_function[256];

	// compute the inner hash
	sha256(input_to_hash_function, output_from_hash_function, 512 + data_len);

	// create the opad
	unsigned char opad_bytes[512 / 8];
	for (int i = 0; i < 64; i++) {
		opad_bytes[i] = 0x5c;
	}

	// convert opad into bits
	block opad[512];
	for (int i = 0; i < 64; i++) {
		unsigned char tmp = opad_bytes[i];
		for (int j = 0; j < 8; j++) {
			opad[i * 8 + j] = (tmp & 1) != 0 ? one : zero;
			tmp >>= 1;
		}
	}

	block input_2_to_hash_function[512 + 256];
	for (int i = 0; i < 512; i++) {
		input_2_to_hash_function[i] = opad[i];
	}
	for (int i = 0; i < key_len; i++) {
		input_2_to_hash_function[i] = CircuitExecution::circ_exec->xor_gate(input_2_to_hash_function[i], key[i]);
	}
	for (int i = 0; i < 256; i++) {
		input_2_to_hash_function[512 + i] = output_from_hash_function[i];
	}

	// allocate the hash function output
	block output_2_from_hash_function[256];

	// compute the outer hash
	sha256(input_2_to_hash_function, output_2_from_hash_function, 512 + 256);

	for (int i = 0; i < 256; i++) {
		output[i] = output_2_from_hash_function[i];
	}
}

void hmac_test() {
	printf("HMAC test:\n");
	block output[256];
	block key[256];
	block input[2048];

	block one = CircuitExecution::circ_exec->public_label(true);
	block zero = CircuitExecution::circ_exec->public_label(false);

	for (int i = 0; i < 2048; i++) {
		input[i] = zero;
	}

	for (int i = 0; i < 256; i++) {
		key[i] = one;
	}

	// empty sha256
	hmac(key, 256, input, 0, output);
	print_hash(output);

	// hash of 256 bits "1"
	for (int i = 0; i < 256; i++) {
		input[i] = one;
	}
	hmac(key, 256, input, 256, output);
	print_hash(output);

	// hash of 512 bits "1"
	// needs another block
	for (int i = 0; i < 512; i++) {
		input[i] = one;
	}
	hmac(key, 256, input, 512, output);
	print_hash(output);

	// hash of 1024 bits "1"
	// needs three blocks
	for (int i = 0; i < 1024; i++) {
		input[i] = one;
	}
	hmac(key, 256, input, 1024, output);
	print_hash(output);
}

void hkdf_extract(block *salt, int salt_len, block *ikm, int ikm_len, block *output) {
	if (salt_len == 0) {
		block key[256];

		block zero = CircuitExecution::circ_exec->public_label(false);
		for (int i = 0; i < 256; i++) {
			key[i] = zero;
		}

		hmac(key, 256, ikm, ikm_len, output);
	} else {
		hmac(salt, salt_len, ikm, ikm_len, output);
	}
}

void hkdf_extract_test() {
	block output[256];

	block zero = CircuitExecution::circ_exec->public_label(false);
	block key[256];
	for (int i = 0; i < 256; i++) {
		key[i] = zero;
	}

	block salt[8];
	for (int i = 0; i < 8; i++) {
		salt[i] = zero;
	}

	hkdf_extract(salt, 8, key, 256, output);
	print_hash(output);
}

void hkdf_expand(block *key, int key_len, block *info, int info_len, block *output, int output_byte_len) {
	if (key_len < 256) {
		error("Key length for HKDF expand must be at least 256 bits.\n");
	}

	int N = (output_byte_len + 32 - 1) / 32;

	block cur_T[256];
	int cur_T_len = 0;

	for (int i = 1; i <= N; i++) {
		auto input = new block[cur_T_len + info_len + 8];
		for (int j = 0; j < cur_T_len; j++) {
			input[j] = cur_T[j];
		}
		for (int j = 0; j < info_len; j++) {
			input[cur_T_len + j] = info[j];
		}

		bool ctr[8];
		int w = i;
		for (int j = 0; j < 8; j++) {
			ctr[j] = w & 1;
			w >>= 1;
		}

		block one = CircuitExecution::circ_exec->public_label(true);
		block zero = CircuitExecution::circ_exec->public_label(false);

		for (int j = 0; j < 8; j++) {
			input[cur_T_len + info_len + j] = ctr[j] == 1 ? one : zero;
		}

		hmac(key, key_len, input, cur_T_len + info_len + 8, cur_T);
		cur_T_len = 256;
		for (int j = 0; j < 256; j++) {
			if (((i - 1) * 256 + j) < output_byte_len * 8) {
				output[(i - 1) * 256 + j] = cur_T[j];
			}
		}
	}
}

void hkdf_expand_label(block *key, int key_len, const char *label, block *context, int context_len, block *output,
					   int output_byte_len) {
	char long_label[255];
	sprintf(long_label, "tls13 %s", label);

	int long_label_len = strlen(long_label);

	block hkdf_label[16 + 8 + long_label_len * 8 + 8 + context_len];

	block one = CircuitExecution::circ_exec->public_label(true);
	block zero = CircuitExecution::circ_exec->public_label(false);

	int offset = 0;
	int w;

	w = output_byte_len;
	for (int i = 0; i < 8; i++) {
		hkdf_label[8 + i] = w & 1 ? one : zero;
		w >>= 1;
	}
	for (int i = 0; i < 8; i++) {
		hkdf_label[i] = w & 1 ? one : zero;
		w >>= 1;
	}

	offset += 16;

	w = long_label_len;
	for (int i = 0; i < 8; i++) {
		hkdf_label[offset++] = w & 1 ? one : zero;
		w >>= 1;
	}

	for (int i = 0; i < long_label_len; i++) {
		w = (unsigned char) long_label[i];
		for (int j = 0; j < 8; j++) {
			hkdf_label[offset++] = w & 1 ? one : zero;
			w >>= 1;
		}
	}

	w = context_len / 8;    // length in bytes
	for (int i = 0; i < 8; i++) {
		hkdf_label[offset++] = w & 1 ? one : zero;
		w >>= 1;
	}

	for (int i = 0; i < context_len; i++) {
		hkdf_label[offset++] = context[i];
	}

	hkdf_expand(key, key_len, hkdf_label, 16 + 8 + long_label_len * 8 + 8 + context_len, output, output_byte_len);
}

void hkdf_expand_label_test() {
	// first, compute the early secret
	block early_secret[256];

	block zero = CircuitExecution::circ_exec->public_label(false);
	block key[256];
	for (int i = 0; i < 256; i++) {
		key[i] = zero;
	}

	block salt[8];
	for (int i = 0; i < 8; i++) {
		salt[i] = zero;
	}

	hkdf_extract(salt, 8, key, 256, early_secret);
	print_hash(early_secret);

	// second, compute the empty hash
	block empty_hash[256];
	sha256(nullptr, empty_hash, 0);
	print_hash(empty_hash);

	// third, compute derived
	block derived_secret[256];
	hkdf_expand_label(early_secret, 256, "derived", empty_hash, 256, derived_secret, 32);
	print_hash(derived_secret);
}

void DeriveHandshakeSecret_PreMasterSecret() {
	setup_plain_prot(true, "DeriveHandshakeSecret_PreMasterSecret.txt");
	unsigned char derived_secert[] =
			{
					0x6f, 0x26, 0x15, 0xa1, 0x08, 0xc7, 0x02, 0xc5,
					0x67, 0x8f, 0x54, 0xfc, 0x9d, 0xba, 0xb6, 0x97,
					0x16, 0xc0, 0x76, 0x18, 0x9c, 0x48, 0x25, 0x0c,
					0xeb, 0xea, 0xc3, 0x57, 0x6c, 0x36, 0x11, 0xba
			};

	unsigned char test_shared_secret[] =
			{
					0xdf, 0x4a, 0x29, 0x1b, 0xaa, 0x1e, 0xb7, 0xcf,
					0xa6, 0x93, 0x4b, 0x29, 0xb4, 0x74, 0xba, 0xad,
					0x26, 0x97, 0xe2, 0x9f, 0x1f, 0x92, 0x0d, 0xcc,
					0x77, 0xc8, 0xa0, 0xa0, 0x88, 0x44, 0x76, 0x24
			};

	block one = CircuitExecution::circ_exec->public_label(true);
	block zero = CircuitExecution::circ_exec->public_label(false);

	block salt[256];
	for (int i = 0; i < 32; i++) {
		int w = derived_secert[i];
		for (int j = 0; j < 8; j++) {
			salt[i * 8 + j] = w & 1 ? one : zero;
			w >>= 1;
		}
	}

	bool ikm_plaintext[256];
	for (int i = 0; i < 32; i++) {
		int w = test_shared_secret[i];
		for (int j = 0; j < 8; j++) {
			ikm_plaintext[i * 8 + j] = (w & 1) != 0;
			w >>= 1;
		}
	}

	block ikm[256];
	ProtocolExecution::prot_exec->feed(ikm, ALICE, ikm_plaintext, 256);

	block output[256];
	hkdf_extract(salt, 256, ikm, 256, output);
	print_hash(output);

	// expected output: fb9fc80689b3a5d02c33243bf69a1b1b20705588a794304a6e7120155edf149a

	finalize_plain_prot();
}

// Application
void DeriveMasterSecret() {
	setup_plain_prot(true, "DeriveMasterSecret.txt");

	unsigned char empty_hash[] =
			{
					0xe3, 0xb0, 0xc4, 0x42, 0x98, 0xfc, 0x1c, 0x14,
					0x9a, 0xfb, 0xf4, 0xc8, 0x99, 0x6f, 0xb9, 0x24,
					0x27, 0xae, 0x41, 0xe4, 0x64, 0x9b, 0x93, 0x4c,
					0xa4, 0x95, 0x99, 0x1b, 0x78, 0x52, 0xb8, 0x55
			};

	unsigned char test_handshake_secret[] =
			{
					0xfb, 0x9f, 0xc8, 0x06, 0x89, 0xb3, 0xa5, 0xd0,
					0x2c, 0x33, 0x24, 0x3b, 0xf6, 0x9a, 0x1b, 0x1b,
					0x20, 0x70, 0x55, 0x88, 0xa7, 0x94, 0x30, 0x4a,
					0x6e, 0x71, 0x20, 0x15, 0x5e, 0xdf, 0x14, 0x9a
			};

	block one = CircuitExecution::circ_exec->public_label(true);
	block zero = CircuitExecution::circ_exec->public_label(false);

	bool key_plaintext[256];
	for (int i = 0; i < 32; i++) {
		int w = test_handshake_secret[i];
		for (int j = 0; j < 8; j++) {
			key_plaintext[i * 8 + j] = w & 1;
			w >>= 1;
		}
	}

	block context[256];
	for (int i = 0; i < 32; i++) {
		int w = empty_hash[i];
		for (int j = 0; j < 8; j++) {
			context[i * 8 + j] = w & 1 ? one : zero;
			w >>= 1;
		}
	}

	block key[256];
	ProtocolExecution::prot_exec->feed(key, ALICE, key_plaintext, 256);

	block derived_secret[256];
	hkdf_expand_label(key, 256, "derived", context, 256, derived_secret, 32);
	// print_hash(derived_secret);
	// expected output: de9f5c98db4261a46911f1349c1ba2c84dd84482249f8f2cb3a989e4e4a804e6

	block zero_key[256];
	for (int i = 0; i < 256; i++) {
		zero_key[i] = zero;
	}

	block master_secret[256];
	hkdf_extract(derived_secret, 256, zero_key, 256, master_secret);
	print_hash(master_secret);
	// expected output: 7f2882bb9b9a46265941653e9c2f19067118151e21d12e57a7b6aca1f8150c8d

	finalize_plain_prot();
}

void hkdf_wolfssl_test() {
	setup_plain_prot(false, "test.txt");

	unsigned char ikm1[] =
			{
					0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
					0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
					0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b
			};
	unsigned char salt1[] =
			{
					0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
					0x08, 0x09, 0x0a, 0x0b, 0x0c
			};
	unsigned char info1[] =
			{
					0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
					0xf8, 0xf9
			};
	// Add a 0x00 at the end since our API only works with 0x00-ended string.

	block one = CircuitExecution::circ_exec->public_label(true);
	block zero = CircuitExecution::circ_exec->public_label(false);

	block ikm1_plaintext[22 * 8];
	for (int i = 0; i < 22; i++) {
		int w = ikm1[i];
		for (int j = 0; j < 8; j++) {
			ikm1_plaintext[i * 8 + j] = w & 1? one : zero;
			w >>= 1;
		}
	}

	block salt1_plaintext[13 * 8];
	for (int i = 0; i < 13; i++) {
		int w = salt1[i];
		for (int j = 0; j < 8; j++) {
			salt1_plaintext[i * 8 + j] = w & 1? one : zero;
			w >>= 1;
		}
	}

	block info1_plaintext[10 * 8];
	for (int i = 0; i < 10; i++) {
		int w = info1[i];
		for (int j = 0; j < 8; j++) {
			info1_plaintext[i * 8 + j] = w & 1? one : zero;
			w >>= 1;
		}
	}


	block key[256];
	hkdf_extract(salt1_plaintext, 13 * 8, ikm1_plaintext, 22 * 8, key);

	block res4_result[42 * 8];
	hkdf_expand(key, 256, info1_plaintext, 10 * 8, res4_result, 42);
	print_many_bytes(res4_result, 42);

	/*
	 * 				Supposed output:
	 * 				0x3c, 0xb2, 0x5f, 0x25, 0xfa, 0xac, 0xd5, 0x7a,
	 *				0x90, 0x43, 0x4f, 0x64, 0xd0, 0x36, 0x2f, 0x2a,
	 *				0x2d, 0x2d, 0x0a, 0x90, 0xcf, 0x1a, 0x5a, 0x4c,
	 *				0x5d, 0xb0, 0x2d, 0x56, 0xec, 0xc4, 0xc5, 0xbf,
	 *				0x34, 0x00, 0x72, 0x08, 0xd5, 0xb8, 0x87, 0x18,
	 *				0x58, 0x65
	 */

	finalize_plain_prot();
}

void DeriveClientHandshakeSecret() {
	setup_plain_prot(true, "DeriveClientHandshakeSecret.txt");

	unsigned char handshake_secret_test_data[]=
			{
					0xfb, 0x9f, 0xc8, 0x06, 0x89, 0xb3, 0xa5, 0xd0,
					0x2c, 0x33, 0x24, 0x3b, 0xf6, 0x9a, 0x1b, 0x1b,
					0x20, 0x70, 0x55, 0x88, 0xa7, 0x94, 0x30, 0x4a,
					0x6e, 0x71, 0x20, 0x15, 0x5e, 0xdf, 0x14, 0x9a
			};

	unsigned char hello_hash_test_data[] =
			{
					0xda, 0x75, 0xce, 0x11, 0x39, 0xac, 0x80, 0xda,
					0xe4, 0x04, 0x4d, 0xa9, 0x32, 0x35, 0x0c, 0xf6,
					0x5c, 0x97, 0xcc, 0xc9, 0xe3, 0x3f, 0x1e, 0x6f,
					0x7d, 0x2d, 0x4b, 0x18, 0xb7, 0x36, 0xff, 0xd5
			};

	bool handshake_secret_plaintext[32 * 8];
	for (int i = 0; i < 32; i++) {
		int w = handshake_secret_test_data[i];
		for (int j = 0; j < 8; j++) {
			handshake_secret_plaintext[i * 8 + j] = w & 1;
			w >>= 1;
		}
	}

	bool hello_hash_plaintext[256];
	for (int i = 0; i < 32; i++) {
		int w = hello_hash_test_data[i];
		for (int j = 0; j < 8; j++) {
			hello_hash_plaintext[i * 8 + j] = w & 1;
			w >>= 1;
		}
	}

	block handshake_secret[256];
	ProtocolExecution::prot_exec->feed(handshake_secret, ALICE, handshake_secret_plaintext, 256);

	block hello_hash[256];
	ProtocolExecution::prot_exec->feed(hello_hash, ALICE, hello_hash_plaintext, 256);

	// client_handshake_traffic_secret = HKDF-Expand-Label(
	//    key = handshake_secret,
	//    label = "c hs traffic",
	//    context = hello_hash,
	//    len = 32)

	block client_handshake_traffic_secret[256];
	hkdf_expand_label(handshake_secret, 256, "c hs traffic", hello_hash, 256, client_handshake_traffic_secret, 32);
	print_many_bytes(client_handshake_traffic_secret, 32);

	finalize_plain_prot();
}

void DeriveServerHandshakeSecret() {
	setup_plain_prot(true, "DeriveServerHandshakeSecret.txt");

	unsigned char handshake_secret_test_data[]=
			{
					0xfb, 0x9f, 0xc8, 0x06, 0x89, 0xb3, 0xa5, 0xd0,
					0x2c, 0x33, 0x24, 0x3b, 0xf6, 0x9a, 0x1b, 0x1b,
					0x20, 0x70, 0x55, 0x88, 0xa7, 0x94, 0x30, 0x4a,
					0x6e, 0x71, 0x20, 0x15, 0x5e, 0xdf, 0x14, 0x9a
			};

	unsigned char hello_hash_test_data[] =
			{
					0xda, 0x75, 0xce, 0x11, 0x39, 0xac, 0x80, 0xda,
					0xe4, 0x04, 0x4d, 0xa9, 0x32, 0x35, 0x0c, 0xf6,
					0x5c, 0x97, 0xcc, 0xc9, 0xe3, 0x3f, 0x1e, 0x6f,
					0x7d, 0x2d, 0x4b, 0x18, 0xb7, 0x36, 0xff, 0xd5
			};

	bool handshake_secret_plaintext[32 * 8];
	for (int i = 0; i < 32; i++) {
		int w = handshake_secret_test_data[i];
		for (int j = 0; j < 8; j++) {
			handshake_secret_plaintext[i * 8 + j] = w & 1;
			w >>= 1;
		}
	}

	bool hello_hash_plaintext[256];
	for (int i = 0; i < 32; i++) {
		int w = hello_hash_test_data[i];
		for (int j = 0; j < 8; j++) {
			hello_hash_plaintext[i * 8 + j] = w & 1;
			w >>= 1;
		}
	}

	block handshake_secret[256];
	ProtocolExecution::prot_exec->feed(handshake_secret, ALICE, handshake_secret_plaintext, 256);

	block hello_hash[256];
	ProtocolExecution::prot_exec->feed(hello_hash, ALICE, hello_hash_plaintext, 256);

	// server_handshake_traffic_secret = HKDF-Expand-Label(
	//    key = handshake_secret,
	//    label = "s hs traffic",
	//    context = hello_hash,
	//    len = 32)

	block server_handshake_traffic_secret[256];
	hkdf_expand_label(handshake_secret, 256, "s hs traffic", hello_hash, 256, server_handshake_traffic_secret, 32);
	print_many_bytes(server_handshake_traffic_secret, 32);

	finalize_plain_prot();
}

void DeriveClientHandshakeKey() {
	setup_plain_prot(true, "DeriveClientHandshakeKey.txt");

	unsigned char client_handshake_traffic_secret_test_data[]=
			{
					0xff, 0x0e, 0x5b, 0x96, 0x52, 0x91, 0xc6, 0x08,
					0xc1, 0xe8, 0xcd, 0x26, 0x7e, 0xef, 0xc0, 0xaf,
					0xcc, 0x5e, 0x98, 0xa2, 0x78, 0x63, 0x73, 0xf0,
					0xdb, 0x47, 0xb0, 0x47, 0x86, 0xd7, 0x2a, 0xea
			};

	bool client_handshake_traffic_secret_plaintext[32 * 8];
	for (int i = 0; i < 32; i++) {
		int w = client_handshake_traffic_secret_test_data[i];
		for (int j = 0; j < 8; j++) {
			client_handshake_traffic_secret_plaintext[i * 8 + j] = w & 1;
			w >>= 1;
		}
	}

	block client_handshake_traffic_secret[256];
	ProtocolExecution::prot_exec->feed(client_handshake_traffic_secret, ALICE, client_handshake_traffic_secret_plaintext, 256);

	// client_handshake_key = HKDF-Expand-Label(
	//    key = client_handshake_traffic_secret,
	//    label = "key",
	//    context = "",
	//    len = 16)

	block client_handshake_key[128];
	hkdf_expand_label(client_handshake_traffic_secret, 256, "key", NULL, 0, client_handshake_key, 16);
	print_many_bytes(client_handshake_key, 16);

	finalize_plain_prot();
}

void DeriveClientHandshakeIV() {
	setup_plain_prot(true, "DeriveClientHandshakeIV.txt");

	unsigned char client_handshake_traffic_secret_test_data[]=
			{
					0xff, 0x0e, 0x5b, 0x96, 0x52, 0x91, 0xc6, 0x08,
					0xc1, 0xe8, 0xcd, 0x26, 0x7e, 0xef, 0xc0, 0xaf,
					0xcc, 0x5e, 0x98, 0xa2, 0x78, 0x63, 0x73, 0xf0,
					0xdb, 0x47, 0xb0, 0x47, 0x86, 0xd7, 0x2a, 0xea
			};

	bool client_handshake_traffic_secret_plaintext[32 * 8];
	for (int i = 0; i < 32; i++) {
		int w = client_handshake_traffic_secret_test_data[i];
		for (int j = 0; j < 8; j++) {
			client_handshake_traffic_secret_plaintext[i * 8 + j] = w & 1;
			w >>= 1;
		}
	}

	block client_handshake_traffic_secret[256];
	ProtocolExecution::prot_exec->feed(client_handshake_traffic_secret, ALICE, client_handshake_traffic_secret_plaintext, 256);

	// client_handshake_iv = HKDF-Expand-Label(
	//    key = client_handshake_traffic_secret,
	//    label = "iv",
	//    context = "",
	//    len = 16)

	block client_handshake_iv[96];
	hkdf_expand_label(client_handshake_traffic_secret, 256, "iv", NULL, 0, client_handshake_iv, 12);
	print_many_bytes(client_handshake_iv, 12);

	finalize_plain_prot();
}

void DeriveClientTrafficSecret() {
	setup_plain_prot(true, "DeriveClientTrafficSecret.txt");

	unsigned char master_secret_test_data[]=
			{
					0x7f, 0x28, 0x82, 0xbb, 0x9b, 0x9a, 0x46, 0x26,
					0x59, 0x41, 0x65, 0x3e, 0x9c, 0x2f, 0x19, 0x06,
					0x71, 0x18, 0x15, 0x1e, 0x21, 0xd1, 0x2e, 0x57,
					0xa7, 0xb6, 0xac, 0xa1, 0xf8, 0x15, 0x0c, 0x8d
			};

	unsigned char handshake_hash_test_data[] =
			{
					0x22, 0x84, 0x4b, 0x93, 0x0e, 0x5e, 0x0a, 0x59,
					0xa0, 0x9d, 0x5a, 0xc3, 0x5f, 0xc0, 0x32, 0xfc,
					0x91, 0x16, 0x3b, 0x19, 0x38, 0x74, 0xa2, 0x65,
					0x23, 0x6e, 0x56, 0x80, 0x77, 0x37, 0x8d, 0x8b
			};

	bool master_secret_plaintext[32 * 8];
	for (int i = 0; i < 32; i++) {
		int w = master_secret_test_data[i];
		for (int j = 0; j < 8; j++) {
			master_secret_plaintext[i * 8 + j] = w & 1;
			w >>= 1;
		}
	}

	bool handshake_hash_plaintext[256];
	for (int i = 0; i < 32; i++) {
		int w = handshake_hash_test_data[i];
		for (int j = 0; j < 8; j++) {
			handshake_hash_plaintext[i * 8 + j] = w & 1;
			w >>= 1;
		}
	}

	block master_secret[256];
	ProtocolExecution::prot_exec->feed(master_secret, ALICE, master_secret_plaintext, 256);

	block handshake_hash[256];
	ProtocolExecution::prot_exec->feed(handshake_hash, ALICE, handshake_hash_plaintext, 256);

	// client_application_traffic_secret = HKDF-Expand-Label(
	//    key = master_secret,
	//    label = "c ap traffic",
	//    context = handshake_hash,
	//    len = 32)

	block client_handshake_traffic_secret[256];
	hkdf_expand_label(master_secret, 256, "c ap traffic", handshake_hash, 256, client_handshake_traffic_secret, 32);
	print_many_bytes(client_handshake_traffic_secret, 32);

	finalize_plain_prot();
}

void DeriveClientTrafficKey() {
	setup_plain_prot(true, "DeriveClientTrafficKey.txt");

	unsigned char client_application_traffic_secret_test_data[]=
			{
					0xb8, 0x82, 0x22, 0x31, 0xc1, 0xd6, 0x76, 0xec,
					0xca, 0x1c, 0x11, 0xff, 0xf6, 0x59, 0x42, 0x80,
					0x31, 0x4d, 0x03, 0xa4, 0xe9, 0x1c, 0xf1, 0xaf,
					0x7f, 0xe7, 0x3f, 0x8f, 0x7b, 0xe2, 0xc1, 0x1b
			};

	bool client_application_traffic_secret_plaintext[32 * 8];
	for (int i = 0; i < 32; i++) {
		int w = client_application_traffic_secret_test_data[i];
		for (int j = 0; j < 8; j++) {
			client_application_traffic_secret_plaintext[i * 8 + j] = w & 1;
			w >>= 1;
		}
	}

	block client_application_traffic_secret[256];
	ProtocolExecution::prot_exec->feed(client_application_traffic_secret, ALICE, client_application_traffic_secret_plaintext, 256);

	// client_application_key = HKDF-Expand-Label(
	//    key = client_application_traffic_secret,
	//    label = "key",
	//    context = "",
	//    len = 16)

	block client_application_key[128];
	hkdf_expand_label(client_application_traffic_secret, 256, "key", NULL, 0, client_application_key, 16);
	print_many_bytes(client_application_key, 16);

	finalize_plain_prot();
}

void DeriveClientTrafficIV() {
	setup_plain_prot(true, "DeriveClientTrafficIV.txt");

	unsigned char client_application_traffic_secret_test_data[]=
			{
					0xb8, 0x82, 0x22, 0x31, 0xc1, 0xd6, 0x76, 0xec,
					0xca, 0x1c, 0x11, 0xff, 0xf6, 0x59, 0x42, 0x80,
					0x31, 0x4d, 0x03, 0xa4, 0xe9, 0x1c, 0xf1, 0xaf,
					0x7f, 0xe7, 0x3f, 0x8f, 0x7b, 0xe2, 0xc1, 0x1b
			};

	bool client_application_traffic_secret_plaintext[32 * 8];
	for (int i = 0; i < 32; i++) {
		int w = client_application_traffic_secret_test_data[i];
		for (int j = 0; j < 8; j++) {
			client_application_traffic_secret_plaintext[i * 8 + j] = w & 1;
			w >>= 1;
		}
	}

	block client_application_traffic_secret[256];
	ProtocolExecution::prot_exec->feed(client_application_traffic_secret, ALICE, client_application_traffic_secret_plaintext, 256);

	// client_application_iv = HKDF-Expand-Label(
	//    key = client_application_traffic_secret,
	//    label = "iv",
	//    context = "",
	//    len = 16)

	block client_application_iv[96];
	hkdf_expand_label(client_application_traffic_secret, 256, "iv", NULL, 0, client_application_iv, 12);
	print_many_bytes(client_application_iv, 12);

	finalize_plain_prot();
}

void CreateGCMSequence(){
	for(int BLOCK = 1; BLOCK <= 10; BLOCK ++) {
		setup_plain_prot(true, "gcm_shares_" + std::to_string(BLOCK) + ".txt");

		unsigned char key_test_data[] =
				{
						0x01, 0x6d, 0xbb, 0x38, 0xda, 0xa7, 0x6d, 0xfe,
						0x7d, 0xa3, 0x84, 0xeb, 0xf1, 0x24, 0x03, 0x64
				};

		bool key_plaintext[16 * 8];
		for (int i = 0; i < 16; i++) {
			int w = key_test_data[i];
			for (int j = 0; j < 8; j++) {
				key_plaintext[(15 - i) * 8 + j] = w & 1;
				w >>= 1;
			}
		}

		block key[128];
		ProtocolExecution::prot_exec->feed(key, ALICE, key_plaintext, 128);

		block key_derivation_block_plaintext[128];
		block zero = CircuitExecution::circ_exec->public_label(false);
		for(int i = 0; i < 128; i++) {
			key_derivation_block_plaintext[i] = zero;
		}

		block input[256];
		for(int i = 0; i < 128; i++) {
			input[i] = key[i];
		}
		for(int i = 0; i < 128; i++) {
			input[i + 128] = key_derivation_block_plaintext[i];
		}

		block gcm_key_raw[128];
		block gcm_key[128];

		BristolFormat bf("./emp-tool/circuits/files/bristol_format/aes128_full.txt");
		bf.compute(gcm_key_raw, input, input);

		for(int i = 0; i < 16; i++) {
			for(int j = 0; j < 8; j++) {
				gcm_key[i * 8 + j] = gcm_key_raw[i * 8 + (7 - j)];
			}
		}

		block middle_results[128][128];
		memset(middle_results, 0, sizeof(block) * 128 * 128);

		for(int i = 0; i < 128; i++) {
			middle_results[0][i] = gcm_key_raw[i];
		}

		for(int i = 1; i < 128; i++) {
			middle_results[i][0] = middle_results[i - 1][127];

			for (int j = 1; j < 128; j++) {
				middle_results[i][j] = middle_results[i - 1][j - 1];
			}

			middle_results[i][7] = CircuitExecution::circ_exec->xor_gate(middle_results[i - 1][6], middle_results[i - 1][127]);
			middle_results[i][2] = CircuitExecution::circ_exec->xor_gate(middle_results[i - 1][1], middle_results[i - 1][127]);
			middle_results[i][1] = CircuitExecution::circ_exec->xor_gate(middle_results[i - 1][0], middle_results[i - 1][127]);
		}

		block result[128 * BLOCK];
		for(int i = 0; i < 128 * BLOCK; i++) {
			result[i] = zero;
		}
		for(int i = 0; i < 128; i++) {
			result[i] = gcm_key_raw[i];
		}
		for(int b = 1; b < BLOCK; b++) {
			for (int i = 0; i < 128; i++) {
				for (int j = 0; j < 128; j++) {
					block tmp = CircuitExecution::circ_exec->and_gate(middle_results[i][j], result[(b - 1)* 128 + i]);
					result[b * 128 + j] = CircuitExecution::circ_exec->xor_gate(result[b * 128 + j], tmp);
				}
			}
		}

		block result_change_endianness[128 * BLOCK];
		for(int b = 0; b < BLOCK; b++) {
			change_endian(&result[b * 128], &result_change_endianness[b * 128], 128);
		}
		print_many_bytes(result_change_endianness, 16 * BLOCK);

		finalize_plain_prot();
	}
}

int main(int argc, char **argv) {
	DeriveHandshakeSecret_PreMasterSecret();
	DeriveMasterSecret();
	DeriveClientHandshakeSecret();
	DeriveServerHandshakeSecret();
	DeriveClientHandshakeKey();
	DeriveClientHandshakeIV();
	DeriveClientTrafficSecret();
	DeriveClientTrafficKey();
	DeriveClientTrafficIV();
	CreateGCMSequence();

	return 0;
}

