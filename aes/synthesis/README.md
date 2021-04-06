# Synthesis of the AES circuits

This directory contains some raw scripts for synthesis. The code is adapted from existing libraries in [SCALE-MAMBA](https://github.com/KULeuven-COSIC/SCALE-MAMBA/tree/master/Circuits).

The main roadmap is as follows:

- Using Synopsys, we convert the VHDL files into net files.
- We use the convertor in SCALE-MAMBA to convert it into the Bristol Fashion circuits. The `clean.py` script is often needed to clean the labels and line breaks, so that SCALE-MAMBA's convertor can work on it.
- We use `convert.py` to make it into a Bristol Format circuit file.
