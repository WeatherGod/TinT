using namespace std;

#include <iostream>
#include <string>
#include <map>
#include <algorithm>

#include <unistd.h>				// for optarg, opterr, optopt
#include <getopt.h>             // for getopt_long()


#include <ctime>				// for time_t type
#include "TimeUtly.h"			// for GetTimeUTC(), MonthIndex()

#include "JobClass.h"


void PrintSyntax()
{
	cerr << "ListJob (--jobfile | -J) _JOBFILENAME_ [--long | -l]\n"
	     << "        [-f[_FINISHEDJOBFILENAME_] | --finjob[=_FINISHEDJOBFILENAME_]]\n"
	     << "        [-p[_PROBLEMJOBFILENAME_] | --probjob[=_PROBLEMJOBFILENAME_]]\n"
	     << "        [--syntax | -x] [--help | -h]" << endl << endl;
}

void PrintHelp()
{
	PrintSyntax();

	cerr << endl;
	cerr << "Description:  List details of the Job file..." << endl;
	cerr << "  Given -f, but no finished jobfile name, then we assume that the name is the same" << endl;
	cerr << "     as _JOBFILENAME_ with '_Finished' appended to the name." << endl;
	cerr << "  Given -p, but no problem jobfile name, then we assume that the name is the same" << endl;
	cerr << "     as _JOBFILENAME_ with '_Problem' appeneded to the name." << endl;
	cerr << endl;
	cerr << "\t-l" << endl;
	cerr << "\t\tPrint the long list of information about each volume, like file counts and filegroup counts." << endl;
	cerr << endl;
}

