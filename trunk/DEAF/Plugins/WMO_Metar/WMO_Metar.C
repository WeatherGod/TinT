#ifndef _WMO_METAR_C
#define _WMO_METAR_C

using namespace std;

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <ctime>
#include <cmath>	// for nanf
#include <mysql++/mysql++.h>

#include <DeafPlugin.h>
#include <DeafBean.h>

#include "WMO_Metar.h"
#include "MetarBean.h"
#include "BMetar.h"

#include "StrUtly.h"		// for StripWhiteSpace(), TakeDelimitedList(), GiveDelimitedList(), DoubleToStr(), FloatToStr()

#include <boost/regex.hpp>              // boost's regex stuff



WMO_Metar::WMO_Metar()
	:	myReportRegEx("(?:SA|SP)[A-Z][A-Z]\\d?\\d?\\s+[A-Z]{4}\\s+(:?0[1-9]|[12]\\d|3[01])(:?[01][0-9]|2[0-3])[0-5]\\dZ?"),
		myBestGuessTime()
{
	myBestGuessTime.tm_year = 70;
	myBestGuessTime.tm_mon = 0;
	myBestGuessTime.tm_mday = 1;
	myBestGuessTime.tm_hour = 0;
	myBestGuessTime.tm_min = 0;
	myBestGuessTime.tm_sec = 0;
}

WMO_Metar::~WMO_Metar()
{
}



bool WMO_Metar::From_File(const string &InDirName, const string &InFileName, DeafBean &DataLayer)
{
//	MetarBean& BeanLayer = ReCast(DataLayer);
	ifstream WMOStream((InDirName + '/' + InFileName).c_str());

	if (!WMOStream.is_open())
	{
		cerr << "[ERROR] WMO_Metar::From_File(): Could not open file " << InDirName + '/' + InFileName << " for reading.\n";
		return(false);
	}

	// Initialize the BestGuessTime to help this plug-in determine the
	// complete date and time information.  WMO reports do not contain the
	// year or the month.
	size_t DotPos = InFileName.rfind('.');

	if (DotPos != string::npos)
	{
		strptime(InFileName.c_str() + DotPos + 1, "%y%m%d", &myBestGuessTime);
	}

	const bool Result = From_Stream(WMOStream, DataLayer);

	WMOStream.close();
	return(Result);
}



bool WMO_Metar::To_File(const DeafBean &DataLayer, const string &OutDirName, const string &OutFileName)
{
//	const MetarBean& BeanLayer = ReCast(DataLayer);
	ofstream OutStream((OutDirName + '/' + OutFileName).c_str());

        if (!OutStream.is_open())
        {
                cerr << "[ERROR] WMO_Metar::To_File(): Could not open file " << OutDirName + '/' + OutFileName << " for writing.\n";
                return(false);
        }

	const bool Result = To_Stream(DataLayer, OutStream);

	OutStream.close();


	return(Result);
}



