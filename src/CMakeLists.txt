##########Check if boost exists
FIND_PACKAGE(Boost REQUIRED COMPONENTS system thread date_time filesystem iostreams serialization)
if ( Boost_FOUND )
   message( "-- Boost found. include=${Boost_INCLUDE_DIR} libs=${Boost_LIBRARIES}" )
   include_directories( ${Boost_INCLUDE_DIR} )
endif ( Boost_FOUND )

FIND_PACKAGE(Mysql)
if ( MYSQL_FOUND )
   message( "-- Mysql found. include=${MYSQL_INCLUDE_DIR} libs=${MYSQL_LIBRARIES}" )
   include_directories( ${MYSQL_INCLUDE_DIR} )
   SET(COMPILE_WITH_MYSQL)
endif ( MYSQL_FOUND )


FIND_PACKAGE(PostgreSQL)
if ( PostgreSQL_FOUND )
   message( "-- Pgsql found. include=${PostgreSQL_INCLUDE_DIR} libs=${PostgreSQL_LIBRARIES}" )
   include_directories( ${PostgreSQL_INCLUDE_DIR} )
   SET(COMPILE_WITH_PGSQL)
endif ( PostgreSQL_FOUND )



SET(DSH_SOURCE 
  Main.cc
  Work.cc
)
SET(DSH_TARGET dshserver)

INCLUDE_DIRECTORIES("../include")

ADD_EXECUTABLE(${DSH_TARGET} ${DSH_SOURCE})
TARGET_LINK_LIBRARIES(${DSH_TARGET} ${Boost_LIBRARIES} ${MYSQL_LIBRARY} ${PostgreSQL_LIBRARY} pthread)
