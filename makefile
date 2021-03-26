all:
	g++ main.cpp -o set -msse4.2 -O3 -march=native -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio