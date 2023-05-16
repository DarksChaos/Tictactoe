all: main

main: main.cpp
	clang++ -o prog main.cpp -w -std=c++2b -O2 -DNDEBUG

d: main.cpp
	clang++ -o prog main.cpp -Wall -Wextra -std=c++2b