using namespace std;

#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#include <unistd.h>			// for access(), remove()
#include <getopt.h>			// for getopt_long()

#include <csignal>			// for some signal handling.  I am very weak in this area, but this should be sufficient.

#include "Reader.h"
#include "DataFile.h"

#include <boost/regex.hpp>		// boost's regex stuff

#include <StrUtly.h>			// for StripWhiteSpace(), TakeDelimitedList(), GiveDelimitedList(), 
					//     IntToStr(), StrToSize_t(), Size_tToStr(), OnlyDigits()
#include <TimeUtly.h>			// for TakeMonthStr()

#include "WACUtil.h"			// for GetWACDir()


/*  SigHandle will be the function called for SIGINT signals.  It will simply set an ignore for SIGINT
    and send out a message saying that the program is in the process of shutting down.
    Then, it will set a flag to true.  Execution will then resume where it left off before the SIGINT.
    At strategic points in the code, there are checks for the flag.  If it is true at that point, an exception
    will be thrown, triggering a safe shutdown of the program.
*/

void SigHandle(int SigNum);
bool SIGNAL_CAUGHT = false;		// flag for indicating that a signal was caught and it is time to clean up!


string EvaluateExpression(const string &Express, const vector <string> &LineList, const vector <string> &Captures)
{
	if (Express[0] == '\\')					// assume the next character is a number
	{
		size_t CaptureIndex = StrToSize_t(Express.substr(1));

		try
		{
			return(Captures.at(CaptureIndex));
		}
		catch (const exception &Err)
		{
			cerr << "ERROR: Bad index value for captures.  CaptureIndex: " << CaptureIndex << endl;
			return("");
		}
	}
	else if (Express.find("GetYearFromLabel(") == 0)
	{
		// Syntax:  GetYearFromLabel(TapeLabel, Month)
		// This part of the algorithm assumes that the years that the volume contains only spans 2 years.
		// Any more than two years, and this algorithm may yield incorrect results.
		// This is only to be used when the file names do not contain the year for the data.

		// not meant to handle complex groupings.  Keep it simple!
		size_t OpenParenPos = Express.find('(');
		size_t CloseParenPos = Express.rfind(')');
	
		vector <string> Arguements = TakeDelimitedList(Express.substr(OpenParenPos + 1, CloseParenPos - (OpenParenPos + 1)), ',');
		
		if (Arguements.size() != 2)
		{
			cerr << "ERROR: Expression syntax error.  Only 2 arguements for GetYearFromLabel() List: |";
			cerr <<  Express.substr(OpenParenPos + 1, CloseParenPos - (OpenParenPos + 1)) << "|  Size = " << Arguements.size() << "\n";
			cerr << "\tGetYearFromLabel(TapeLabel, Month)" << endl;
			return("");
		}

//		cout << "Getting TapeLabel\n";
		string TapeLabel = EvaluateExpression(RipWhiteSpace(Arguements[0]), LineList, Captures);
//		cout << "Got That!  Getting MonthNum\n";
		size_t MonthNum = TakeMonthStr(EvaluateExpression(RipWhiteSpace(Arguements[1]), LineList, Captures));
//		cout << "Got That!\n";

		size_t SpotPos = TapeLabel.rfind("to");
	        if (SpotPos == string::npos)
		{
                        cerr << "ERROR: Couldn't find 'to' in the tapelabel name: " << LineList[2] << endl;
                        return("");
                }

		//  This next part of the algorithm makes an assumption that the volume name will not have anything after the date information.
		string FirstYear = "";
		string FirstMonth = "";
		string SecondYear = "";

		if ((TapeLabel.size() - SpotPos) == 10)
		{
			// Four-digit years
			FirstYear = TapeLabel.substr(SpotPos - 4, 4);
			FirstMonth = TapeLabel.substr(SpotPos - 8, 2);
			SecondYear = TapeLabel.substr(SpotPos + 6, 4);
		}
		else
		{
			// two-digit years
			FirstYear = LineList[2].substr(SpotPos - 2, 2);                 // 2 chars before 'to' are year numbers
               		FirstMonth = LineList[2].substr(SpotPos - 6, 2);
       			SecondYear = LineList[2].substr(SpotPos + 6, 2);                // 6 chars after the t in 'to' has the first char for year
		}
	
                if (MonthNum >= TakeMonthStr(FirstMonth))
		{
//			cerr << "Got First\n";
               	        return(FirstYear);
       		}
       		else
       	        {
//			cerr << "Got Second\n";
  		        return(SecondYear);
                }
       	}
	else if (Express.find("BestValue(") == 0)
	{
		// Nothing complex, please!
		size_t OpenParenPos = Express.find('(');
               	size_t CloseParenPos = Express.rfind(')');

		vector <string> Arguements = TakeDelimitedList(Express.substr(OpenParenPos + 1, CloseParenPos - (OpenParenPos + 1)), ',');

		if (Arguements.size() == 0)
		{
			cerr << "ERROR: Expression syntax error.  Need at least one arguement for BestValue()  List: |";
			cerr << Express.substr(OpenParenPos + 1, CloseParenPos - (OpenParenPos + 1)) << "\n\n";
			return("");
		}

		size_t ArgIndex = 0;
		// Goes through the arguements and tries to evaluate each arguement, returning the first one that doesn't return empty string
		while (ArgIndex < Arguements.size())
		{
			string ArgValue = EvaluateExpression(RipWhiteSpace(Arguements[ArgIndex]), LineList, Captures);
			if (ArgValue != "")
			{
				return(ArgValue);
			}
			ArgIndex++;
		}
		cerr << "ERROR: No good Values found!\n";
		return("");
	}
	else if (Express.find("$Tape_Label") == 0)
	{
		return(LineList[2]);
	}
	else if (Express.find('"') == 0)
	{
		// Doesn't support nested quotes
		size_t OpenQuotePos = 0;
		size_t CloseQuotePos = Express.find('"', OpenQuotePos + 1);

		if (CloseQuotePos != string::npos)
		{
			return(Express.substr(OpenQuotePos + 1, CloseQuotePos - (OpenQuotePos + 1)));
		}
		else
		{
			cerr << "ERROR: Missing closing quote in string literal for the PatternExplain: " << Express << "\n";
			return("");
		}
	}
	else if (Express.find('\'') == 0)
        {
                // Doesn't support nested quotes 
                size_t OpenQuotePos = 0;
                size_t CloseQuotePos = Express.find('\'', OpenQuotePos + 1);

                if (CloseQuotePos != string::npos)
                {
                        return(Express.substr(OpenQuotePos + 1, CloseQuotePos - (OpenQuotePos + 1)));
                }
                else
                {
                        cerr << "ERROR: Missing closing quote in string literal for the PatternExplain: " << Express << "\n";
                        return("");
                }
        }

	else
	{
		return(Express);
	}
}


