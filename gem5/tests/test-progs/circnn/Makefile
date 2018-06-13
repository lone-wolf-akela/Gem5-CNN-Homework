main: peripheral.o
	aarch64-linux-gnu-gcc-5 -o main main.c peripheral.o -lm -static

peripheral.o: peripheral.c
	aarch64-linux-gnu-gcc-5 -c peripheral.c

clean:
	rm main peripheral.o
	