# GeneAlign Pro: Sequence Harmony Explorer 🧬

A React web-app implementation of the Smith-Waterman algorithm for identifying common molecular subsequences, based on the 1981 paper by T.F. SMITH and M.S. WATERMAN.

## Technical Stack

- **Core Implementation**: Written in C for performance (and gets pleasure from debugging segmentation fault). 
- **WebAssembly**: Compiles C code to run in the browser.
- **Frontend**: React.js for a modern, responsive user interface.
- **Deployment**: Hosted on Vercel.


## Context

This project implements the algorithm described in:
[Identification of Common Molecular Subsequences](https://arep.med.harvard.edu/pdf/Smith81.pdf)

## Setup and Installation

### Prerequisites

1. Install Emscripten:
```bash
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
emcc --version
```

2. Create a new React project:
```bash
npx create-react-app alignement-react
cd alignement-react
```

### Compiling the C code to WebAssembly

Compile the C implementation using Emscripten:

```bash
emcc identification_common_molecular.c -s WASM=1 -s MODULARIZE=1 \
  -s EXPORTED_RUNTIME_METHODS='["stringToUTF8", "UTF8ToString", "getValue", "ccall", "cwrap"]' \
  -s EXPORTED_FUNCTIONS='["_malloc", "_free", "_align_sequences", "_free_alignment_result"]' \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s ASSERTIONS=1 \
  -s STACK_OVERFLOW_CHECK=2 \
  -o public/common_molecular.js
```

This command generates two files:
- `common_molecular.js`: The JavaScript wrapper for interacting with WebAssembly.
- `common_molecular.wasm`: The compiled WebAssembly module.

### Running the React App

Start the React development server:

```bash
npm start
```

## Usage and Example

To demonstrate the accuracy of our implementation, we can compare our results with the example provided in the original Smith-Waterman paper.

### Input Sequences

1. Sequence from the paper:
   - Seq1: A-A-U-G-C-C-A-U-U-G-A-C-G-G
   - Seq2: C-A-G-C-C-U-C-G-C-U-U-A-G

2. Input in our application:
   - Seq1: AAUGCCAUUGACGG
   - Seq2: CAGCCUCGCUUAG

### Results Comparison

1. Alignment in the paper:
   ```
   -G-C-C-A-U-U-G-
   -G-C-C—U-C-G-
   ```

2. Alignment from our implementation:
   ```
   GCCAUUG
   GCC-UCG
   ```

3. Maximal Similarity Score:
   - Paper: 3.30
   - Our implementation: 3.34

4. Occurrences:
   Our result indicates 1 occurrence, which is correct as there is only one optimal alignment in this example.

### Conclusion

The results from our implementation are remarkably close to the example given in the original Smith-Waterman paper. The obtained alignment is identical, and the difference in the similarity score is very small (3.34 vs 3.30). This slight difference could be explained by minor variations in scoring parameters or numerical precision used in the implementation.

Overall, our implementation works correctly and produces results consistent with the original Smith-Waterman algorithm.

## Contributing

Contributions to improve the implementation or extend its features are welcome. Please feel free to submit pull requests or open issues for any bugs or enhancements.

## License

MIT License