dishante
========

distance search using sfc

what is it
==========

A standalone service to perform NN searches over a large number of points using space-filling curves (STANN library) 

Compiling
=======

you need to have boost mysql /pgsql libraries are optional, 
if you want to compile without just unset the flags COMPILE_WITH_MYSQL and COMPILE_WITH_PGSQL
(use the makefile the cmake is untested)


Running
======

Once Compiled the parameters can be seen from command line.
Generally, you need to have the variables gid,x,y as unsigned long
The type of x,y can be changed in Defaults.hh to double though STANN is faster with unsigned long

If there is a variable z, the space becomes three-dimentional.
Note that we store the 2D and 3D indexes separately, a query with z in parameters is automatically assumed a 3d query.

Sorry for the sparse documentation, I will make a more detailed description when time permits.


