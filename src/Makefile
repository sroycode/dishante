## Linux
CC = g++
CCFLAGS = -ansi -Wall -Wno-deprecated -O2 -DNDEBUG -I../include -I/opt/local/include
LDFLAGS = -rdynamic -L/opt/local/lib

MYSQL_INCLUDE = $(shell mysql_config --include) -DCOMPILE_WITH_MYSQL
MYSQL_LDFLAGS = $(shell mysql_config --libs)

PGSQL_INCLUDE = -I$(shell pg_config --includedir) -DCOMPILE_WITH_PGSQL
PGSQL_LDFLAGS = -L$(shell pg_config --pkglibdir) -lpq

BOOST_INCLUDE = -DBOOST_HAS_THREADS
BOOST_LDFLAGS = -rdynamic -lboost_system-mt -lboost_serialization-mt -lboost_regex-mt -lboost_thread-mt

SOURCES = Main.o Work.o

all:	dshserver

dshserver:	$(SOURCES)
	$(CC) $(CCFLAGS) $(LDFLAGS) $(MYSQL_LDFLAGS) $(PGSQL_LDFLAGS) $(BOOST_LDFLAGS) -o dshserver $(SOURCES)

Main.o:	Main.cc
	$(CC) -c $(CCFLAGS) $(BOOST_INCLUDE) Main.cc -o Main.o

Work.o:	Work.cc
	$(CC) -c $(CCFLAGS) $(MYSQL_INCLUDE) $(PGSQL_INCLUDE) Work.cc -o Work.o

strip:
	strip dshserver

clean:
	rm -f dshserver *.o

install:
	cp dshserver /usr/local/bin/
