using namespace std;

#include <iostream>

#include <fstream>
#include <string>
#include <vector>
#include <map>

#include "JobClass.h"
#include <unistd.h>			// also needed for optarg, opterr, optopt
#include <getopt.h>             	// for getopt_long()

#include "VolumeInfo.h"			// for VolumeInfo struct

void PrintSyntax()
{
	cerr << "MakeJob [--help | -h] [--syntax | -x]\n"
	     << "        (--jobfile | -J) _JOBFILE_ \n"
	     << "        (--catalog | -c) _SUBCATALOGFILE_\n"
	     << "        (--volumeinfo | -v) _VOLUMELOCATOR_\n";

	cerr << endl;
}

void PrintHelp()
{
	PrintSyntax();
	
	cerr << "This program takes subcatalog files that can come from a WAC query and converts\n"
	     << "them into a job request file.\n\n";
	cerr << "Note that multiple subcatalog files can be given through multiple uses of -c.\n\n";
}

map<string, VolumeInfo> LoadVolumeLocator(const string &FileName)
{
	ifstream VolumeStream(FileName.c_str());

	if (!VolumeStream.is_open())
	{
		cerr << "ERROR: Unable to open file: " << FileName << endl;
		return(map<string, VolumeInfo>());
	}

	map<string, VolumeInfo> AvailVols;

	char LineRead[256];
	memset(LineRead, '\0', 256);

	VolumeStream.getline(LineRead, 256);

	while (VolumeStream.good())
	{
		VolumeInfo TempInfo;

		TempInfo.VolumeName = strtok(LineRead, ",");
		strtok(NULL, ",");				// just the server name.
		TempInfo.DirName = strtok(NULL, ",");
		TempInfo.MediumType = strtok(NULL, ",");
		char* BlockSizeStr = strtok(NULL, ",");

		if (BlockSizeStr != NULL && strcmp(BlockSizeStr, "\\N") != 0)
		{
			TempInfo.BlockSize = strtol(BlockSizeStr, NULL, 10);
		}

		TempInfo.TakeStatusStr(strtok(NULL, ","));

		AvailVols[TempInfo.VolumeName] = TempInfo;

		
		VolumeStream.getline(LineRead, 256);
	}

	if (!VolumeStream.eof())
	{
		cerr << "WARNING: Unable to finish processing file: " << FileName << endl;
	}

	VolumeStream.close();
	return(AvailVols);
}


int main(int argc, char *argv[])
{
	vector <string> SubcatalogueNames(0);
	string JobFilename = "";
	string VolumeLocatorFile = "";

	int OptIndex = 0;
	int OptionChar = 0;
	opterr = 0;

	
	static struct option TheLongOptions[] = {
		{"jobfile", 1, NULL, 'J'},
		{"catalog", 1, NULL, 'c'},
		{"volumeinfo", 1,NULL,'v'},
		{"syntax", 0, NULL, 'x'},
		{"help", 0, NULL, 'h'},
		{0, 0, 0, 0}
	};

	while ((OptionChar = getopt_long(argc, argv, "J:c:v:xh", TheLongOptions, &OptIndex)) != -1)
	{
		switch (OptionChar)
		{
		case 'J':
			JobFilename = optarg;
			break;
		case 'c':
			SubcatalogueNames.push_back(optarg);
			break;
		case 'v':
			VolumeLocatorFile = optarg;
			break;
		case 'x':
			PrintSyntax();
			return(2);
			break;
		case 'h':
			PrintHelp();
			return(1);
			break;
		case '?':
			cerr << "ERROR: Unknown arguement: -" << (char) optopt << endl;
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
		cerr << "ERROR: Invalid syntax.  Must supply a job file name.\n";
		PrintSyntax();
		return(2);
	}

	if (VolumeLocatorFile.empty())
	{
		cerr << "ERROR: Invalid syntax.  Must supply a volume info file name.\n";
		PrintSyntax();
		return(2);
	}

	const map<string, VolumeInfo> AvailVolumes = LoadVolumeLocator(VolumeLocatorFile);

	if (AvailVolumes.empty())
	{
		cerr << "WARNING: Unable to find any volume information from file: " << VolumeLocatorFile << endl;
		cerr << "       : No job request made.\n";
		return(3);
	}

	if (SubcatalogueNames.empty())
	{
		cerr << "WARNING: No files given for input..." << endl;
		cerr << "       : No job request made.\n\n";
		PrintHelp();
		return(8);
	}

	JobClass JobRequest;
	
	for (vector<string>::const_iterator ACatalogueName = SubcatalogueNames.begin(); 
		 ACatalogueName != SubcatalogueNames.end();
		 ACatalogueName++)
	{
		if (!JobRequest.LoadCatalogueFile(*ACatalogueName))
		{
			cerr << "Problem with loading the catalogue file: " << *ACatalogueName << '\n';
		}
	}// end subcatalogue for-loop


	JobRequest.LoadVolumeInfos(AvailVolumes);

	JobRequest.OptimizeJob();		// checks across all the volumes and removes duplicate 
						// files and tries to minimize the number of volumes used
						// Also avoids damaged volumes, if possible and gets rid of missing volumes.



	cout << "Volume Count: " << JobRequest.GiveVolumeCount() << endl;
	cout << "FileName Count: " << JobRequest.GiveFileCount() << endl;

	if (JobRequest.WriteJobFile(JobFilename))
	{
		cout << "Successful writing of job request list: " << JobFilename << endl;
		return(0);
	}
	else
	{
		cout << "Unsucessful writing of job request list: " << JobFilename << endl;
		return(1);
	}
}
