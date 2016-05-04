all: final

final: CImg.h main.cpp
	g++ main.cpp -o final -lX11 -lpthread -I.
	
train: CImg.h train.cpp
	g++ train.cpp -o train -lX11 -lpthread -I.

clean:
	rm final
	rm train
