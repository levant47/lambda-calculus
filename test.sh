set -ex
g++ -I /home/levant/doc/plain/code/c++ -Werror -O2 test.cpp -o test.bin
g++ -I /home/levant/doc/plain/code/c++ -Werror -g test.cpp -o debug.bin
./test.bin