vector <string> GatherInfo(const vector <string> &LineList, const vector <string> &Pieces, const vector <string> &PatternExplains, 
			   const vector <string> &AssignExpressions, bool &IsSuccessful)
// IsSuccessful needs to be set either true or false by the end of the function.  Don't assume it has any value to start with.
{
	IsSuccessful = true;			// set to true to start, try to prove as false.
//	cout << "Going into GatherInfo()\n";
	string DateTimeFormat = "";
	string DateTimeStr = "";

	vector <string> Descriptors(1, "");			// A spot for the datetime descriptor, which is mandatory

	for (vector<string>::const_iterator APattern( PatternExplains.begin() ), AnExpression( AssignExpressions.begin() ); 
	     APattern != PatternExplains.end(); APattern++, AnExpression++)
	{
//		cout << "   Top of the for loop\n";
		if (APattern->find("$Date") == 0)
		{
			if (*APattern == "$Date_Year")
	                {
				//The TempYearStr checks the length of the string to see if it is 4 digits or not
				const string TempYearStr = EvaluateExpression(*AnExpression, LineList, Pieces);
				DateTimeStr += TempYearStr + "-";

				if (TempYearStr.length() == 2)
				{
					DateTimeFormat += "%y-";		// takes the year within century (0-99).  69 through 99 are treated as
                                                                                // 20th century and 00-68 are treated as 21st century.
                                                                                // this is according to the strptime man file.

				}
				else
				{
					DateTimeFormat += "%Y-";
				}
                	}
	                else if (*APattern == "$Date_Month")
        	        {
				const string TempMonthStr = EvaluateExpression(*AnExpression, LineList, Pieces);
				DateTimeStr += TempMonthStr + "-";
				if (OnlyDigits(TempMonthStr))
				{
					DateTimeFormat += "%m-";
				}
				else
				{
					// If it isn't digits, then it must be a month name.
					DateTimeFormat += "%b-";
				}
	                }
        	        else if (*APattern == "$Date_Day")
                	{
				DateTimeStr += EvaluateExpression(*AnExpression, LineList, Pieces) + "-";
				DateTimeFormat += "%d-";
	                }
        	        else if (*APattern == "$Date_DayOfYear")
                	{
	                        DateTimeStr += EvaluateExpression(*AnExpression, LineList, Pieces) + "-";
				DateTimeFormat += "%j-";
        	        }
			else
			{
				cerr << "ERROR: Invalid PatternExplain word! PatternExplains[]: |" << *APattern << "|\n";
				IsSuccessful = false;
			}
		}
		else if (APattern->find("$Time") == 0)
		{
			DateTimeStr += EvaluateExpression(*AnExpression, LineList, Pieces) + "-";

			if (*APattern == "$Time_Hour")
	                {
				if (find(PatternExplains.begin(), PatternExplains.end(), "$Time_Meridian") == PatternExplains.end())
				{
					// We are not expecting any AM or PM, so expect this to be in 24 hour format.
					DateTimeFormat += "%H-";
				}
				else
				{
					// We are expecting an AM or PM, so this must be a 12 hour clock hour.
                                        DateTimeFormat += "%I-";
				}
	                }
                	else if (*APattern == "$Time_Meridian")
        	        {
				DateTimeFormat += "%p-";
        	        }
	                else if (*APattern == "$Time_Minute")
                	{
				DateTimeFormat += "%M-";
	                }
                	else if (*APattern == "$Time_Second")
        	        {
				DateTimeFormat += "%S-";
                	}
        	        else if (*APattern == "$Time_MinSec")
	                {
				DateTimeFormat += "%M%S-";
        	        }
			else
                        {
				cerr << "ERROR: Invalid PatternExplain word! PatternExplains[]: |" << *APattern << "|\n";
				IsSuccessful = false;
                        }
		}
		else
		{
			Descriptors.push_back(EvaluateExpression(*AnExpression, LineList, Pieces));
		}
	}

	struct tm DateTime;
	// Initializing structure in case not all information is available (for example, typically seconds and minutes are not given).
	DateTime.tm_year = 0;
	DateTime.tm_mon = 0;
	DateTime.tm_mday = 1;
	DateTime.tm_hour = 0;
	DateTime.tm_min = 0;
	DateTime.tm_sec = 0;
 
	if (strptime(DateTimeStr.c_str(), DateTimeFormat.c_str(), &DateTime) == '\0')
	{
		cerr << "ERROR: Mistake with parsing the time!  DateTimeStr: " << DateTimeStr << "   DateTimeFormat: " << DateTimeFormat << endl;
		Descriptors[0] = "0000-00-00 00:00:00";
		IsSuccessful = false;
	}
	else
	{
		char DateTimeCStr[20];
		if (strftime(&DateTimeCStr[0], 20, "%Y-%m-%d %H:%M:%S", &DateTime) == 0)
		{
			// nothing was put into the DateTimeCStr!
			cerr << "ERROR: Mistake with building datetime string!" << endl;
			Descriptors[0] = "0000-00-00 00:00:00";
			IsSuccessful = false;
		}
		else
		{
			Descriptors[0] = DateTimeCStr;
		}
	}
	
	return(Descriptors);
}

