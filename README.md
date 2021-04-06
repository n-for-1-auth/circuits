# N-for-1 Auth's collection of circuits

When we develop N-for-1 Auth ([ePrint 2021/342](https://eprint.iacr.org/2021/342)), many circuit files that we need were unavailable. 

This repository contains the circuits that we synthesize or assemble using existing works.

## Methods

To synthesize circuits, we leverage the DFF library from [TinyGarble](https://github.com/esonghori/TinyGarble) and scripts from [SCALE-MAMBA](https://github.com/KULeuven-COSIC/SCALE-MAMBA/tree/master/Circuits). The synthesizing is done with the Synopsys tools thanks to UC Berkeley's subscription.

To assemble circuits, we use base circuits from [Nigel Smart](https://homes.esat.kuleuven.be/~nsmart/MPC/) and [Matteo Campanelli](https://github.com/matteocam/sha256-circuit). We want to highlight the latter's SHA256 circuit: [Steven Goldfeder](http://stevengoldfeder.com/) discovered that previous methods to synthesize SHA256 circuits are suboptimal, and special tricks are needed to force the synthesizer to use a simple adder.
The assembly is done using [EMP-toolkit](https://github.com/emp-toolkit/emp-tool), which has a powerful plaintext engine for making circuit files.

## Summary

A collection of Bristol format circuit files related to TLS-in-SMPC.

- **[\[key-derivation\]](./key-derivation):** Circuits for key derivation in TLS
- **[\[aes\]](./aes):** Circuits for AES 
- **[\[gcm\]](./gcm):** Circuits for computing GCM power shares
- **[\[sha256\]](./sha256):** Circuits for multi-block SHA256  
- **[\[generator\]](./generator):** Program that assembles these circuits

## Regulatory issue
This repository is not subject to the U.S. Export Administration Regulation (EAR) because it is publicly available; notifications to U.S. Bureau of Industry and Security (BIS) and National Security Agency (NSA) have been sent.

For more information about this regulatory issue, see [this post](https://www.eff.org/deeplinks/2019/08/us-export-controls-and-published-encryption-source-code-explained) by Electronic Frontier Foundation (EFF).