using namespace std;

#include <iostream>
#include <vector>
#include <string>
#include "DataFile.h"
#include "Reader.h"

#include <unistd.h>			// for getpass() (which I know has been obsoleted, but who cares?)
#include <mysql++/mysql++.h>

#include "WACUtil.h"            // for GetWACDir(), GetDatabaseName(), GetAdminServerName(), GetAdminUserName(), OnAdminSystem()


int main()
{
	if (!OnAdminSystem())
	{
		cerr << "ERROR: You must be on " << GetAdminServerName() << " to administer the WAC database." << endl;
		return(1);
	}

	
        const string WorkingDir = GetWACDir();
	const string DatabaseName = GetDatabaseName();
	const string UserName = GetAdminUserName();
	const string ServerName = GetAdminServerName();


        Reader TableInfo;
        const vector <DataFile> Files = TableInfo.ProcessEntryDefinitions(WorkingDir + "/ConfigFiles/EntryDefinitions");

	if (Files.empty())
        {
                cerr << "ERROR: Problem with the EntryDefinitions file" << endl;
                cerr << "No changes were made..." << endl;
                return(2);
        }

	mysqlpp::Connection WACLink;

        try
        {
                char* ThePass = getpass("Enter WAC Password: ");

                if (ThePass == NULL)
                {
                        cerr << "ERROR: Failure obtaining password.  Exiting..." << endl;
                        throw((string) "PASSWORD ERROR");
                }

                // Need this inside a try-catch block in case it throws an error.  I don't want to lose scope of the password variable...
                try
                {
                        WACLink.connect(DatabaseName.c_str(), ServerName.c_str(), UserName.c_str(), ThePass);
                }
                catch (...)
                {
                        // I know there are better ways to do this, but I am not trying to build anything bullet-proof.
			while (*ThePass != '\0')
			{
				*ThePass++ = '0';
			}
                        
                        throw;
                }

                // I know there are better ways to do this, but I am not trying to build anything bullet-proof.
                while (*ThePass != '\0')
		{
			*ThePass++ = '0';
		}

		mysqlpp::Query TheQuery = WACLink.query();

		/* The idea behind the enumerations for the status is to show the lifespan of a potential volume.
		   At first, it is new, then it becomes listed.  Then, the contents of that listing is Catalogued.
		   After that, the volume may become damaged or destroyed, which is useful to know for retrieving data.
		   By damaged, I mean that some parts of the volume is readable, while others are not.
		   By destroyed, I mean that the volume has been eliminated or physically lost or intentionally destroyed.
		*/

		
		TheQuery << "CREATE TABLE IF NOT EXISTS Volume_Info(VolumeName VARCHAR(60) PRIMARY KEY, SystemName VARCHAR(65) NOT NULL, "
			 << "Directory VARCHAR(255) NOT NULL, MediumType ENUM('DLT', 'Ultrium', 'Disk') NOT NULL, BlockSize INT UNSIGNED NULL, "
			 << "Status ENUM('New', 'Listed', 'Catalogued', 'Damaged', 'Destroyed'))";

		try
		{
			TheQuery.execute();
		}
		catch (...)
		{
			cerr << "ERROR: Problem with query...\nMySQL query: " << TheQuery.preview() << endl;
			throw;
		}




		for (vector<DataFile>::const_iterator AFile = Files.begin(); AFile != Files.end(); AFile++)
	        {
        	        TheQuery << "CREATE TABLE IF NOT EXISTS " << mysqlpp::escape << AFile->GiveFileTypeName() << "(FileName VARCHAR(255) NOT NULL, "
                	         << "FileGroupNum INT UNSIGNED NOT NULL, VolumeName VARCHAR(60) NOT NULL, "
				 << "FileSize INT UNSIGNED NOT NULL, FilingIndex INT UNSIGNED NOT NULL";


	                vector <string> DescriptList = AFile->GiveDescriptors();
        	        vector <string> DeclareList = AFile->GiveDeclarations();

                	for (vector<string>::const_iterator ADescript( DescriptList.begin() ), ADeclare( DeclareList.begin() );
	                     ADescript != DescriptList.end(); ADescript++, ADeclare++)
        	        {
                	        TheQuery << ", " << mysqlpp::escape << *ADescript << ' ' << *ADeclare;
	                }

        	        TheQuery << ")";

			try
			{
				TheQuery.execute();
			}
			catch (...)
			{
				cerr << "MySQL query: " << TheQuery.preview() << endl;
				throw;
			}
		}
        //      cout << endl;
        }
	catch (const exception &Err)
	{
		cerr << "ERROR: Exception caught...\n" << Err.what() << endl;
		cerr << "Possible additional info: " << WACLink.error() << endl;
		WACLink.close();
		return(3);
	}
	catch (const string &ErrStr)
	{
		cerr << "ERROR: Exception caught...\n" << ErrStr << endl;
		cerr << "Possible additional info: " << WACLink.error() << endl;
		WACLink.close();
		return(3);
	}
	catch (...)
	{
		cerr << "ERROR: Unknown exception caught!" << endl;
		cerr << "Possible additional info: " << WACLink.error() << endl;
		WACLink.close();
		return(3);
	}

	WACLink.close();
	return(0);

}


