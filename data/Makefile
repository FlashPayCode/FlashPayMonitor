# This options is for SOLARIS CC 5.0
#CCOPT=-compat=4

# This options is for SOLARIS CC 4.2
CCOPT=

#-----------------------------------------------------------
#OS=SOLARIS
#CC=gcc
#OPTLIB=-L/usr/local/lib -lz -lsocket -lnsl ${CCOPT}
#OPTINC=
#OPTDEF=${CCOPT} -DHAVE_UNISTD_H -DUSE_MMAP
#THRLIB=
#-----------------------------------------------------------
#OS=SOLGCC
#CC=g++
#OPTLIB=-lz -lsocket -lnsl -lthread
#OPTINC=
#OPTDEF=-D__cplusplus
#THRLIB=migo_thread_native
#-----------------------------------------------------------
#OS=AIXRS
#CC=cc
#OPTLIB=-lpthread -L/usr/lib -L/usr/lib/sna -bimport:/usr/lib/sna/cpic.exp -liconv
#OPTINC=-g -qcpluscmt -I/usr/include -I/usr/include/sna
#OPTDEF=-DHAVE_UNISTD_H -DUSE_MMAP
## -DWITHOUT_HOST
#THRLIB=
#-----------------------------------------------------------
OS=LINUX
CC=g++
OPTLIB=-lpthread -L/usr/local/lib -ljson-c -lcrypto -L../lib -L/usr/local/pgsql/lib
OPTINC= -I/usr/local/include/json-c -I../lib -I/usr/local/pgsql/include
OPTDEF=-Werror -W -Wall -Wpointer-arith -Wcast-align -Wshadow -Wno-long-long -Wformat=2
THRLIB=

OBJS =	CreditTrade.o		\
		Reserve.o		\

STDCFLAGS=

LFLAGS=${OPTLIB}

CFLAGS=${STDCFLAGS} -D${OS} ${OPTINC} ${OPTDEF}

all: libobjs.a

libobjs.a: $(OBJS)
	rm -f libobjs.a
	ar -r libobjs.a $(OBJS)
	ranlib libobjs.a

clean:
	rm -f *.o *.a $(BINS) *~

.cpp.o:
	$(CC) $(CFLAGS) -c -o $@ $<

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