bool WMO_Metar::From_MySQL(mysqlpp::Query &InQuery, const string &DatabaseName, const string &TableName, DeafBean &DataLayer)
{
	MetarBean& BeanLayer = ReCast(DataLayer);
	
	InQuery << "SELECT StationID, ReportDate, TimeOfReport, ReportType, IsCorrection, WindSpeed, WindGust, WindDir, "
		<< "       Visibility, Temperature, DewPoint, AltimeterSetting, StationPressure, " 
		<< "       IsWindDirVariable, IsAuto, IsCAVOK "
		<< "FROM " << mysqlpp::escape << TableName;

	mysqlpp::Result QueryResult = InQuery.store();

	if (!QueryResult)
	{
		cerr << "[ERROR] WMO_Metar::From_MySQL(): Problem with mysql request!\n";
		return(false);
	}
	
	if (QueryResult.rows() == 0)
	{
		cerr << "[WARNING] WMO_Metar::From_MySQL(): Empty result set...\n";
	}

	mysqlpp::Row ARow;
	for (mysqlpp::Row::size_type Index = 0; Index < QueryResult.rows(); Index++)
	{
		const mysqlpp::Row ARow = QueryResult.at(Index);
		BMetar AReport;

	        AReport.TakeStationID((string) ARow["StationID"]);
	        struct tm TempDate;
		const mysqlpp::Date TempSQLDate = ARow["ReportDate"];
		const mysqlpp::Time TempSQLTime = ARow["TimeOfReport"];


        	if (TempSQLDate.year < 1900)
        	{
			cerr << "[WARNING] WMO_Metar::From_MySQL(): Bad year: " << TempSQLDate.year << "\n";
                	continue;
        	}


	        TempDate.tm_year = TempSQLDate.year - 1900;


	        if (TempSQLDate.month == 0)
        	{
			cerr << "[WARNING] WMO_Metar::From_MySQL(): Bad month: " << TempSQLDate.month << "\n";
                	continue;
	        }

        	TempDate.tm_mon = (int) TempSQLDate.month - 1;


	        if (TempSQLDate.day == 0)
	        {
			cerr << "[WARNING] WMO_Metar::From_MySQL(): Bad day: " << TempSQLDate.day << "\n";
        	        continue;
	        }

        	TempDate.tm_mday = TempSQLDate.day;


	        TempDate.tm_hour = TempSQLTime.hour;
        	TempDate.tm_min = TempSQLTime.minute;
        	TempDate.tm_sec = TempSQLTime.second;

	        AReport.TakeTime(TempDate);

	        AReport.TakeCodeType((string) ARow["ReportType"]);
	        AReport.IsCorrection((bool) ARow["IsCorrection"]);

	        if (!ARow["WindSpeed"].is_null())
	        {
        	        AReport.TakeWindSpeed((float) ARow["WindSpeed"]);
	        }

	        if (!ARow["WindGust"].is_null())
        	{
                	AReport.TakeWindGust((float) ARow["WindGust"]);
        	}

	        if (!ARow["WindDir"].is_null())
        	{
                	AReport.TakeWindDir((int) ARow["WindDir"]);
        	}

	        if (!ARow["Visibility"].is_null())
        	{
                	AReport.TakeVisibility((float) ARow["Visibility"]);
	        }

	        if (!ARow["Temperature"].is_null())
        	{
                	AReport.TakeTemperature((float) ARow["Temperature"]);
        	}

	        if (!ARow["DewPoint"].is_null())
        	{
                	AReport.TakeDewPoint((float) ARow["DewPoint"]);
	        }

	        if (!ARow["AltimeterSetting"].is_null())
        	{
                	AReport.TakeAltimeterSetting((float) ARow["AltimeterSetting"]);
	        }

	        if (!ARow["StationPressure"].is_null())
        	{
                	AReport.TakeStationPressure((float) ARow["StationPressure"]);
	        }

	        AReport.IsAuto((bool) ARow["IsAuto"]);
	        AReport.IsDirVariable((bool) ARow["IsWindDirVariable"]);
        	AReport.IsCAVOK((bool) ARow["IsCAVOK"]);
		AReport.IsNil(false);
/*

	        AReport.TakeWeatherTypes(TakeDelimitedList((string) ARow["WeatherConditions"], ' '));
        	AReport.TakeCloudTypes(TakeDelimitedList((string) ARow["CloudConditions"], ' '));
	        AReport.TakeRemarks(TakeDelimitedList((string) ARow["Remarks"], ' '));
*/

		BeanLayer.AddReport(AReport);
	}

	return(true);
}

