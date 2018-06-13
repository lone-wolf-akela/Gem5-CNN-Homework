main: test.cpp ai_chip.o core.o memhub.o memory.o
	aarch64-linux-gnu-g++ -o main test.cpp ai_chip.o core.o memhub.o memory.o -lm -static -std=c++11 -O3

ai_chip.o: ai_chip.cpp
	aarch64-linux-gnu-g++ -c ai_chip.cpp -std=c++11 -O3

core.o: core.cpp
	aarch64-linux-gnu-g++ -c core.cpp -std=c++11 -O3
	
memhub.o: memhub.cpp
	aarch64-linux-gnu-g++ -c memhub.cpp -std=c++11 -O3

memory.o: memory.cpp
	aarch64-linux-gnu-g++ -c memory.cpp -std=c++11 -O3
		
clean:
	rm main ai_chip.o core.o memhub.o memory.o
	