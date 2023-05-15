all: main

main: main.cpp
	clang++ -o prog main.cpp -O2 -DNDEBUG

d: main.cpp
	clang++ -o prog main.cpp