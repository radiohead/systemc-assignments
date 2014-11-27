CC=g++
CFLAGS=-Wall -O2

SYSTEMC_HEADERS_PATH=/usr/local/lib/systemc-2.3/include/
SYSTEMC_LIB_PATH=/usr/local/lib/systemc-2.3/lib-linux64/

all:
	$(CC) $(CFLAGS) -I=$(SYSTEMC_HEADERS_PATH) -L=$(SYSTEMC_LIB_PATH) -lsystemc main.cpp -o assignment_2

clean:
	rm assignment_2
