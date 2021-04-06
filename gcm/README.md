# GCM circuits

This directory contains circuits that generates the GCM power series _from the AES key_.

| Filename    | Number of AND gates |
| ----------- | ----------- |
| gcm_shares_1.txt | 6400 |
| gcm_shares_2.txt | 22784 |
| gcm_shares_3.txt | 39168 |
| gcm_shares_4.txt | 55552 |
| gcm_shares_5.txt | 71936 |
| gcm_shares_6.txt | 88320 |
| gcm_shares_7.txt | 104704 |
| gcm_shares_8.txt | 121088 |
| gcm_shares_9.txt | 137472 |
| gcm_shares_10.txt | 153856 |

## Syntax

- **Alice's Input:** 128 bits, the AES key. 
- **Output:** 128 * N bits, the sequence `g`, `g^2`, `g^3`, ... 


## Synthesis 

The subdirectory [synthesis](./synthesis) provides information that generates the GCM multiplication circuit. The final circuits are assembled by the C++ script [here](../generator/).

## Applications

In TLS-in-SMPC, one important step is to compute the GCM tag for ciphertexts. Instead of doing such computation each time inside the garbled circuit, the better approach is to move such computation out, by generating GCM series and secret-sharing them among different parties. This technique comes from [DECO](https://www.deco.works/).

The secret-sharing can be done using the flexible input-output AG-MPC [here](https://github.com/n-for-1-auth/emp-agmpc-flex-in-out).

Choosing the length of the GCM power series, `N`, is important. If you choose a very small `N`, the last step of GCM tag computation, which adds the AES-ed result of the IV, happens so often that it dominates the cost. If  `N` is too large, then generating the GCM series is already expensive. One would need to balance this by doing a linear optimization.

