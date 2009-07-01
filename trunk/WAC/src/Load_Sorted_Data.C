using namespace std;

#include <iostream>
#include <vector>
#include <string>

#include <unistd.h>		// for access(), remove(), rename() and getpass() (Yeah, I know it is bad, but until an alternative is given, tough luck!)
#include <getopt.h>             // for getopt_long()
#include <errno.h>		// for errno support.
#include <cstdlib>		// for strtol()

#include <mysql++/mysql++.h>

#include "Reader.h"
#include "DataFile.h"
#include <StrUtly.h>		// for Size_tToStr()

#include <WACUtil.h>		// for GetWACDir(), GetDatabaseName(), GetAdminServerName(), GetAdminUserName(), OnAdminSystem()

void PrintSyntax()
{
	cout << "\n\tLoad_Sorted_Data -i | --index _FILINGINDEX_ [-v | --verbose] [-h | --help] [-s | --syntax]\n\n";
}

void PrintHelp()
{
	PrintSyntax();

	cout << "\nThis program takes the sorted subcatalogues of indicated filing index and.\n";
        cout << "loads them into the WAC database.\n\n";

	cout << "\t\t-i  --index\n";
	cout << "\t\t\t Required arguement to indicate which filing index to use for loading the sorted data.\n\n";
	cout << "\t\t-v  --verbose\n";
	cout << "\t\t\t Verbosity level.  Increase level by multiple -v options.\n\n";
	cout << "\t\t-h  --help\n";
	cout << "\t\t\t Prints out the program description and usage page and exits program.\n\n";
	cout << "\t\t-s  --syntax\n";
	cout << "\t\t\t Prints out the program's usage information and quits.\n\n";

	cout << "Author: Ben Root  2007 Penn State Meteorology Department  bvroot [AT] meteo [DOT] psu [DOT] edu\n\n";
}

