main: main.c peripheral.o
	aarch64-linux-gnu-gcc -o main main.c peripheral.o -lm -static

peripheral.o: peripheral.c
	aarch64-linux-gnu-gcc -c peripheral.c

clean:
	rm main peripheral.o
	