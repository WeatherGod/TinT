using namespace std;

#include <iostream>
#include <fstream>
#include <string>

#include <unistd.h>		// for remove() and getpass() (Yeah, I know it is bad, but until an alternative is given, tough luck!)
#include <getopt.h>             // for getopt_long()

#include <mysql++/mysql++.h>

#include <WACUtil.h>		// for GetWACDir(), GetDatabaseName(), GetAdminServerName(), GetAdminUserName(), OnAdminSystem()

void PrintSyntax()
{
	cout << "\nLoad_Volume_Data [-v | --verbose] [-h | --help] [-s | --syntax]\n\n";
}

void PrintHelp()
{
	PrintSyntax();

	cout << "\t\t-v  --verbose\n";
	cout << "\t\t\t Each use of -v increases verbosity by one.  Right now, setting this does nothing.\n\n";
	cout << "\t\t-h  --help\n";
	cout << "\t\t\t Prints out the program description and usage page and exits program.\n\n";
	cout << "\t\t-s  --syntax\n";
	cout << "\t\t\t Prints out the program syntax information and exits the program.\n\n";

	cout << "Author: Ben Root  2007 Penn State Meteorology Department  bvroot [AT] meteo [DOT] psu [DOT] edu\n\n";
}



bool UpdateVolTable(mysqlpp::Query &TheQuery, const string &VolumeName, const string &SysName,
                    const string &DriveName, const string &MediumType, const string &BlockSize, const string &StatusVal)
{
        TheQuery.reset();

        TheQuery << "UPDATE Volume_Info SET SystemName = " << mysqlpp::quote << SysName
		 << ", Directory = " << mysqlpp::quote << DriveName
		 << ", MediumType = " << mysqlpp::quote << MediumType
		 << ", BlockSize = " << mysqlpp::quote << BlockSize
		 << ", Status = " << mysqlpp::quote << StatusVal
		 << " WHERE VolumeName = " << mysqlpp::quote << VolumeName;

	mysqlpp::ResNSel TheResult;

	try
	{
                TheResult = TheQuery.execute();
        }
        catch (...)
        {
                cerr << "ERROR: Problem trying to update " << VolumeName << " in the Volume_Info database!\n";
                cerr << "Additional info: " << TheQuery.error() << endl;
                cerr << "The query: " << TheQuery.preview() << endl << endl;
                throw;
        }

	
        return(0 != TheResult.rows);
}



bool LoadVolTable(mysqlpp::Query &TheQuery, const string &VolumeName, const string &SysName, 
		    const string &DriveName, const string &MediumType, const string &BlockSize, const string &StatusVal)
{
	TheQuery.reset();

        TheQuery << "INSERT INTO Volume_Info (VolumeName, SystemName, Directory, MediumType, BlockSize, Status) "
                 << " VALUES(" << mysqlpp::quote << VolumeName << ", " << mysqlpp::quote << SysName
                 << ", " << mysqlpp::quote << DriveName << ", " << mysqlpp::quote << MediumType
		 << ", " << mysqlpp::quote << BlockSize << ", " << mysqlpp::quote << StatusVal;

	mysqlpp::ResNSel TheResult;

        try
        {
	        TheResult = TheQuery.execute();
        }
        catch (...)
        {
        	cerr << "ERROR: Problem trying to load " << VolumeName << " into the Volume_Info table!\n";
                cerr << "Additional info: " << TheQuery.error() << endl;
                cerr << "The query: " << TheQuery.preview() << endl << endl;
		throw;
        }

	if (0 == TheResult.rows)
	{
		cerr << "ERROR: Unable to insert volume " << VolumeName << " into the Volume_Info table.\n"
		     << "The Query: " << TheQuery.preview() << endl << endl;
		return(false);
	}
	else
	{
		return(true);
	}
}



