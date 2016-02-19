all: build/analyzer build/net-dump

build/:
	mkdir build

build/analyzer: build/
	gcc -Wall analyzer/*.c analyzer/commands/*.c -o build/analyzer -lm

build/net-dump: build/
	gcc -Wall -lpcap -lpthread *.c -o build/net-dump

clean:
	rm -rf build
