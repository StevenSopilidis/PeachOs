FILES=./build/start.o ./build/peachos.asm.o ./build/stdlib.o ./build/peachos.o ./build/string.o ./build/memory.o
INCLUDES=-I./src
FLAGS= -g

all: ${FILES}
	i686-elf-ld -m elf_i386 -relocatable ${FILES} -o ./stdlib.elf

./build/start.o: ./src/start.asm
	nasm -f elf ./src/start.asm -o ./build/start.o

./build/peachos.asm.o: ./src/peachos.asm
	nasm -f elf ./src/peachos.asm -o ./build/peachos.asm.o

./build/peachos.o: ./src/peachos.c
	i686-elf-gcc ${INCLUDES} $(FLAGS) -std=gnu99 -c ./src/peachos.c -o ./build/peachos.o

./build/stdlib.o: ./src/stdlib.c
	i686-elf-gcc ${INCLUDES} $(FLAGS) -std=gnu99 -c ./src/stdlib.c -o ./build/stdlib.o

./build/string.o: ./src/string.c
	i686-elf-gcc ${INCLUDES} $(FLAGS) -std=gnu99 -c ./src/string.c -o ./build/string.o

./build/memory.o: ./src/memory.c
	i686-elf-gcc ${INCLUDES} $(FLAGS) -std=gnu99 -c ./src/memory.c -o ./build/memory.o


clean:
	rm -rf ${FILES}
	rm ./stdlib.elf