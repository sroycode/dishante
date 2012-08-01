/**
* @project dishante
* @file include/dsh/DbMysql.hpp
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
* DB Access file for Mysql Database
*
*/

#ifndef _DSH_DB_MYSQL_HPP_
#define _DSH_DB_MYSQL_HPP_
#define DSH_DB_MYSQL_HPP_PROGNO 1102
#include <vector>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <apn/Convert.hpp>

#include <mysql.h>
#include <my_global.h>

namespace dsh {
namespace db {
class Mysql {
	typedef std::pair<std::string,std::string> ssPair;
public:

	/**
	* Constructor : default for Mysql
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
	Mysql(
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
	virtual ~Mysql () {}

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

		MYSQL *conn = mysql_init(NULL);
		if (!mysql_real_connect(conn, dbHost_.c_str(), dbUser_.c_str(),
		                        dbPass_.c_str(), dbName_.c_str(),
		                        apn::Convert::AnyToAny<std::string,int>(dbPort_),
		                        NULL, 0)) {
			throw apn::GenericException(DSH_DB_MYSQL_HPP_PROGNO,"Connection to Mysql Failed","");
		}
		mysql_query(conn, getSqlCmd<T>(InVars).c_str());
		MYSQL_RES *res = mysql_store_result(conn);
		if (!res) {
			if (conn) mysql_close(conn);
			throw apn::GenericException(DSH_DB_MYSQL_HPP_PROGNO,"No records found, nothing to SELECT","");
		}
		MYSQL_ROW row;
		while ((row = mysql_fetch_row(res))) {
			U data(header_size);
			std::copy ( row, row+header_size, data.begin() );
			Callback(data);
		}
		mysql_free_result(res);
		mysql_close(conn);
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
#endif /* _DSH_DB_MYSQL_HPP_ */
