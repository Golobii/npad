mkdir build
g++ -o build/main -I ./src src/*.cpp -lncurses && ./build/main test.py
