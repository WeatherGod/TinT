#ifndef _WMO_MOS_C
#define _WMO_MOS_C

using namespace std;

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <ctime>
#include <mysql++/mysql++.h>

#include <DeafPlugin.h>
#include <DeafBean.h>

#include "WMO_MOS.h"
#include "MOSBean.h"
#include <BMOS.h>

#include <boost/regex.hpp>              // boost's regex stuff



WMO_MOS::WMO_MOS()
	:	myReportRegEx("(?:FEUS21|FOUS(?:14|21|44))\\s+(?:KWBC|KWNO)\\s+(:?0[1-9]|[12]\\d|3[01])(:?[01][0-9]|2[0-3])[0-5]\\dZ?")
{
}

WMO_MOS::~WMO_MOS()
{
}



bool WMO_MOS::From_File(const string &InDirName, const string &InFileName, DeafBean &DataLayer)
{
//	MOSBean& BeanLayer = ReCast(DataLayer);
	ifstream WMOStream((InDirName + '/' + InFileName).c_str());

	if (!WMOStream.is_open())
	{
		cerr << "[ERROR] WMO_MOS::From_File(): Could not open file " << InDirName + '/' + InFileName << " for reading.\n";
		return(false);
	}

	const bool Result = From_Stream(WMOStream, DataLayer);

	WMOStream.close();
	return(Result);
}



bool WMO_MOS::To_File(const DeafBean &DataLayer, const string &OutDirName, const string &OutFileName)
{
//	const MOSBean& BeanLayer = ReCast(DataLayer);
	ofstream OutStream((OutDirName + '/' + OutFileName).c_str());

        if (!OutStream.is_open())
        {
                cerr << "[ERROR] WMO_MOS::To_File(): Could not open file " << OutDirName + '/' + OutFileName << " for writing.\n";
                return(false);
        }

	const bool Result = To_Stream(DataLayer, OutStream);

	OutStream.close();


	return(Result);
}



bool WMO_MOS::From_MySQL(mysqlpp::Query &InQuery, const string &DatabaseName, const string &TableName, DeafBean &DataLayer)
{
	MOSBean& BeanLayer = ReCast(DataLayer);

	return(false);
}

bool WMO_MOS::To_MySQL(const DeafBean &DataLayer, mysqlpp::Query &OutQuery, const string &DatabaseName, const string &TableName)
{
	const MOSBean& BeanLayer = ReCast(DataLayer);

	return(false);
}



bool WMO_MOS::From_Stream(istream &InStream, DeafBean &DataLayer)
{
	MOSBean& BeanLayer = ReCast(DataLayer);

	InStream.ignore(64, 1);	// ASCII char 1 begins every report, so 64 characters max should be sufficient to find the first one.
	string ReportRead;

	int ReportNum;
	InStream >> ReportNum;
	InStream.clear();
	InStream.ignore(32, '\n');	// get to the next line.
	getline(InStream, ReportRead);

	while (InStream.good())
	{
		if (IsMOSReportHeader(ReportRead))
		{
			getline(InStream, ReportRead, (char) 3);

			// The Record separator character denotes the beginning of each MOS.
			size_t CharPos = ReportRead.find((char) 30);

			while (CharPos != string::npos)
			{
				const size_t NextPos = ReportRead.find((char) 30, CharPos + 1);

				string MOSStr;

				if (NextPos != string::npos)
				{
					MOSStr = ReportRead.substr(CharPos + 1, NextPos - CharPos);
					//cerr << "[START1]\n" << MOSStr << "\n[END1]\n";
				}
				else
				{
					MOSStr = ReportRead.substr(CharPos + 1);
					//cerr << "[START2]\n" << MOSStr << "\n[END2]\n";
				}

				BMOS AReport( MOSStr );
				

				if (AReport.IsValid())
				{
					BeanLayer.AddReport(AReport);
				}
				else
				{
//					cerr << MOSStr << "\n\n";
				}

				CharPos = NextPos;
			}
		}
		

		while (InStream.peek() != 1 && InStream.peek() != EOF && InStream.good())
		// peeking around for ASCII char 1, which begins the next report, or EOF
		{
			if (ReportRead[0] == 'E' || ReportRead[0] == 'Q' || ReportRead[0] == 'V')
			{
				InStream.ignore(250002, 3);
			}
			else
			{
				InStream.ignore(15008, 3);	// ASCII char 3 ends every report.  Each report has a max size of 15000 characters
								// (unless it is national 'V', pictorial 'Q', or satellite 'E' data, which 
								//  has max of 250000 characters)...
								// 15008 is divisible by 32, so maybe I can get a boost in optimization here?
			}
		}
		
		InStream.get();		// gets that character...(the ASCII char 1 or the EOF)

		
		InStream >> ReportNum;
		InStream.clear();	// sometimes, towards the end of a file, there may be extra stuff that is incorrectly done.
					// I won't be able to read them anyway, so just finish reading the file and move on.
		InStream.ignore(32, '\n');	// Get to the next line.
		getline(InStream, ReportRead);
	}

	
	return(InStream.eof());
}

bool WMO_MOS::To_Stream(const DeafBean &DataLayer, ostream &OutStream)
{
	const MOSBean& BeanLayer = ReCast(DataLayer);
	
//	vector<string>::const_iterator AOrigReport = BeanLayer.Give_StrBegin();
        for (vector<BMOS>::const_iterator AReport = BeanLayer.Give_Begin(); AReport != BeanLayer.Give_End(); AReport++)
        {
//		OutStream << *AOrigReport << '\n';
		OutStream << AReport->Encode() << "\n\n";
	}

	return(true);
}

MOSBean& WMO_MOS::ReCast(DeafBean& BeanRef) const
{
	return(static_cast<MOSBean&>(BeanRef));
}

const MOSBean& WMO_MOS::ReCast(const DeafBean& BeanRef) const
{
	return(static_cast<const MOSBean&>(BeanRef));
}

//------------------------------------------- Private Functions -------------------------------------------------------------
bool WMO_MOS::IsMOSReportHeader(const string &ReportStr) const
{
	return(regex_search(ReportStr, myReportRegEx));
}



// DON"T TOUCH!

extern "C" WMO_MOS* WMO_MOSFactory()
{
	return(new WMO_MOS);
}

extern "C" void WMO_MOSDestructor( WMO_MOS* &p )
{
	if (p != 0)
	{
		delete p;
		p = 0;
	}
}



#endif
