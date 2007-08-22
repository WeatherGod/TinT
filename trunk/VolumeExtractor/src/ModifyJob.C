using namespace std;

#include <iostream>
#include <string>

#include <unistd.h>				// for optarg, opterr, optopt
#include <getopt.h>             // for getopt_long()


#include "JobClass.h"

void PrintSyntax()
{
	cerr << "ModifyJob (--jobfile | -J _JOBFILENAME_)\n"
	     << "          (--add | --remove _VOLUMENAME_)\n"
	     << "          [--source | -s _SOURCEJOB_]\n"
	     << "          [--optimize | -o]\n"
	     << "          [--help | -h] [--syntax | -x]" << endl;
}

void PrintHelp()
{
	PrintSyntax();
	
	cerr << endl;
	cerr << "Description: This program will perform an action regarding a given _VOLUMENAME_.\n\n";
	cerr << "   ACTIONS:\n"
	     << "         --add\n"
	     << "               Add the volume called _VOLUMENAME_ from the jobfile _SOURCEJOB_ to the\n"
	     << "               jobfile _JOBFILENAME_.  If the volume already exists in the jobfile, then any\n"
	     << "               new information from the source volume will be added to the original volume.\n"
	     << "         --remove\n"
	     << "               Remove the volume called _VOLUMENAME_ from the jobfile _JOBFILENAME_.\n"
	     << "               If _SOURCEJOB_ is specified, then remove the parts of _VOLUMENAME_ that is\n"
	     << "               found in _SOURCEJOB_.  If _SOURCEJOB_ is not named, then remove the\n"
	     << "               entire volume.\n\n";
	cerr << "   --optimize | -o\n"
	     << "        Do an optimization of the job after all other actions are taken\n";
	cerr << endl;
}

enum JobAction {NoAction, AddVol, RemoveVol};

int main(int argc, char *argv[])
{
	int OptionIndex = 0;
	int OptionChar = 0;
	opterr = 0;

	static struct option TheLongOptions[] = {
		{"jobfile", 1, NULL, 'J'},
		{"add", 1, NULL, 'a'},
		{"remove", 1, NULL, 'r'},
		{"optimize", 0, NULL, 'o'},
		{"source", 1, NULL, 's'},
		{"syntax", 0, NULL, 'x'},
		{"help", 0, NULL, 'h'},
		{0, 0, 0, 0}
	};

	string JobFilename = "";
	string SourceJobFilename = "";
	JobAction RequestedAction = NoAction;
	bool DoOptimize = false;
	string VolumeName = "";

	while ((OptionChar = getopt_long(argc, argv, "J:a:r:os:xh", TheLongOptions, &OptionIndex)) != -1)
	{
		switch (OptionChar)
		{
		case 'J':
			JobFilename = optarg;
			break;
		case 'a':
			if (RequestedAction != NoAction)
			{
				cerr << "ERROR: You can only specify one action!\n";
				return(2);
			}

			RequestedAction = AddVol;
			VolumeName = optarg;
			break;
		case 'r':
			if (RequestedAction != NoAction)
                        {
                                cerr << "ERROR: You can only specify one action!\n";
                                return(2);
                        }

			RequestedAction = RemoveVol;
			VolumeName = optarg;
			break;
		case 'o':
			DoOptimize = true;
			break;
		case 's':
			SourceJobFilename = optarg;
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

	if (RequestedAction == NoAction && !DoOptimize)
	{
		cerr << "WARNING: No action and optimization was requested!\n";
	}
		
	
	if (JobFilename.empty())
	{
		cerr << "ERROR: Incorrect syntax.  Must supply a jobfile for modification..." << endl;
		PrintSyntax();
		return(2);
	}



	if (SourceJobFilename.empty())
	{
		// SourceJobFilename is optional only when RequestedAction == NoAction || RequestedAction == RemoveVol

		if (RequestedAction == NoAction || RequestedAction == RemoveVol)
		{
			SourceJobFilename = JobFilename;
		}
		else
		{
			cerr << "ERROR: No source jobfile is given for your requested action!\n";
			PrintSyntax();
			return(7);
		}
	}


		
	JobClass JobToModify;

	if (!JobToModify.LoadJobFile(JobFilename))
	{
		cerr << "ERROR: Could not load jobfile: " << JobFilename << endl;
		return(3);
	}

	JobClass SourceJob;
	VolumeClass VolumeModification;

	if (RequestedAction != NoAction)
	{
		if (!SourceJob.LoadJobFile(SourceJobFilename))
		{
			cerr << "ERROR: Could not load the source jobfile: " << SourceJobFilename << endl;
			return(3);
		}

		if (!SourceJob.IsValid())
		{
			cerr << "ERROR: Invalid source jobfile! " << SourceJobFilename << endl;
			return(3);
		}

		if (!SourceJob.VolumeExist(VolumeName))
		{
			cerr << "ERROR: " << VolumeName << " was not found in the source jobfile." << endl;
			cerr << "Did you supply a source filename?\n";
			cerr << "No changes were made." << endl;
			return(4);
		}
	
		VolumeModification = SourceJob.GiveVolume(VolumeName);
	}
	

	switch (RequestedAction)
	{
	case AddVol:
		if (!JobToModify.AddVolume(VolumeName, VolumeModification))
		{
			cerr << "Could not add volume to the jobfile!" << endl;
			cerr << "No changes were made." << endl;
			return(5);
		}
		break;
	case RemoveVol:
		if (!JobToModify.RemoveVolume(VolumeName, VolumeModification))
		{
			cerr << "ERROR: Could not remove volume from the jobfile!" << endl;
			cerr << "No changes were made." << endl;
			return(5);
		}
		break;
	default:
		break;
	}

	if (DoOptimize)
	{
		JobToModify.OptimizeJob();
	}
	
	if (!JobToModify.WriteJobFile(JobFilename))
	{
		cerr << "ERROR: Could not write to the destination jobfile: " << JobFilename << endl;
		cerr << "None of the changes should have been saved, but please double-check." << endl;
		return(6);
	}
	
	return(0);
}
