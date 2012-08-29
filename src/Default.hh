/**
* @project dishante
* @file src/Default.hh
* @author  S Roychowdhury <sroycode AT gmail DOT com>
* @version 1.0
*
* @section LICENSE
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* General Public License for more details at
* http://www.gnu.org/copyleft/gpl.html
*
* @section DESCRIPTION
*
* Defaults file
*
*/

#ifndef _DSHN_DEFAULT_HH_
#define _DSHN_DEFAULT_HH_

#define DSHN_DEFAULT_PORT 9999
#define DSHN_DEFAULT_HTTP_THREADS 3
#define DSHN_DEFAULT_JOBQ_THREADS 3
#define DSHN_DEFAULT_TEMPDIR "."
#define DSHN_DEFAULT_STRN_DIRSEP "/"
#define DSHN_DEFAULT_STRN_UNDERSCORE "_"
#define DSHN_DEFAULT_STRN_DOT "."

#ifndef DSHN_DEFAULT_COORDT
#define DSHN_DEFAULT_COORDT long int
#endif

#define DSHN_DEFAULT_STRN_HELP "help"
#define DSHN_DEFAULT_STRN_CONFIG "config"
#define DSHN_DEFAULT_STRN_SYSTEM "system"
#define DSHN_DEFAULT_STRN_PORT "port"
#define DSHN_DEFAULT_STRN_THREADS "threads"
#define DSHN_DEFAULT_STRN_DEFAULT "default"

#define DSHN_DEFAULT_STRN_INDEXES "indexes"
#define DSHN_DEFAULT_STRN_INDEXES_SEPARATOR ","

#define DSHN_DEFAULT_STRN_FIELDS "fields"
#define DSHN_DEFAULT_STRN_FIELDS_SEPARATOR ","

#define DSHN_DEFAULT_STRN_DELIM "delim"
#define DSHN_DEFAULT_STRN_FILENAME "filename"

#define DSHN_DEFAULT_STRN_DBHOST "dbhost"
#define DSHN_DEFAULT_STRN_DBPORT "dbport"
#define DSHN_DEFAULT_STRN_DBNAME "dbname"
#define DSHN_DEFAULT_STRN_DBUSER "dbuser"
#define DSHN_DEFAULT_STRN_DBPASS "dbpass"
#define DSHN_DEFAULT_STRN_DBTABLE "dbtable"
#define DSHN_DEFAULT_STRN_DBWHERE "dbwhere"

#define DSHN_DEFAULT_STRN_INDEX "index"
#define DSHN_DEFAULT_STRN_GID "gid"
#define DSHN_DEFAULT_STRN_X "x"
#define DSHN_DEFAULT_STRN_Y "y"
#define DSHN_DEFAULT_STRN_Z "z"
#define DSHN_DEFAULT_STRN_NO "no"

#define DSHN_DEFAULT_STRN_TEMPDIR "tempdir"
#define DSHN_DEFAULT_VAL_TEMPDIR "."

#define DSHN_DEFAULT_STRN_FMT "fmt"
#define DSHN_DEFAULT_VAL_FMT "json"

#define DSHN_DEFAULT_BUFF_SIZE 4096
#define DSHN_DEFAULT_PAGES_USED 10

/** imports */
#define DSH_POINT_DATA_MAX_POINTS 10
#endif
