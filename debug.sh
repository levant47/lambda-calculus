set -ex
g++ -Werror -g main.cpp -o main.bin
gdb main.bin
