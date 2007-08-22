#ifndef _KLEITMOS_C
#define _KLEITMOS_C

using namespace std;

#include <iostream>
#include <fstream>
#include <string>
#include <mysql++/mysql++.h>

#include <DeafPlugin.h>
#include <DeafBean.h>

#include <KleitMOS.h>
#include <MOSBean.h>

#include <StrUtly.h>			// for FloatToStr()

KleitMOS::KleitMOS()
{
}

KleitMOS::~KleitMOS()
{
}



bool KleitMOS::From_File(const string &InDirName, const string &InFileName, DeafBean &DataLayer)
{
	MOSBean& BeanLayer = ReCast(DataLayer);
	return(false);
}

bool KleitMOS::To_File(const DeafBean &DataLayer, const string &OutDirName, const string &OutFileName)
{
	const MOSBean& BeanLayer = ReCast(DataLayer);

	if (BeanLayer.MOSCount() == 0)
	{
		cerr << "[WARNING] KleitMOS::To_File(): No data to output!\n";
		return(true);
	}

	ofstream OutStream;

	OutStream.open((OutDirName + '/' + OutFileName).c_str());

	if (!OutStream.is_open())
	{
		cerr << "[ERROR] KleitMOS::To_File(): Could not open file " << OutDirName << '/' << OutFileName << " for writing!\n";
		return(false);
	}

	for (vector<BMOS>::const_iterator A_MOS = BeanLayer.Give_Begin();
	     A_MOS != BeanLayer.Give_End();
	     A_MOS++)
	{
		char DateStr[20];
                memset(DateStr, '\0', 20);
                struct tm TempTime = A_MOS->GiveTime();

                if (strftime(DateStr, 20, "%Y-%m-%d %H:%M:%S", &TempTime) == 0)
                {
	                cerr << "[WARNING] KleitMOS::To_File(): Problem converting date.  Result: |" << DateStr << "|\n";
                        cerr << "Defaulting to 0000-00-00 00:00:00\n";
                        strcpy(DateStr, "0000-00-00 00:00:00");
                }

		OutStream << A_MOS->GiveStationID() << ' ' << DateStr << ' ' << A_MOS->GiveModelType() << ' '
			  << A_MOS->GiveTemperature(12) << ' ' << A_MOS->GiveTemperature(24) << ' '
			  << A_MOS->GiveTemperature(36) << ' ' << A_MOS->GiveTemperature(48) << ' '
			  << A_MOS->GiveTemperature(60) << ' '
			  << GetProbOfPrecip(*A_MOS, 12, 24) << ' ' << GetProbOfPrecip(*A_MOS, 12, 36) << ' '
			  << GetProbOfPrecip(*A_MOS, 12, 48) << ' ' << GetProbOfPrecip(*A_MOS, 12, 60) << "\n\n";

	}

	OutStream.close();

	return(true);
}



bool KleitMOS::From_MySQL(mysqlpp::Query &InQuery, const string &DatabaseName, const string &TableName, DeafBean &DataLayer)
{
	MOSBean& BeanLayer = ReCast(DataLayer);
	return(false);
}

