# Vicuna Analysis

Vicuna is a static analysis tool for OCaml-C FFI code that combines the MOPSA analyzer with the OxCaml compiler to detect runtime errors in C implementations of OCaml external functions.

## Prerequisites

Before running the analysis, you need:

1. **OxCaml Compiler** - A modified OCaml compiler that generates shape annotations
2. **MOPSA Analyzer** - The static analyzer framework
3. **Python 3** - For running the Vicuna script


## Running the Demo

The `demo/` directory contains an example with OCaml external function declarations (`demo.ml`) and their C implementations (`demo.c`). Follow these steps to analyze the code:

### 1. Compile C stub and OCaml file
```bash
export OXCAML_INSTALL=/path/to/oxcaml/installation
gcc -c -I ${OXCAML_INSTALL}/lib/ocaml -o demo_c.o demo.c
${OXCAML_INSTALL}/bin/ocamlopt.opt -bin-annot-cms unix.cmxa -I +unix -o demo.exe demo_c.o demo.ml
```


### 2. Extract externals from .cms file
```bash
${OXCAML_INSTALL}/bin/extract_externals.opt -output-file demo.externals demo.cms
```

### 3. Run Vicuna analysis
```bash
python3 ../vicuna.py -externals demo.externals -input-file demo.c -output-file demo.report -mopsa-exe ../../bin/mopsa -runtime ../runtime -mopsa-config ../../share/mopsa
```

The analysis will generate a report in `demo.report`.
