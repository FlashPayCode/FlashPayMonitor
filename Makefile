OS=LINUX
CC=g++
OPTLIB=-lpthread -L./lib -lsgp -L/usr/local/pgsql/lib -lpq -L/usr/local/lib -ljson-c -lcurl -lcrypto -L./data -lobjs
OPTINC=-I./lib -I/usr/local/pgsql/include -I/usr/local/include/json-c -I./data
#OPTINC=-I../lib -I/usr/include/postgresql -I/usr/include/json/
OPTDEF=-Werror -W -Wall -Wpointer-arith -Wcast-align -Wshadow -Wno-long-long -Wformat=2
THRLIB=
STDCFLAGS=
LFLAGS=${OPTLIB}
CFLAGS=${STDCFLAGS} -D${OS} ${OPTINC} ${OPTDEF}

BINS =	monitorsrv	\



all: $(BINS)

monitorsrv: monitorsrv.o main.o main_db.o returnmsg.o
	$(CC) -o $@ monitorsrv.o main.o main_db.o returnmsg.o $(LFLAGS)

clean:
	rm -f *.o *.a $(BINS) *~

install:
	cp -f monitorsrv	bin/

	
.cpp.o:
	$(CC) $(CFLAGS) -c -o $@ $<

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

