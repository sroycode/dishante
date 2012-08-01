dishante
========

distance search using sfc

what is it
==========

A standalone service to perform NN searches over a large number of points using space-filling curves (STANN library) 

Compiling
=======

This is experimental, there is no CMake so please use the makefile
you need to have boost 
mysql /pgsql libraries are optional, if you want to compile without just unset the flags
**SQL_INCLUDE and **SQL_LDFLAGS

cd src && make
