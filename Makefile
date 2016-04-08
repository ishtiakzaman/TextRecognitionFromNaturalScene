all: CImg.h main.cpp
	g++ main.cpp -o final -lX11 -lpthread -I. -O3

clean:
	rm final
