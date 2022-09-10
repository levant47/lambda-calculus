set -ex
g++ -Werror -nostdinc -nostdlib include_linux.h -o build.o
rm build.o
