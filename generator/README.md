# EMP-toolkit Program for Circuit Assembly

This directory contains the EMP-toolkit program that is used to assemble many circuits in this repository.

## How to use?

This program should compile together with [emp-tool](https://github.com/emp-toolkit/emp-tool) through the following steps.

- copy `generate_handshake_circuits.cpp` to the `test` repo of emp-tool.
- modify emp-tool's `CMakeLists.txt` to add `add_test(generate_handshake_circuits)`.
- compile and run `bin/generate_hanshake_circuits`; circuits will show up in the current directory.