int main(int argc, char* argv[])
{
	size_t FilingIndex = string::npos;
	int VerboseLevel = 0;

	int OptionIndex = 0;
        int OptionChar = 0;
        bool OptionError = false;
        opterr = 0;     // don't print out error messages...

        static struct option TheLongOptions[] = {
                {"index", 1, NULL, 'i'},
                {"verbose", 0, NULL, 'v'},
                {"help", 0, NULL, 'h'},
		{"syntax", 0, NULL, 's'},
                {0, 0, 0, 0}
        };


        while ((OptionChar = getopt_long(argc, argv, "i:vhs", TheLongOptions, &OptionIndex)) != -1)
        {
		const int PrevErrVal = errno;
                switch(OptionChar)
                {
                case 'i':
			errno = 0;
			FilingIndex = strtol(optarg, NULL, 10);
			if (errno != 0)
			{
				cerr << "ERROR: Invalid arguement.  Need a number for _FILINGINDEX_\n";
				OptionError = true;
			}
			errno = PrevErrVal;
			break;
                case 'v':
                        VerboseLevel++;
                        break;
                case 'h':
                        PrintHelp();
                        return(1);
                        break;
		case 's':
			PrintSyntax();
			return(1);
			break;
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


	if (FilingIndex == string::npos)
	{
		cerr << "ERROR: missing filing index -i\n";
		PrintSyntax();
		return(2);
	}


	if (!OnAdminSystem())
	{
		cerr << "ERROR: Must be on the " << GetAdminServerName() << " to administer the WAC database.\n";
		return(1);
	}

	const string WACDatabase = GetDatabaseName();
	const string WACServer = GetAdminServerName();
	const string WACAdmin = GetAdminUserName();
	const string WorkingDir = GetWACDir();


	const string EntryFileName = WorkingDir + "/ConfigFiles/EntryDefinitions";

	Reader TableInfo;
	const vector <DataFile> Files = TableInfo.ProcessEntryDefinitions(EntryFileName);

	if (Files.empty())
	{
		cerr << "ERROR: Problem with EntryDefinitions file: " << EntryFileName << endl;
		return(2);
	}

	mysqlpp::Connection WACLink;

	try
	{
		char* ThePass = getpass("Enter WAC password: ");

		if (ThePass == NULL)
                {
                        cerr << "ERROR: Failure obtaining password..." << endl;
                        throw((string) "PASSWORD ERROR");
                }

		try
		{
			WACLink.connect(WACDatabase.c_str(), WACServer.c_str(), WACAdmin.c_str(), ThePass);
		}
		catch (...)
		{
			// probably a better way of doing this, but, oh well.
                        while(*ThePass != '\0')
                        {
                                *ThePass++ = '\0';
                        }

			throw;
		}

		// probably a better way of doing this, but, oh well.
                while(*ThePass != '\0')
                {
	                *ThePass++ = '\0';
                }


		mysqlpp::Query TheQuery = WACLink.query();

		

		for (size_t TypeIndex = 0; TypeIndex < Files.size(); TypeIndex++)
		{
			const string FileTypeName = Files[TypeIndex].GiveFileTypeName();
			const string SourceFile = WorkingDir + "/ToBeLoaded/" + FileTypeName + '.' + Size_tToStr(FilingIndex) + ".subcatalogue";

			if (access(SourceFile.c_str(), F_OK) != 0)
			{
				// file doesn't exist, which is perfectly ok since there might not have been
				// any of that type to be loaded.
				continue;
			}

			cout << FileTypeName << endl;

			// allows the files to have quoted string values and to use \ for escaping characters.
			// each row is a record.
			TheQuery << "LOAD DATA INFILE " << mysqlpp::quote << SourceFile
				 << " INTO TABLE " << mysqlpp::escape << FileTypeName
				 << " FIELDS TERMINATED BY ','";

			
			try
			{
				TheQuery.execute();
			}
			catch (...)
			{
				cerr << "ERROR: Problem trying to load " << SourceFile << " into the WAC database!\n";
                                cerr << "Additional info: " << TheQuery.error() << endl;
                                cerr << "The query: " << TheQuery.preview() << endl << endl;
                                continue;
			}

			if (!TheQuery.success())
			{
                                cerr << "ERROR: Problem trying to load " << SourceFile << " into the WAC database!\n";
                                cerr << "Additional info: " << TheQuery.error() << endl;
				cerr << "The query: " << TheQuery.preview() << endl << endl;
				continue;
                        }

			
			const string DestFile = WorkingDir + "/Loaded/" + FileTypeName + '.' + Size_tToStr(FilingIndex) + ".subcatalogue";

			if (access(DestFile.c_str(), F_OK) == 0)
			{
				// This shouldn't have to happen, but I don't want to accidentially overwrite anything, though.
				string SysCommand = "cat " + SourceFile + " >> " + DestFile;
				if (system(SysCommand.c_str()) != 0)
				{
					cerr << "ERROR: Some error occurred trying to cat " << SourceFile << " to " << DestFile << endl;
					// Don't remove the source file!
				}
				else
				{
					// Note, I have verified the existance of the file indicated by SourceFile, so it should
					// be ok to do this without checking results, right?
					// Probably not...what if the user doesn't have write permissions to that directory?
					remove(SourceFile.c_str());
				}
			}
			else
			{
				// the destination file doesn't exist, so just simply move it.
				if (rename(SourceFile.c_str(), DestFile.c_str()) != 0)
				{
					cerr << "ERROR: Some error occurred trying to move " << SourceFile << " to " << DestFile << endl;
				}
			}
		}// end of for loop
	}
	catch (const exception &Err)
	{
		cerr << "ERROR: exception caught: " << Err.what() << endl;
		cerr << "Potentially more info: " << WACLink.error() << endl;
		WACLink.close();
		return(3);
	}
	catch (const string &ErrStr)
	{
		cerr << "ERROR: exception caught: " << ErrStr << endl;
		cerr << "Potentially more info: " << WACLink.error() << endl;
		WACLink.close();
		return(3);
	}
	catch (...)
	{
		cerr << "ERROR: unknown exception caught..." << endl;
		cerr << "Potentially more info: " << WACLink.error() << endl;
		WACLink.close();
		return(3);
	}


	WACLink.close();
	return(0);
}


