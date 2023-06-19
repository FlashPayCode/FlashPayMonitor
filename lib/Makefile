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
OS=LINUX
CC=g++
OPTLIB=-lpthread -L/usr/local/lib -ljson-c -lcrypto
OPTINC= -I/usr/local/include/json-c
OPTDEF=-Werror -W -Wall -Wpointer-arith -Wcast-align -Wshadow -Wno-long-long -Wformat=2
THRLIB=
#-----------------------------------------------------------
#OS=AIXRS
#CC=cc
#OPTLIB=-lpthread -L/usr/lib -L/usr/lib/sna -bimport:/usr/lib/sna/cpic.exp -liconv
#OPTINC=-g -qcpluscmt -I/usr/include -I/usr/include/sna
#OPTDEF=-DHAVE_UNISTD_H -DUSE_MMAP
## -DWITHOUT_HOST
#THRLIB=
#-----------------------------------------------------------
OBJS =	commu.o		\
	StopWatch.o	\
	Thread.o	\
	CsvInput.o	\
	sha256.o	\
	aes256.o	\
	htmlpost.o	\
	md5.o		\
	util.o		\
	Log.o		\
    child.o     \
#	Menu.o		\

BINS =	TestLib

STDCFLAGS=

LFLAGS=${OPTLIB}

CFLAGS=${STDCFLAGS} -D${OS} ${OPTINC} ${OPTDEF}

all: $(BINS)

TestLib: libsgp.a TestLib.o
	$(CC) -o $@ TestLib.o $(OBJS) -L. -lsgp $(LFLAGS)

libsgp.a: $(OBJS)
	rm -f libsgp.a
	ar -r libsgp.a $(OBJS)
	ranlib libsgp.a

clean:
	rm -f *.o *.a $(BINS) *~

.cpp.o:
	$(CC) $(CFLAGS) -c -o $@ $<

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<