bool WMO_Metar::To_MySQL(const DeafBean &DataLayer, mysqlpp::Query &OutQuery, const string &DatabaseName, const string &TableName)
{
	const MetarBean& BeanLayer = ReCast(DataLayer);

	if (BeanLayer.MetarCount() == 0)
	{
		cerr << "[WARNING] WMO_Metar::To_MySQL(): No data to load!\n";
		return(true);
	}

	OutQuery.reset();
	OutQuery << "INSERT INTO " << mysqlpp::escape << TableName 
		 << " (StationID, ReportDate, TimeOfReport, ReportType, IsCorrection, WindSpeed, WindGust, WindDir, Visibility, "
		 << "Temperature, DewPoint, AltimeterSetting, StationPressure, IsAuto, IsWindDirVariable, IsCAVOK, IsPrecip) "
		 << "VALUES ";

	const double RecordCount = ((BeanLayer.MetarCount() < 100) ? 1.0 : ((double) BeanLayer.MetarCount() / 100.0));
	double NextFlushIndex = RecordCount;
	
	try
	{
		double RecordIndex = 0;

		for (vector<BMetar>::const_iterator AMetar = BeanLayer.Give_Begin();
		     AMetar != BeanLayer.Give_End();
		     AMetar++)
		{
			char DateStr[11], TimeStr[9];
			memset(DateStr, '\0', 11);
			memset(TimeStr, '\0', 9);
			struct tm TempTime = AMetar->GiveTime();

			if (strftime(DateStr, 11, "%Y-%m-%d", &TempTime) == 0)
			{
				cerr << "[WARNING] WMO_Metar::To_MySQL(): Problem converting date.  Result: |" << DateStr << "|\n";
				cerr << "Defaulting to 0000-00-00\n";
				strcpy(DateStr, "0000-00-00");
			}

			if (strftime(TimeStr, 9, "%H:%M:%S", &TempTime) == 0)
			{
				cerr << "[WARNINNG] WMO_Metar::To_MySQL(): Problem converting time.  Reset: |" << TimeStr << "|\n";
				cerr << "Defaulting to 00:00:00\n";
				strcpy(TimeStr, "00:00:00");
			}

			bool IsPrecip = false;
                        vector <WeatherType> TheWeathers = AMetar->GiveWeatherTypes();
                        vector <WeatherType> RecentWeathers = AMetar->GiveRecentWeathers();

                        for (vector<WeatherType>::const_iterator AWeather = TheWeathers.begin(); AWeather != TheWeathers.end(); AWeather++)
                        {
                                if (AWeather->GivePrecipType() != 0)
                                {
                                        IsPrecip = true;
                                        break;
                                }
                        }

                        for (vector<WeatherType>::const_iterator AWeather = RecentWeathers.begin(); AWeather != RecentWeathers.end(); AWeather++)
                        {
                                if (AWeather->GivePrecipType() != 0)
                                {
                                        IsPrecip = true;
                                        break;
                                }
                        }

			OutQuery << '('
				 << mysqlpp::quote << AMetar->GiveStationID() << ','
                                 << mysqlpp::quote << (string) DateStr << ','
				 << mysqlpp::quote << (string) TimeStr << ','
                                 << mysqlpp::quote << AMetar->GiveCodeType() << ','
                                 << AMetar->IsCorrection() << ','
                                 << FloatToStr(AMetar->GiveWindSpeed()) << ','
                                 << FloatToStr(AMetar->GiveWindGust()) << ','
                                 << FloatToStr((AMetar->GiveWindDir() == -1 ? nanf("nan") : (float) AMetar->GiveWindDir())) << ','
                                 << FloatToStr(AMetar->GiveVisibility()) << ','
                                 << FloatToStr(AMetar->GiveTemperature()) << ','
                                 << FloatToStr(AMetar->GiveDewPoint()) << ','
                                 << FloatToStr(AMetar->GiveAltimeterSetting()) << ','
                                 << FloatToStr(AMetar->GiveStationPressure()) << ','
                                 << AMetar->IsAuto() << ','
                                 << AMetar->IsDirVariable() << ','
                                 << AMetar->IsCAVOK() << ','
				 << IsPrecip
				 << ')';
//                                         AMetar->EncodeWeathers(),
//                                         AMetar->EncodeClouds(),
//                                         GiveDelimitedList(AMetar->GiveRemarks(), ' ')
			RecordIndex++;
			if (RecordIndex >= NextFlushIndex)
			{
				NextFlushIndex += RecordCount;
				if (NextFlushIndex > BeanLayer.MetarCount())
				{
					NextFlushIndex = (double) BeanLayer.MetarCount();
				}

				OutQuery.execute();
				
				OutQuery << "INSERT INTO " << mysqlpp::escape << TableName
			              	 << " (StationID, ReportDate, TimeOfReport, ReportType, IsCorrection, "
	                                 << "WindSpeed, WindGust, WindDir, Visibility, "
				         << "Temperature, DewPoint, AltimeterSetting, StationPressure, "
                	                 << "IsAuto, IsWindDirVariable, IsCAVOK, IsPrecip) "
                 			 << "VALUES ";
			}
			else
			{
				OutQuery << ',';
			}
		}
	}
	catch (const mysqlpp::BadConversion &Err)
	{
		cerr << "[ERROR] WMO_Metar::To_MySQL(): Bad Conversion.... " << Err.what() << endl;
		return(false);
	}
	catch (const mysqlpp::Exception &Err)
	{
		cerr << "[ERROR] WMO_Metar::To_MySQL(): mysqlpp exception caught: " << Err.what() << endl;
		return(false);
	}
	catch (...)
	{
		cerr << "[ERROR] WMO_Metar::To_MySQL(): Unknown exception caught..." << endl;
		return(false);
	}

	return(true);
}



