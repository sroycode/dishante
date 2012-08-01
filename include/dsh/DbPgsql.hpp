/**
* @project dishante
* @file include/dsh/DbPgsql.hpp
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
* DB Access file for Pgsql Database
*
*/

#ifndef _DSH_DB_PGSQL_HPP_
#define _DSH_DB_PGSQL_HPP_
#define DSH_DB_PGSQL_HPP_PROGNO 1103
#include <vector>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <apn/Convert.hpp>

#include <libpq-fe.h>

namespace dsh {
namespace db {
class Pgsql {
	typedef std::pair<std::string,std::string> ssPair;
public:

	/**
	* Constructor : default for Pgsql
	*
	* @param dbHost
	*   std::string Default for dbHost
	*
	* @param dbPort
	*   std::string Default for dbPort
	*
	* @param dbName
	*   std::string Default for dbName
	*
	* @param dbUser
	*   std::string Default for dbUser
	*
	* @param dbPass
	*   std::string Default for dbPass
	*
	* @param SqlTable
	*   std::string Default for SqlTable
	*
	* @param SqlWhere
	*   std::string Default for SqlWhere
	*
	* @return
	*   none
	*
	*/
	Pgsql(
	    std::string dbHost,
	    std::string dbPort,
	    std::string dbName,
	    std::string dbUser,
	    std::string dbPass,
	    std::string SqlTable,
	    std::string SqlWhere
	) :
		dbHost_(dbHost),
		dbPort_(dbPort),
		dbName_(dbName),
		dbUser_(dbUser),
		dbPass_(dbPass),
		SqlTable_(SqlTable),
		SqlWhere_(SqlWhere)
	{}

	/**
	* virtual destructor
	*/
	virtual ~Pgsql () {}

	/**
	* Process: Process the result
	*
	* @param InVars
	*   T Input Variable and transform List
	*
	* @param Callback
	*   boost::function nullary callback function
	*
	* @return
	*   none
	*/
	template<class T,class U>
	void Process(T InVars, boost::function <void (U)> Callback) {

		std::size_t header_size=InVars.size();

		std::string p = "host=" + dbHost_
		                + " port=" + dbPort_
		                + " user=" + dbUser_
		                + " password=" + dbPass_
		                + " dbname=" + dbName_ ;
		PGconn *conn = PQconnectdb(p.c_str());
		if (PQstatus(conn) != CONNECTION_OK)
			throw apn::GenericException(DSH_DB_PGSQL_HPP_PROGNO,"Connection to Pgsql Failed", PQerrorMessage(conn));
		PGresult *res = PQexec(conn, getSqlCmd<T>(InVars).c_str());
		if (PQresultStatus(res) != PGRES_TUPLES_OK) {
			if (res && conn) PQclear(res);
			if (conn) PQfinish(conn);
			throw apn::GenericException(DSH_DB_PGSQL_HPP_PROGNO,"Tuple Mismatch:",PQerrorMessage(conn));
		}
		if (PQntuples(res) ==0) {
			if (res && conn) PQclear(res);
			if (conn) PQfinish(conn);
			throw apn::GenericException(DSH_DB_PGSQL_HPP_PROGNO,"No records found, nothing to SELECT","");
		}
		int nResults = PQntuples(res);
		int nFields = PQnfields(res);
		for (int i = 0; i < nResults; ++i) {
			U data(header_size);
			for (int j = 0; j < nFields; ++j) {
				data[j]=PQgetvalue(res, i, j);
			}
			Callback(data);
		}
		PQclear(res);
		PQfinish(conn);
	}

private:
	/** Sql Params */
	std::string dbHost_;
	std::string dbPort_;
	std::string dbName_;
	std::string dbUser_;
	std::string dbPass_;
	std::string SqlTable_;
	std::string SqlWhere_;
	/**
	* getSqlCmd : get the Query
	*
	* @param InVars
	*   T Input Variable and transform List
	*
	* @return
	*   std::string
	*/
	template<class T>
	std::string getSqlCmd(T InVars) {
		int ctr=0;
		std::string SqlCmd = "SELECT ";
		BOOST_FOREACH(ssPair sp, InVars) {
			SqlCmd.append( ((ctr>0)?",":" ") + sp.second + " as " + sp.first);
			++ctr;
		}
		SqlCmd.append(" FROM " + SqlTable_) ;
		SqlCmd.append(" WHERE " + SqlWhere_) ;
		return SqlCmd;
	}
};
} // namespace db
} // namespace dsh
#endif /* _DSH_DB_PGSQL_HPP_ */
