gcc -o common_molecular src/identification_common_molecular.c
./common_molecular data/data.txt
emcc --emrun src/identification_common_molecular.c -s WASM=1 -o common_molecular.html
emrun --no_browser --port 8080 common_molecular.html