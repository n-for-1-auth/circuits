# AES circuits

This directory contains a full AES circuit (same as the one commonly available) and a version of AES where the key schedule is separate.

| Filename    | Description | Number of AND gates |
| ----------- | ----------- | ----------- |
| aes128_full.txt  | Full AES circuit | 6400 |
| aes128_key_schedule.txt  | Part of AES circuit: Key scheduling | 1280 |
| aes128.txt  | Part of AES circuit: Post key-scheduling       | 5120 |

## Syntax

### aes128_full.txt

- **Alice's Input:** 256 bits. 
  * First 128 bits belong to the key. 
  * Second 128 bits belong to the plaintext.
  * Remark: this differs from the well-known `AES-non-expanded.txt` circuit.
- **Output:** 128 bits. 

### aes128_key_schedule.txt

- **Alice's Input:** 128 bits, the key.
- **Output:** 1408 bits, 11 round keys for 10 rounds-AES.

### aes128.txt

- **Alice's Input:** 1408 bits, the round keys.
- **Bob's Input:** 128 bits, the plaintext.
- **Output:** 128 bits.

## Synthesis 

The subdirectory [synthesis](./synthesis) provides more information on how these circuits are made. 

## Warning: Endianness and byte-ordering

When integrating the circuits with some other protocols (e.g., TLS-in-SMPC), we often need to rematch the endianness and byte-ordering. We did not have a magical way to do this (but seems to be highly possible). 

We recommend, nevertheless, to match the endianness and byte-ordering by trying different inputs, from all-zero to completely random. This method has helped us match the representation with wolfssl. 