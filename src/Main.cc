/**
* @project dishante
* @file src/Main.cc
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
* Default Main file
*
*/

#include <iostream>
#include <boost/assign/list_of.hpp>
#include <apn/CmdLineOptions.hpp>
#include <apn/CfgFileOptions.hpp>
#include <apn/ConnServ.hpp>
#include "Work.hpp"

/** definitions */
apn::CmdLineOptions MyCLO;
apn::CfgFileOptions MyCFG;
apn::CmdLineOptions::eList L = boost::assign::tuple_list_of
                               (DSHN_DEFAULT_STRN_CONFIG,"c","set config file",true)
                               ("verbose","v","set verbosity level",true)
                               (DSHN_DEFAULT_STRN_PORT,"p","set port",true)
                               (DSHN_DEFAULT_STRN_THREADS,"t","no of threads",true)
                               ;
/** definitions */
template<class T>
T FindInSystem(const std::string s, T defval)
{
	T p = defval;
	if (MyCLO.Check(s) )
		p = MyCLO.Find<T>(s);
	else if (MyCFG.Check<T>(DSHN_DEFAULT_STRN_SYSTEM,s) )
		p = MyCFG.Find<T>(DSHN_DEFAULT_STRN_SYSTEM,s);
	return p;
}

int main(int argc, char *argv[])
{

	try {

		MyCLO = apn::CmdLineOptions(argc,argv,L);
		if(MyCLO.Check(DSHN_DEFAULT_STRN_HELP) || (!MyCLO.Check(DSHN_DEFAULT_STRN_CONFIG))) {
			std::cout << MyCLO.GetHelpLine() << std::endl;
			exit(0);
		}

		MyCFG = apn::CfgFileOptions(MyCLO.Find<std::string>(DSHN_DEFAULT_STRN_CONFIG));
		MyCFG.Show();
		/** port: CommandLine, Config , default */
		int port = FindInSystem<int>(DSHN_DEFAULT_STRN_PORT,DSHN_DEFAULT_PORT);
		int threads = FindInSystem<int>(DSHN_DEFAULT_STRN_THREADS,DSHN_DEFAULT_HTTP_THREADS);
		/**  work */
		dshn::Work::pointer Sdata = dshn::Work::create(MyCFG);
		/** http */
		apn::ConnServ<dshn::Work::pointer>::create(threads, port, Sdata->share());

	} catch(apn::GenericException e) {
		std::cerr << e.ErrorCode_ << " " << e.ErrorMsg_ << " " << e.ErrorFor_ << std::endl;
		exit(1);

	} catch(std::exception e) {
		std::cerr << e.what() << std::endl;
		exit(1);

	} catch(...) {
		std::cerr << " Unknown Exit" << std::endl;
		exit(1);
	}

	return 0;
}
