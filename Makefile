all: final

final: CImg.h main.cpp
	g++ main.cpp -o final -lX11 -lpthread -I. -O3
	
train: CImg.h train.cpp
	g++ train.cpp -o train -lX11 -lpthread -I. -O3

clean:
	rm final
	rm train
