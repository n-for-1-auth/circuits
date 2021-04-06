# Key derivation circuits

This directory contains circuits that derives the many keys needed in TLS. These are invocations of the HKDF functions. 

We now list the circuits in the order that they are used in TLS-in-SMPC, also depicted in Michael Driscoll's illustration of TLS 1.3 [here](https://tls13.ulfheim.net/).

| Filename    | Number of AND gates | Note |
| ----------- | ----------- | ----------- |
| DeriveHandshakeSecret_PreMasterSecret.txt | 42897 | 
| DeriveClientHandshakeSecret.txt | 87893 |
| DeriveServerHandshakeSecret.txt | 87894 |
| DeriveClientHandshakeKey.txt | 83320 | can also be used for servers |
| DeriveClientHandshakeIV.txt | 83328 | can also be used for servers |
| DeriveMasterSecret.txt | 166654 |
| DeriveClientTrafficSecret.txt | 87900 | 
| DeriveClientTrafficKey.txt | 83320 | can also be used for servers |
| DeriveClientTrafficIV.txt | 83328| can also be used for servers |

We are still looking at what would be the minimal invocations needed for TLS-in-SMPC. 
- It is proven that handshake keys and IVs could be revealed (so encryption/decryption for this part can be done outside).
- It remains intriguing to reveal client/server handshake secrets, which seems to be secure if one opens at the right time, but needs more careful research

Note that we did not have `DeriveServerTrafficSecret.txt` since in our application, decryption of server reponses is never needed. One can easily craft one using the script.

## Synthesis 

The circuits are assembled by the C++ script [here](../generator/).

## Applications

In TLS-in-SMPC, many keys need to be secret-shared and passed over throughout the computation. The flexible input-output AG-MPC [here](https://github.com/n-for-1-auth/emp-agmpc-flex-in-out) can help with this. Basically, the key derivation output is stored in terms of authenticated shares, which can be inputted later into another garbled circuits.