int main(int argc, char *argv[])
{
	int OptionIndex = 0;
	int OptionChar = 0;
	opterr = 0;

	
	static struct option TheLongOptions[] = {
		{"jobfile", 1, NULL, 'J'},
		{"finjob", 2, NULL, 'f'},
		{"probjob", 2, NULL, 'p'},
		{"long", 0, NULL, 'l'},
		{"syntax", 0, NULL, 'x'},
		{"help", 0, NULL, 'h'},
		{0, 0, 0, 0}
	};

	string JobFilename = "";
	bool UseFinishedJobFile = false;
	bool UseProblemJobFile = false;
	string FinishedJobFilename = "";
	string ProblemJobFilename = "";
	bool PrintLongFormat = false;		// false - just print out the names.  True - print out additional information.

	while ((OptionChar = getopt_long(argc, argv, "J:f::p::lxh", TheLongOptions, &OptionIndex)) != -1)
	{
		switch (OptionChar)
		{
		case 'J':
			JobFilename = optarg;
			break;
		case 'f':
			UseFinishedJobFile = true;

			if (optarg != NULL)
			{
				FinishedJobFilename = optarg;
			}

			break;
		case 'p':
			UseProblemJobFile = true;

			if (optarg != NULL)
			{
				ProblemJobFilename = optarg;
			}

			break;
		case 'l':
			PrintLongFormat = true;
			break;
		case 'h':
			PrintHelp();
			return(1);
			break;
		case 'x':
			PrintSyntax();
			return(2);
			break;
		case '?':
			cerr << "ERROR: Unknown option: -" << (char) optopt << endl;
			PrintSyntax();
			return(2);
			break;
		case ':':
			cerr << "ERROR: Missing value for arguement: -" << (char) optopt << endl;
			PrintSyntax();
			return(2);
			break;
		default:
			cerr << "ERROR: Programming error... Unaccounted option: -" << (char) OptionChar << endl;
			PrintSyntax();
			return(2);
			break;
		}
	}
		

	if (JobFilename.empty())
	{
		cerr << "You need to specify at least the JobFile's name." << endl;
		cerr << "JobFilename: " << JobFilename << endl;
		PrintSyntax();
		return(2);
	}


	

	JobClass JobRequest;
	JobClass FinishedJobRequest;
	JobClass ProblemJobRequest;

	if (!JobRequest.LoadJobFile(JobFilename))
	{
		cerr << "Problem loading jobfile: " << JobFilename << endl;
		return(3);
	}
	
	if (!JobRequest.IsValid())
	{
		cerr << "Invalid job: " << JobFilename << endl;
		return(4);
	}

	
	if (FinishedJobFilename.empty() && UseFinishedJobFile)
	{
		FinishedJobFilename = JobFilename + "_Finished";
	}
	
/*
		if (access(FinishedJobFilename.c_str(), R_OK | W_OK) != 0)
		{
			// The user didn't specify a finished job file, and a file with the default name of "_Finished" appended
			// onto the regular jobfile name doesn't exist, so we assume that the user has no intention of using a finished jobfile.
			IgnoreFinishedJobFile = true;
		}
*/

	if (UseFinishedJobFile)
	{
		if (!FinishedJobRequest.LoadJobFile(FinishedJobFilename))
		{
			cerr << "ERROR: Problem loading jobfile: " << FinishedJobFilename << endl;
			return(3);
		}

		JobRequest -= FinishedJobRequest;
	}
	
	if (ProblemJobFilename.empty() && UseProblemJobFile)
	{
		ProblemJobFilename = JobFilename + "_Problem";
	}
	
	if (UseProblemJobFile)
	{
		if (!ProblemJobRequest.LoadJobFile(ProblemJobFilename))
		{
			cerr << "ERROR: Problem loading jobfile: " << ProblemJobFilename << endl;
			return(3);
		}
	
		JobRequest -= ProblemJobRequest;
	}
	

	multimap <time_t, VolID_t> VolumeInfo;		// holds the ID of the volumes in chronological order
	
	for (VolumeIter_Const AVol = JobRequest.VolumeBegin(); AVol != JobRequest.VolumeEnd(); AVol++)
	{
		string TempName = AVol->first.GiveName();
		// By definition, the end of every volume name HAS TO BE (in this order):
		//   (1)  An underscore.
		//   (2)  Start of the Date Range for the volume with the date in the order of month, day, year.
		//        The month can be represented using two digits or two-letters.
		//        The day is represented using two digits.
		//        The year is represented using two or four digits.
		//        We assume that if there is only 6 characters for the date, then use %y for the year,
		//        otherwise, use %Y.
		//   (3)  The word 'to'.
		//   (4)  The second date in the same format as above.
		//   Nothing may come after this!

		const size_t UnderscorePos = TempName.rfind('_');
		const size_t DateRangeDelimPos = TempName.rfind("to");

		if (UnderscorePos == string::npos || DateRangeDelimPos == string::npos)
		{
			cerr << "Invalid VolumeName: " << TempName << endl;
			continue;
		}
		
		
		time_t TempDateTime;
		string MonthSpec = "%m";
		string YearSpec = "%Y";

		if (isalpha(TempName[UnderscorePos + 1]))
		{
			// Change the two letter representation into a 2 digit representation.
			// note that this is internal.  This changed name will not be printed out.

			const int MonthNum = MonthIndex(TempName.substr(UnderscorePos + 1, 2));
			char NumStr[3];
			snprintf(NumStr, 3, "%.2i", MonthNum);
			
			TempName.replace(UnderscorePos + 1, 2, NumStr, 0, 2);
		}

		if ((DateRangeDelimPos - UnderscorePos) == 7)
		{
			YearSpec = "%y";
			// if there are only 6 characters between these two delimiters
			// then use the %y format specifier.
		}


		TempDateTime = GetTimeUTC(TempName.substr(UnderscorePos + 1, DateRangeDelimPos - UnderscorePos - 1), 
				       MonthSpec + "%d" + YearSpec);

		


		VolumeInfo.insert(make_pair(TempDateTime, AVol->first));
	}

	size_t MaxNameLength = 0;
	size_t MaxFileGroupCountLength = 0;
	size_t MaxFileCountLength = 0;
	size_t MaxMediumTypeLength = 0;

	if (PrintLongFormat)
	{
		for (multimap<time_t, VolID_t>::const_iterator AVolID = VolumeInfo.begin(); AVolID != VolumeInfo.end(); AVolID++)
		{
			if (AVolID->second.GiveName().length() > MaxNameLength)
			{
				MaxNameLength = AVolID->second.GiveName().length();
			}

			if (IntToStr(JobRequest.GiveFileGroupCount(AVolID->second)).length() > MaxFileGroupCountLength)
			{
				MaxFileGroupCountLength = IntToStr(JobRequest.GiveFileGroupCount(AVolID->second)).length();
			}

			if (IntToStr(JobRequest.GiveFileCount(AVolID->second)).length() > MaxFileCountLength)
			{
				MaxFileCountLength = IntToStr(JobRequest.GiveFileCount(AVolID->second)).length();
			}

			if (JobRequest.GiveVolume_MediumType(AVolID->second).length() > MaxMediumTypeLength)
			{
				MaxMediumTypeLength = JobRequest.GiveVolume_MediumType(AVolID->second).length();
			}
		}
	}


	for (multimap<time_t, VolID_t>::const_iterator AVolID = VolumeInfo.begin(); AVolID != VolumeInfo.end(); AVolID++)
	{
		printf("%*s", MaxNameLength, AVolID->second.GiveName().c_str());

		if (PrintLongFormat)
		{
			printf("   FileGroups: %*u      Files: %*u", 
			       MaxFileGroupCountLength, JobRequest.GiveFileGroupCount(AVolID->second),
			       MaxFileCountLength, JobRequest.GiveFileCount(AVolID->second));

			if (!JobRequest.GiveVolume_MediumType(AVolID->second).empty())
			{
				printf("    MediumType: %*s", 
				       MaxMediumTypeLength, JobRequest.GiveVolume_MediumType(AVolID->second).c_str());
			}
		}

		cout << '\n';
	}

	return(0);
}
