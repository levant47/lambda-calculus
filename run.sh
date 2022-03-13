set -ex
g++ -I /home/levant/doc/plain/code/c++ -Werror -O2 main.cpp -o main.bin
g++ -I /home/levant/doc/plain/code/c++ -Werror -g main.cpp -o debug.bin
./main.bin
