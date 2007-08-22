#ifndef _DEAFPLUGIN_H
#define _DEAFPLUGIN_H

#include <string>
#include <mysql++/mysql++.h>

#include "DeafBean.h"

class DeafPlugin
{
	public:
//		DeafPlugin();
		virtual ~DeafPlugin()
		{
		}

		virtual bool From_File(const string &InDirName, const string &InFileName, DeafBean &DataLayer) = 0;
		virtual bool From_MySQL(mysqlpp::Query &InQuery, const string &DatabaseName, const string &TableName, DeafBean &DataLayer) = 0;
		virtual bool From_Stream(istream &InStream, DeafBean &DataLayer) = 0;
		
		virtual bool To_File(const DeafBean &DataLayer, const string &OutDirName, const string &OutFileName) = 0;
		virtual bool To_MySQL(const DeafBean &DataLayer, mysqlpp::Query &OutQuery, const string &DatabaseName, const string &TableName) = 0;
		virtual bool To_Stream(const DeafBean &DataLayer, ostream &OutStream) = 0;

//		virtual DeafBean* ReCast(DeafBean* BeanPtr) const = 0;
};

#endif