bool WMO_Metar::From_Stream(istream &InStream, DeafBean &DataLayer)
{
	MetarBean& BeanLayer = ReCast(DataLayer);

	InStream.ignore(64, 1);	// ASCII char 1 begins every report, so 64 characters max should be sufficient to find the first one.
	string ReportRead;

	int ReportNum;
	InStream >> ReportNum;

	if (InStream.fail())		// Check to see if the last step failed or not...
	{
		InStream.clear();
	}

	InStream.ignore(32, '\n');	// get to the next line.
	getline(InStream, ReportRead);

	while (InStream.good())
	{
		if (IsMetarReportHeader(ReportRead))
		{
			string LastCodeType = "METAR";		// Default to METAR type (vs. SPECI type).

			getline(InStream, ReportRead, (char) 3);

			vector <string> MetarStrs = TakeDelimitedList(ReportRead, '=');

			for (vector<string>::iterator AMetarStr = MetarStrs.begin();
			     AMetarStr != MetarStrs.end();
			     AMetarStr++)
			{
				StripWhiteSpace(*AMetarStr);

				if (AMetarStr->empty() || *AMetarStr == "NIL")
				{
					continue;
				}

				BMetar AReport(*AMetarStr, myBestGuessTime, LastCodeType);
				
				if (AReport.IsValid())
				{
					if (!AReport.IsNil())
					{
						BeanLayer.AddReport(AReport);
					}

					LastCodeType = AReport.GiveCodeType();
				}
			}
		}

		size_t IgnoreHowMuch = 15008;

		if (!ReportRead.empty())
		{
			// Each report has a max size of 15000 characters
	                // (unless it is national 'V', pictorial 'Q', or satellite 'E' data, which
        	        //  has max of 250000 characters)...
                	// 15008 and 250002 are divisible by 32, so maybe I can get a boost in optimization here?
			switch (ReportRead[0])
			{
			case 'E':
			case 'Q':
			case 'V':
				IgnoreHowMuch = 250002;
				break;
			default:
				IgnoreHowMuch = 15008;
				break;
			}
		}
		

		while (InStream.peek() != 1 && InStream.peek() != EOF && InStream.good())
		// peeking around for ASCII char 1, which begins the next report, or EOF
		{
			InStream.ignore(IgnoreHowMuch, 3);	// ASCII char 3 ends every report.
		}
		
		InStream.get();		// gets that character...(the ASCII char 1 or the EOF)

		
		InStream >> ReportNum;  // sometimes, towards the end of a file, there may be extra stuff that is incorrectly done.
					// I won't be able to read them anyway, so just finish reading the file and move on.

		if (InStream.fail())	// Check to see if the last step failed or not...
		{
			InStream.clear();
		}

		InStream.ignore(32, '\n');	// Get to the next line.
		getline(InStream, ReportRead);
	}// end while() loop

	
	return(InStream.eof());
}

bool WMO_Metar::To_Stream(const DeafBean &DataLayer, ostream &OutStream)
{
	const MetarBean& BeanLayer = ReCast(DataLayer);
	
        for (vector<BMetar>::const_iterator AReport = BeanLayer.Give_Begin(); AReport != BeanLayer.Give_End(); AReport++)
        {
		if (!AReport->IsNil())
		{
                	OutStream << AReport->Encode() << '\n';
		}
        }

	return(true);
}


// -------------------------- Don't Touch!! -------------------------------------
MetarBean& WMO_Metar::ReCast(DeafBean& BeanRef) const
{
	return(static_cast<MetarBean&>(BeanRef));
}

const MetarBean& WMO_Metar::ReCast(const DeafBean& BeanRef) const
{
	return(static_cast<const MetarBean&>(BeanRef));
}

//------------------------------------------- Private Functions -------------------------------------------------------------
bool WMO_Metar::IsMetarReportHeader(const string &ReportStr) const
{
	return(regex_search(ReportStr, myReportRegEx));
}



// DON"T TOUCH!

extern "C" WMO_Metar* WMO_MetarFactory()
{
	return(new WMO_Metar);
}

extern "C" void WMO_MetarDestructor( WMO_Metar* &p )
{
	if (p != 0)
	{
		delete p;
		p = 0;
	}
}



#endif
