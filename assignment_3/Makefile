CC=g++
CFLAGS=-Wall -O2

SYSTEMC_HEADERS_PATH=/usr/local/lib/systemc-2.3/include/
SYSTEMC_LIB_PATH=/usr/local/lib/systemc-2.3/lib-linux64/

all:
	$(CC) $(CFLAGS) -c -I=$(SYSTEMC_HEADERS_PATH) -L=$(SYSTEMC_LIB_PATH) -lsystemc initiator.cpp
	$(CC) $(CFLAGS) -c -I=$(SYSTEMC_HEADERS_PATH) -L=$(SYSTEMC_LIB_PATH) -lsystemc channel.cpp
	$(CC) $(CFLAGS) -c -I=$(SYSTEMC_HEADERS_PATH) -L=$(SYSTEMC_LIB_PATH) -lsystemc arbiter.cpp
	$(CC) $(CFLAGS) -c -I=$(SYSTEMC_HEADERS_PATH) -L=$(SYSTEMC_LIB_PATH) -lsystemc target.cpp
	$(CC) $(CFLAGS) -c -I=$(SYSTEMC_HEADERS_PATH) -L=$(SYSTEMC_LIB_PATH) -lsystemc main.cpp

	$(CC) $(CFLAGS) -I=$(SYSTEMC_HEADERS_PATH) -L=$(SYSTEMC_LIB_PATH) -lsystemc initiator.o channel.o arbiter.o target.o main.o -o assignment_3

clean:
	rm *.o
	rm assignment_3
