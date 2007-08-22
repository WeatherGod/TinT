#ifndef _WMO_MOS_H
#define _WMO_MOS_H

#include <iostream>
#include <string>
#include <mysql++/mysql++.h>
#include <ctime>
#include <DeafPlugin.h>
#include <DeafBean.h>

#include "MOSBean.h"

#include <boost/regex.hpp>


class WMO_MOS : public DeafPlugin
//Extends the DeafPlugin base class
{
	public:
		WMO_MOS();
		virtual ~WMO_MOS();

		virtual bool From_File(const string &InDirName, const string &InFileName, DeafBean &DataLayer);
		virtual bool To_File(const DeafBean &DataLayer, const string &OutDirName, const string &OutFileName);

		virtual bool From_MySQL(mysqlpp::Query &InQuery, const string &DatabaseName, const string &TableName, DeafBean &DataLayer);
		virtual bool To_MySQL(const DeafBean &DataLayer, mysqlpp::Query &OutQuery, const string &DatabaseName, const string &TableName);

		virtual bool From_Stream(istream &InStream, DeafBean &DataLayer);
		virtual bool To_Stream(const DeafBean &DataLayer, ostream &OutStream);

	private:
		boost::regex myReportRegEx;
		struct tm myBestGuessTime;


		bool IsMOSReportHeader(const string &ReportStr) const;

	protected:
		virtual MOSBean& ReCast(DeafBean& BeanRef) const;
		virtual const MOSBean& ReCast(const DeafBean& BeanRef) const;

};

#endif
