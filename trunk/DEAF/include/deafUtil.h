#ifndef _DEAFUTIL_H
#define _DEAFUTIL_H

#include <string>
#include <vector>
#include <list>

#include <boost/regex.hpp>
#include <mysql++/mysql++.h>


bool EstablishConnection(mysqlpp::Connection &Connect_A, const string &HostName, const string &UserName, const unsigned int PortNum);
bool EstablishConnection(mysqlpp::Connection &Connect_A, mysqlpp::Connection &Connect_B, 
			 const string &HostName, const string &UserName, const unsigned int PortNum);

list <string> FindFileMatches(const string &BasePath, const string &DirName, boost::regex &FilenameRegEx);
vector <string> GetDeafDirs();
bool MakeDirectory(const string &DirectoryPath);


#endif