vector <string> ObtainCaptures(const boost::regex &RegExpress, const string &TargetStr, const size_t &MatchListSize)
{
	boost::match_results<string::const_iterator> Matches;			// holds the sub-expressions captured.
	regex_search(TargetStr.begin(), TargetStr.end(), Matches, RegExpress);

	vector <string> Captures(MatchListSize);

	for (size_t Index = 0; Index < Matches.size(); Index++)
	{
		if (Matches[Index].matched)
                {
                        Captures[Index].assign(Matches[Index].first, Matches[Index].second);
                }
		else
		{
			Captures[Index] = "";
		}
	}

	return(Captures);
}


bool DealWithMatch(const vector <string> &LineList, const boost::regex &RegExpress, const DataFile &TypeEntry, ofstream &TableStream,
		   const size_t FilingIndex)
// MAJOR robustness issues here!  See notes in this function.
{
//	cerr << "Going into DealWithMatch()\n";
	bool IsSuccessful;
	const vector <string> Captures = ObtainCaptures(RegExpress, LineList[0], TypeEntry.PatternExplainCount() + 1);
//	cerr << "Got Captures\n";

	const vector <string> Descriptors = GatherInfo(LineList, Captures, TypeEntry.GiveAllPatternExplains(), 
						       TypeEntry.GiveAllAssignExpressions(), IsSuccessful);
//	cerr << "Got Descriptors\n";

	// May need to put in controls of some sort to determine whether something should be quoted or not.
	if (IsSuccessful)
	{
		TableStream << GiveDelimitedList(LineList, ',') << ',' << FilingIndex << ',' << GiveDelimitedList(Descriptors, ',') << "\n";
	}


	// This above segment prints to the appropriate subcatalogue file the following info:
	//	Filename, File Group number, Volume Name, FileSize, FilingIndex, and Date information for the file, and any other descriptor information.
	// there is always DateTime_Info for Descriptors[0]

	return(TableStream.good());
}


