all: final

final: CImg.h main.cpp
	g++ main.cpp -o final -lX11 -lpthread -I.

clean:
	rm final
