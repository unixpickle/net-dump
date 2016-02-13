build/net-dump: build/
	gcc -Wall -lpcap -lpthread *.c -o build/net-dump

clean:
	rm -rf build

build/:
	mkdir build