size_t GetIndexFiling(const string &WorkingDir)
{
	size_t FileCounter = 0;

	while (FileCounter < string::npos)
	{
        	string NewMasterFilename = WorkingDir + "/Sorted/Master." + Size_tToStr(FileCounter) + ".catalogue";
		string NewMissedFilename = WorkingDir + "/Unsorted/MissedFiles." + Size_tToStr(FileCounter) + ".catalogue";

		if (access(NewMasterFilename.c_str(), F_OK) == 0 || access(NewMissedFilename.c_str(), F_OK) == 0)
		{
			// one of the two Files exists already.
			FileCounter++;
		}
		else
		{
			return(FileCounter);
		}
	}

	cerr << "ERROR: Too many master files and missed files exist!  Please clean up!" << endl;
	return(string::npos);
}



void PrintSyntax()
{
	cout << "\n\tSorter [-f | --fast] [-v | --verbose] [-h | --help]\n\n";
}

void PrintHelp()
{
	PrintSyntax();

	cout << "\t\t-f  --fast\n";
	cout << "\t\t\t Speeds up the sorting process.\n";
	cout << "\t\t\t IMPORTANT:  Only use this option if you know the files being processed would go\n";
	cout << "\t\t\t             through the program normally without any errors.  Do not use this\n";
	cout << "\t\t\t		    option otherwise!  For example, use it for when you need to reprocess\n";
	cout << "\t\t\t		    a catalogue.  The time saved is SIGNIFICANT!\n";

	cout << "\t\t-v  --verbose\n";
	cout << "\t\t\t Set the verbosity parameter within the program.  Each -v increases level by one.\n";
	cout << "\t\t\t NoiseLevel of 0 would limit screen output to only the standard error stream.\n";
	cout << "\t\t\t NoiseLevel of 3 would print out any files that have trouble being sorted.\n";
	cout << "\t\t\t NoiseLevel of 4 and larger would print out even the stupid proceedural messages.\n\n";

	cout << "\t\t-h --help\n";
	cout << "\t\t\t Prints out the program description and usage page and exits program.\n\n";

	cout << "Author: Ben Root   2003 to 2007   Penn State Meteorology Department  bvroot [AT] meteo [DOT] psu [DOT] edu\n\n";
}