int main(int argc, char* argv[])
{
	int Verbosity = 0;

	int OptionIndex = 0;
        int OptionChar = 0;
        bool OptionError = false;
        opterr = 0;     // don't print out error messages...

        static struct option TheLongOptions[] = {
                {"verbose", 0, NULL, 'v'},
                {"help", 0, NULL, 'h'},
		{"syntax", 0, NULL, 's'},
                {0, 0, 0, 0}
        };


        while ((OptionChar = getopt_long(argc, argv, "vhs", TheLongOptions, &OptionIndex)) != -1)
        {
                switch(OptionChar)
                {
                case 'v':
                        Verbosity++;
                        break;
                case 'h':
                        cout << "\nThis program takes the sorted subcatalogues of indicated filing index and.\n";
                        cout << "loads them into the WAC database.\n\n";
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


	if (!OnAdminSystem())
	{
		cerr << "ERROR: Must be on the " << GetAdminServerName() << " to administer the WAC database.\n";
		return(1);
	}

	const string WACDatabase = GetDatabaseName();
	const string WACServer = GetAdminServerName();
	const string WACAdmin = GetAdminUserName();
	const string WorkingDir = GetWACDir();


	mysqlpp::Connection WACLink;

	const string SourceFile = WorkingDir + "/ToBeLoaded/Volumes.locator";
	const string DestFile = WorkingDir + "/Loaded/Volumes.locator";
	const string ErrFile = WorkingDir + "/ToBeLoaded/Error_Volumes.locator";

        ifstream VolumeStream(SourceFile.c_str());


        if (!VolumeStream.is_open())
        {
        	// file doesn't exist, which is perfectly ok since there might not have been
                // any data to load.

                cerr << "WARNING: Could not find file " << SourceFile << " for volume loading...\n";
		return(3);
        }


	ofstream OutVolume(DestFile.c_str(), ios::app);

	if (!OutVolume.is_open())
	{
		cerr << "ERROR: Could not open file " << DestFile << " for volume writing...\n";
		VolumeStream.close();
		return(3);
	}

	ofstream ErrOutVol(ErrFile.c_str(), ios::app);
	
	if (!ErrOutVol.is_open())
	{
		cerr << "ERROR: Could not open error file " << ErrFile << " for volume writing...\n";
		VolumeStream.close();
		OutVolume.close();
		return(3);
	}

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

		while(*ThePass != '\0')
		{
			*ThePass++ = '\0';
		}
		

		mysqlpp::Query TheQuery = WACLink.query();

		
		char LineRead[100];
		memset(LineRead, '\0', 100);
		VolumeStream.getline(LineRead, 100);

		while (VolumeStream)
		{
			const string LineCopy = LineRead;

			const string VolumeName = strtok(LineRead, ",");
			const string SysName = strtok(NULL, ",");
			const string DriveName = strtok(NULL, ",");
                        const string MediumType = strtok(NULL, ",");
			const string BlockSize = strtok(NULL, ",");
			const string StatusVal = strtok(NULL, ",");

			if (!UpdateVolTable(TheQuery, VolumeName, SysName, DriveName, MediumType, BlockSize, StatusVal))
			{
				if (!LoadVolTable(TheQuery, VolumeName, SysName, DriveName, MediumType, BlockSize, StatusVal))
				{
					ErrOutVol << LineCopy << '\n';
					VolumeStream.getline(LineRead, 100);
					continue;
				}
			}// end if(update was not successful)

			OutVolume << LineCopy << '\n';
			VolumeStream.getline(LineRead, 100);
		}// end of while loop
	}
	catch (const exception &Err)
	{
		cerr << "ERROR: exception caught: " << Err.what() << endl;
		cerr << "Potentially more info: " << WACLink.error() << endl;
		WACLink.close();
		VolumeStream.close();
		OutVolume.close();
		ErrOutVol.close();
		return(3);
	}
	catch (const string &ErrStr)
	{
		cerr << "ERROR: exception caught: " << ErrStr << endl;
		cerr << "Potentially more info: " << WACLink.error() << endl;
		WACLink.close();
		VolumeStream.close();
		OutVolume.close();
		ErrOutVol.close();
		return(3);
	}
	catch (...)
	{
		cerr << "ERROR: unknown exception caught..." << endl;
		cerr << "Potentially more info: " << WACLink.error() << endl;
		WACLink.close();
		VolumeStream.close();
		OutVolume.close();
		ErrOutVol.close();
		return(3);
	}

	if (!VolumeStream.eof())
        {
                cerr << "WARNING: This program may have not completed reading the input file: " << SourceFile << endl;
		VolumeStream.close();
        }
	else
	{
		// since everything has ended successfully, remove this file.
		VolumeStream.close();
		remove(SourceFile.c_str());
	}

	OutVolume.flush();
	if (0 == OutVolume.tellp())
	{
		// since nothing was writen to it, remove it...
		OutVolume.close();
		remove(DestFile.c_str());
	}
	else
	{
		OutVolume.close();
	}


        ErrOutVol.flush();
        if (0 == ErrOutVol.tellp())
        {
		// Since nothing was writen to it, remove it...
	        ErrOutVol.close();
                remove(ErrFile.c_str());
        }
        else
        {
                ErrOutVol.close();
        }

	WACLink.close();
	return(0);
}


