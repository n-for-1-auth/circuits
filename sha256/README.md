# SHA-256 circuits for multiple blocks

This directory contains a SHA-256 circuit adapted from Nigel Smart's Bristol Fashion SHA-256 function [here](https://homes.esat.kuleuven.be/~nsmart/MPC/). This circuit is being used when we assemble the HMAC function.

## Syntax

- **Alice's Input:** 768 bits
  * First 512 bits, the SHA-256 input; often it needs to be padded.
  * Following 256 bits, the SHA-256 state; note that the initial state is not zero but instead some constants. 
- **Output:** 256 bits.

## Feeding the initial state

The following digests are the common constants for SHA-256, from [wolfssl](https://github.com/wolfSSL/wolfssl).

```c++
word32 digest[8];
digest[0] = 0x6A09E667L;
digest[1] = 0xBB67AE85L;
digest[2] = 0x3C6EF372L;
digest[3] = 0xA54FF53AL;
digest[4] = 0x510E527FL;
digest[5] = 0x9B05688CL;
digest[6] = 0x1F83D9ABL;
digest[7] = 0x5BE0CD19L;
```

One can see the example code in [generate_handshake_circuits.cpp](../generator) for how to input these constants, where the endianness becomes slightly complicated.