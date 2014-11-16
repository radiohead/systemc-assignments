CC=g++
CFLAGS=-Wall -O3

SYSTEMC_HEADERS_PATH=/usr/local/lib/systemc-2.3/include/
SYSTEMC_LIB_PATH=/usr/local/lib/systemc-2.3/lib-linux64/

all:
	mkdir -p ./bin
	$(CC) $(CFLAGS) -I=$(SYSTEMC_HEADERS_PATH) -L=$(SYSTEMC_LIB_PATH) src/main.cpp -lsystemc -o ./bin/assignment

clean:
	rm -rf ./bin/