bool KleitMOS::To_MySQL(const DeafBean &DataLayer, mysqlpp::Query &OutQuery, const string &DatabaseName, const string &TableName)
{
	const MOSBean& BeanLayer = ReCast(DataLayer);

	if (BeanLayer.MOSCount() == 0)
	{
		cerr << "[WARNING] KleitMOS::To_MySQL(): No data to output!\n";
		return(true);
	}

	OutQuery.reset();
	OutQuery << "INSERT INTO " << mysqlpp::escape << TableName
		 << " (StationID, ReportTime, ModelType, "
		 << "Temp_12hr, Temp_24hr, Temp_36hr, Temp_48hr, Temp_60hr, "
		 << "POP12_24hr, POP12_36hr, POP12_48hr, POP12_60hr) "
		 << "VALUES(%0q,%1q,%2q,%3,%4,%5,%6,%7,%8,%9,%10,%11)";

	try
	{
		OutQuery.parse();

		for (vector<BMOS>::const_iterator A_MOS = BeanLayer.Give_Begin();
		     A_MOS != BeanLayer.Give_End();
		     A_MOS++)
		{
			char DateStr[20];
                        memset(DateStr, '\0', 20);
                        struct tm TempTime = A_MOS->GiveTime();

                        if (strftime(DateStr, 20, "%Y-%m-%d %H:%M:%S", &TempTime) == 0)
                        {
                                cerr << "[WARNING] KleitMOS::To_MySQL(): Problem converting date.  Result: |" << DateStr << "|\n";
                                cerr << "Defaulting to 0000-00-00 00:00:00\n";
                                strcpy(DateStr, "0000-00-00 00:00:00");
                        }

			OutQuery.execute(A_MOS->GiveStationID(),
                                         (string) DateStr,
                                         A_MOS->GiveModelType(),
					 FloatToStr(A_MOS->GiveTemperature(12)),
					 FloatToStr(A_MOS->GiveTemperature(24)),
					 FloatToStr(A_MOS->GiveTemperature(36)),
					 FloatToStr(A_MOS->GiveTemperature(48)),
					 FloatToStr(A_MOS->GiveTemperature(60)),
					 GetProbOfPrecip(*A_MOS, 12, 24),
					 GetProbOfPrecip(*A_MOS, 12, 36),
					 GetProbOfPrecip(*A_MOS, 12, 48),
					 GetProbOfPrecip(*A_MOS, 12, 60)
			);
		}
	}
	catch (const mysqlpp::BadConversion &Err)
	{
		cerr << "[ERROR] KleitMOS::To_MySQL(): Bad Conversion.... " << Err.what() << endl;
                return(false);
        }
        catch (const mysqlpp::Exception &Err)
        {
                cerr << "[ERROR] KleitMOS::To_MySQL(): mysqlpp exception caught: " << Err.what() << endl;
                return(false);
        }
	catch (const string &ErrStr)
	{
		cerr << "[ERROR] KleitMOS::To_MySQL(): " << ErrStr << endl;
		return(false);
	}
	catch (const exception &Err)
	{
		cerr << "[ERROR] KleitMOS::To_MySQL(): " << Err.what() << endl;
		return(false);
	}
        catch (...)
        {
                cerr << "[ERROR] KleitMOS::To_MySQL(): Unknown exception caught..." << endl;
                return(false);
	}


	return(true);
}



bool KleitMOS::From_Stream(istream &InStream, DeafBean &DataLayer)
{
	MOSBean& BeanLayer = ReCast(DataLayer);
	return(false);
}

bool KleitMOS::To_Stream(const DeafBean &DataLayer, ostream &OutStream)
{
	const MOSBean& BeanLayer = ReCast(DataLayer);
	return(false);
}



int KleitMOS::GetProbOfPrecip(const BMOS &TheMOS, const int Hourly, const int ForecastHour) const
{
	int Result;

	try
	{
		switch (Hourly)
		{
		case 6:
			Result = TheMOS.GiveProbOfPrecip_6hr(ForecastHour);
			break;
		case 12:
			Result = TheMOS.GiveProbOfPrecip_12hr(ForecastHour);
			break;
		default:
			Result = 0;
			break;
		}
	}
	catch (...)
	{
		Result = -1;
	}

	return(Result);
}





//-------------- DO NOT TOUCH CODE BELOW! -----------------------------------------

MOSBean& KleitMOS::ReCast(DeafBean& BeanRef)
{
	return(static_cast<MOSBean&>(BeanRef));
}

const MOSBean& KleitMOS::ReCast(const DeafBean& BeanRef)
{
	return(static_cast<const MOSBean&>(BeanRef));
}

extern "C" KleitMOS* KleitMOSFactory()
{
	return(new KleitMOS);
}

extern "C" void KleitMOSDestructor( KleitMOS* &p )
{
	if (p != 0)
	{
		delete p;
		p = 0;
	}
}


#endif
