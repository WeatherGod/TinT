#ifndef _KLEITMOS_H
#define _KLEITMOS_H

#include <iostream>
#include <string>
#include <mysql++/mysql++.h>
#include <DeafPlugin.h>
#include <DeafBean.h>

#include <MOSBean.h>
#include <BMOS.h>

class KleitMOS : public DeafPlugin
//Extends the DeafPlugin base class
{
	public:
		KleitMOS();
		virtual ~KleitMOS();

		virtual bool From_File(const string &InDirName, const string &InFileName, DeafBean &DataLayer);
		virtual bool To_File(const DeafBean &DataLayer, const string &OutDirName, const string &OutFileName);

		virtual bool From_MySQL(mysqlpp::Query &InQuery, const string &DatabaseName, const string &TableName, DeafBean &DataLayer);
		virtual bool To_MySQL(const DeafBean &DataLayer, mysqlpp::Query &OutQuery, const string &DatabaseName, const string &TableName);

		virtual bool From_Stream(istream &InStream, DeafBean &DataLayer);
		virtual bool To_Stream(const DeafBean &DataLayer, ostream &OutStream);

	private:
		int GetProbOfPrecip(const BMOS &TheMOS, const int Hourly, const int ForecastHour) const;

	protected:
		//Do not touch the two ReCast() functions
		MOSBean& ReCast(DeafBean& BeanRef);
		const MOSBean& ReCast(const DeafBean& BeanRef);

};

#endif
