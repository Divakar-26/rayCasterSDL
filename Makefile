all:
	g++ main.cpp -o main -I/usr/local/include/SDL3 -L/usr/local/lib -lSDL3 -lSDL3_image



run:
	g++ main.cpp -o main -I/usr/local/include/SDL3 -L/usr/local/lib -lSDL3 -lSDL3_image
	./main

clean:
	rm main