void CloseTables(ofstream* TableFiles, vector <string> &FileNames, size_t FileCount)
// If the table file is empty, then remove it.  No need to clutter up the directories, right?
{
	if (FileNames.size() < FileCount)
	{
		cerr << "ERROR: Sorry, you tried to close more files than you definitely have.\n";
		FileCount = FileNames.size();
	}

       	for (size_t i = 0; i < FileCount; i++)
        {
		TableFiles[i].flush();
		if (0 == TableFiles[i].tellp())
		{
			TableFiles[i].close();
			remove(FileNames[i].c_str());
		}
		else
		{
			TableFiles[i].close();
		}
       	}
}


void DumpCat(ifstream &Cata, ofstream &Missed, const string &OldLine)
// Needs to be improved...
// This is too slow.
{
	if (!OldLine.empty())
	{
		Missed << OldLine << '\n';
	}

	string LineRead = "";
	getline(Cata, LineRead);

	while (Cata)
	{
		Missed << LineRead << '\n';
		getline(Cata, LineRead);
	}
}


int main(int argc, char *argv[])
{
	const string WorkingDir = GetWACDir();

	bool DoTest = true;		// true for doing the meticulous checks.  Recommended.
	int ReturnVal = 0;		// zero for successful operations.  This will be used in the try...catch blocks later.
	int LevelOfNoise = 2;		// default level
	
	int OptionIndex = 0;
	int OptionChar = 0;
	bool OptionError = false;
	opterr = 0;			// don't print out error messages...

	static struct option TheLongOptions[] = {
		{"fast", 0, NULL, 'f'},
		{"verbose", 0, NULL, 'v'},
		{"help", 0, NULL, 'h'},
		{0, 0, 0, 0}
	};

	
	while ((OptionChar = getopt_long(argc, argv, "fvh", TheLongOptions, &OptionIndex)) != -1)
	{
		switch(OptionChar)
		{
		case 'f':
			DoTest = false;
			break;
		case 'v':
			LevelOfNoise++;
			break;
		case 'h':
			cout << "\nThis program takes the unprocessed catalogue files and processes them into the WAC database.\n";
			cout << "based on the filetype that a listed archived filename matches.\n\n";
			PrintHelp();
			return(1);
			break;		// not really necessary, is it?
		case '?':
			cerr << "ERROR: Unknown arguement: -" << (char) optopt << endl;
			OptionError = true;
			break;
		case ':':
			cerr << "ERROR: Missing value for arguement: -" << (char) optopt << endl;
			OptionError = true;
			break;
		default:
			cerr << "ERROR: Programming error...  Unaccounted for option: -" << (char) OptionChar << endl;
			OptionError = true;
			break;
		}
	}

	if (OptionError)
	{
		PrintSyntax();
		return(2);
	}

	const string EntryFileName = WorkingDir + "/ConfigFiles/EntryDefinitions";

	Reader TableInfo;
	const vector <DataFile> Files = TableInfo.ProcessEntryDefinitions(EntryFileName);

	if (Files.empty())
	{
		cerr << "ERROR: Problem with EntryDefinitions file: " << EntryFileName << endl;
		return(3);
	}

	const size_t FileCount = Files.size();
	const size_t FilingIndex = GetIndexFiling(WorkingDir);

	if (FilingIndex == string::npos)
	{
		cerr << "ERROR: The WAC needs maintainence!  Too many filings have been done!" << endl;
		return(4);
	}

       	vector <boost::regex> RegularExpressions(FileCount);		// An element for each major data type
	vector <string> FileNames(FileCount, "");
	ofstream TableFiles[FileCount];					// c-style vector of ofstreams.  An element for each major data type
									// note that a C++ style vector cannot be done here without some sort of
									// wrapper class to make a copy contructor, since ofstream does not have one.

	if (LevelOfNoise >= 2) { cout << "The Regular expression strings:" << endl; }

	for (size_t TypeIndex = 0; TypeIndex < FileCount; TypeIndex++)
	{
		if (LevelOfNoise >= 2) { cout << TypeIndex << ". " << Files[TypeIndex].GiveSearchString() << endl; }

		const string TempFilename = WorkingDir + "/ToBeLoaded/" + Files[TypeIndex].GiveFileTypeName() 
					    + "." + Size_tToStr(FilingIndex) + ".subcatalogue";
		FileNames[TypeIndex] = TempFilename;
		TableFiles[TypeIndex].open(TempFilename.c_str());

		bool CantOpen = !(TableFiles[TypeIndex].is_open());
		bool CantCompile = false;

		try
		{
			RegularExpressions[TypeIndex] = Files[TypeIndex].GiveSearchString();
		}
		catch (boost::regex_error &Err)
		{
			CantCompile = true;
			if (LevelOfNoise < 2) { cout << TypeIndex << ". " << Files[TypeIndex].GiveSearchString() << endl; }
			cerr << TypeIndex << ". " << (string(Err.position(), ' ')) << '^' << endl;
			cerr << "ERROR: Trouble compiling reg_ex for " << Files[TypeIndex].GiveFileTypeName() << "   Error Code: " << Err.code()
			     << "  at position: " << Err.position() << endl;
		}
		catch (...)
		{
			CantCompile = true;
			cerr << "ERROR: Unexpected exception.  This shouldn't happen, but it did.  Maybe boost::regex can throw other exceptions?\n";
		}
                                                         
                if (CantCompile || CantOpen)
                {
			CloseTables(TableFiles, FileNames, TypeIndex);		// note that the value for TypeIndex is the count for the
										// number of files that have been opened so far.

			if (CantOpen)
			{
				cerr << "ERROR: Can't open file: " << TempFilename << endl;
			}

			cerr << "Due to error, the program will not continue.  No changes have been made...\n";
			return(3);
                }
	}


	const string CataFileName = WorkingDir + "/ToBeSorted/Unsorted.catalogue";
	const string MissedFileName = WorkingDir + "/ToBeSorted/MissedFiles." + Size_tToStr(FilingIndex) + ".catalogue";
	const string MasterFileName = WorkingDir + "/Sorted/Master." + Size_tToStr(FilingIndex) + ".catalogue";

	ifstream CatalogueFile(CataFileName.c_str());
	if (!CatalogueFile.is_open())
        {
                cerr << "ERROR: File could not be opened: " << CataFileName << endl;
		CloseTables(TableFiles, FileNames, FileCount);
                return(3);
        }

	
	ofstream MasterCatalogue(MasterFileName.c_str());
	if (!MasterCatalogue.is_open())
        {
                cerr << "ERROR: File could not be opened: " << MasterFileName << endl;
		CatalogueFile.close();
		CloseTables(TableFiles, FileNames, FileCount);
                return(3);
        }


	ofstream MissedFiles(MissedFileName.c_str());
	if (!MissedFiles.is_open())
	{
		cerr << "ERROR: File could not be opened: " << MissedFileName << endl;
		CatalogueFile.close();
		MasterCatalogue.close();
		remove(MasterFileName.c_str());
		CloseTables(TableFiles, FileNames, FileCount);
		return(3);
	}

	string LineRead = "";		// it is out here because I need it to be in scope if an exception is thrown.

	try
	{
		signal(SIGINT, SigHandle);	// at this point, SigHandle() function will be called when SIGINT is sent

		/* At any point after the above line a SIGINT is sent to this program, it should then initiate a proper shutdown sequence.
		   It is not fully tested, but at least the mechanism is in place for future improvements.  Also, I have not set up abilities
		   to recognize and deal with any other signals.  And a SIGKILL signal or the likes can still cause significant issues if it
		   is called to this program!  If a SIGKILL is raised, the user will have to remove the latest subcatalogues in the ToBeLoaded
		   directory, or modify the CatalogueFile to exclude those lines.  The same will have to be done to MasterCatalogue and
		   MissedFiles.

		   If SIGINT is raised, the system will finish what it was doing and the rest of the CatalogueFile will be dumped into 
		   MissedFiles, and you can replace the CatalogueFile with the MissedFile and re-run the program, hopefully.  
		   That is the theory, anyway.
		*/

		if (LevelOfNoise >= 4) { cout << "\nReading catalogue" << endl; }

		if (DoTest)             // This sorter code verifies the uniqueness of each match
                {
			getline(CatalogueFile, LineRead);

			while (CatalogueFile.good())
			{
				bool GoodMatch = false;		// must be proven true.
				const vector <string> LineList = TakeDelimitedList(LineRead, ',');
				// Makes dangerous assumption that there are no other commas in these values.
                                // The TakeDelimitedList() function is dumb and will not ignore values in quotes or 'escaped' commas.
                                // So, if a volume name or a file name has a comma in it, THIS WILL BREAK!!
				// Also assume that there are 4 items in this list...

				vector <size_t> MatchIndices(0);		// will contain the indices of the elements of the regex vector
									// that make a match. If multiple matches are made, then
									// the vector size will be greater than 1.  If no matches were made,
									// the vector size will be zero.
	                        
				for (size_t RegExIndex = 0; RegExIndex < FileCount; RegExIndex++)
				{
					if (regex_match(LineList[0], RegularExpressions[RegExIndex]))
	        			{
        	                		MatchIndices.push_back(RegExIndex);
                			}
        			}

				
				if (MatchIndices.size() == 1)
				{
					// DealWithMatch can possibly throw exceptions, just so you know.
					GoodMatch = DealWithMatch(LineList, RegularExpressions[MatchIndices[0]], 
						      		  Files[MatchIndices[0]], TableFiles[MatchIndices[0]], FilingIndex);
				}
				else if (MatchIndices.empty())
		        	{
					if (LevelOfNoise >= 3) { cout << "|" << LineList[0] << "|: No matches\n"; }
			        }
				else
				{
					// the vector has more than one matches
					if (LevelOfNoise >= 3)
					{
				               	cout << LineList[0] << " has " << MatchIndices.size() << " matches\n";
					        cout << "Match with: ";
					        for (size_t i = 0; i < MatchIndices.size(); i++)
				        	{
				                	cout << "# " << MatchIndices[i] << "   ";
			                	}
			               		cout << endl;
					}
				}

				if (GoodMatch)
				{
					MasterCatalogue << LineRead << '\n';
				}
				else
				{
					MissedFiles << LineRead << '\n';
				}

				LineRead = "";		// Resetting LineRead to empty string.  That way, if I need to dump the file
							// I can check to see if LineRead itself was already written to somewhere.

				if (SIGNAL_CAUGHT)
                                {
                                        // If a signal was caught, now that I am at the bottom of the loop, I can go ahead
                                        // and dump the rest of the catalogue.
					// Note, do not put this if-statement after the getline command because, in that case,
					// the line that is read before the if-statement would never get dumped.
                                        throw 2;        // just some number I choose for now.  Don't know why, probably will change this later.
                                }

				getline(CatalogueFile, LineRead);
			}// end while loop
		}
		else		// This sorter code is faster, but will NOT verify the uniqueness of the match.
		{
			getline(CatalogueFile, LineRead);
			size_t LastMatchIndex = 0;		// useful for speeding up the non-testing searching

			while (CatalogueFile.good())
			{
				const vector <string> LineList = TakeDelimitedList(LineRead, ',');
				// Makes dangerous assumption that there are no other commas in these values.
				// The TakeDelimitedList() function is dumb and will not ignore values in quotes or 'escaped' commas.
				// So, if a volume name or a file name has a comma in it, THIS WILL BREAK!!
                                // Also assume that there are 4 items in this list...

				bool GoodMatch = false;		// must prove as true.

				// First check with the last used regular expression.  More likely to match that way.
				if (regex_match(LineList[0], RegularExpressions[LastMatchIndex]))
                        	{
        	        		GoodMatch = DealWithMatch(LineList, RegularExpressions[LastMatchIndex],
                                                      		  Files[LastMatchIndex], TableFiles[LastMatchIndex], FilingIndex);
        	                }
				else
				{
					for (size_t RegExIndex = 0; RegExIndex < FileCount; RegExIndex++)
					{
						if (RegExIndex != LastMatchIndex && 
						    regex_match(LineList[0], RegularExpressions[RegExIndex]))
						{
							LastMatchIndex = RegExIndex;
							GoodMatch = DealWithMatch(LineList, RegularExpressions[LastMatchIndex],
                                                				  Files[LastMatchIndex], TableFiles[LastMatchIndex], FilingIndex);
							break;
						}
					}
				}

				if (GoodMatch)
				{
					MasterCatalogue << LineRead << '\n';
				}
				else
				{
					if (LevelOfNoise >= 3) { cout << LineRead << '\n'; }

					MissedFiles << LineRead << '\n';
				}

				LineRead = "";			// resetting LineRead so I may be able to check and see if it has
								// already been written to somewhere.

				if (SIGNAL_CAUGHT)
                                {
                                        // If a signal was caught, now that I am at the bottom of the loop, I can go ahead
                                        // and dump the rest of the catalogue.
                                        // Note, do NOT put this if-statement after the getline command because, in that case,
                                        // the line that is read before the if-statement would never get dumped.
                                        throw 2;        // just some number I choose for now.  Don't know why, probably will change this later.
                                }


				getline(CatalogueFile, LineRead);
			}// end while (CatalogueFile.good())
		} // end if (DoTest)
	}
	catch (boost::regex_error &Err)
        {
                DumpCat(CatalogueFile, MissedFiles, LineRead);
                cerr << "ERROR: Unexpected boost::regex error caught!  Nothing else should be throwing this." << endl;
                ReturnVal = 4;
        }
        catch (const exception &Err)
        {
		DumpCat(CatalogueFile, MissedFiles, LineRead);
                cerr << "ERROR: Error caught: " << Err.what() << endl;
                ReturnVal = 3;
        }
	catch (int ErrNum)
	{
		DumpCat(CatalogueFile, MissedFiles, LineRead);
                cerr << "ERROR: Error number caught: " << ErrNum << endl;
		ReturnVal = 3;
	}
	catch (...)
	{
		DumpCat(CatalogueFile, MissedFiles, LineRead);
                cerr << "ERROR: Unknown error caught..." << endl;
		ReturnVal = 3;
	}

	if (LevelOfNoise >= 4) { cout << "Closing the catalogues\n"; }

	MissedFiles.flush();
	if (0 == MissedFiles.tellp())
	{
		MissedFiles.close();
		remove(MissedFileName.c_str());
	}
	else
	{
		MissedFiles.close();
	}

	MasterCatalogue.flush();
	if (0 == MasterCatalogue.tellp())
	{
		MasterCatalogue.close();
		remove(MasterFileName.c_str());
	}
	else
	{
		MasterCatalogue.close();
	}
				
	CloseTables(TableFiles, FileNames, FileCount);

	if (ReturnVal == 0)
	{
		CatalogueFile.close();
		remove(CataFileName.c_str());
	}
	else
	{
		CatalogueFile.close();
	}

	if (LevelOfNoise >= 4) { cout << "\nProgram finished\n" << endl; }

	return(ReturnVal);
}

void SigHandle(int SigNum)
/* This will disable SIGINT, print out a message, set SIGNAL_CAUGHT = true, and then send execution back to the code.
   at the bottom of the loops going through the unsorted catalogue, there are if-statements that checks SIGNAL_CAUGHT,
   and throws an error number if it is true.
   The idea behind this is that the SIGINT could happen at any time, even during the printing of a line of information.
   Therefore, the code must be allowed to resume to finish whatever it was doing, and then allow the program to pick a
   safe time to shutdown the system without any fears.

   It would be nice to be able to know when even signals like SIGKILL are sent, so that I may possibly get a chance to just finish
   whatever is being done at the moment.
*/
{
	signal(SIGINT, SIG_IGN);
	cerr << "\n--- Signal caught ----\n";
	cerr << "Proceeding to shut down the sorter.  SIGINT signals have been disabled at this point...\n";
	SIGNAL_CAUGHT = true;
}
