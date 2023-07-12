.PHONY: build
.PHONY: run
.PHONY: clean
.PHONY: test

FLAGS = -std=c++20 -Wall -fsanitize=leak -o

build:
	g++ main.cpp ${FLAGS} main.o

run: build
	./main.o $(DF)

clean:
	rm -rf *.o