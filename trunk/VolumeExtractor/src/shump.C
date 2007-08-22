using namespace std;

#include <iostream>
#include <string>
#include <vector>

#include <cstdio>			// for perror()
#include <sys/mtio.h>			// for ioctl()
#include <fcntl.h>			// for open(), close()

#include <csignal>              // for some signal handling.  I am very weak in
				// 	this area, but this should be sufficient.


// these three includes are for the stat() function
#include <sys/types.h>			// need the struct for stat()
#include <sys/stat.h>
#include <unistd.h>			// for access(), optarg, opterr, optopt
#include <getopt.h>             	// for getopt_long()

#include "VolumeInfo.h"			// for the VolumeInfo struct
#include "JobClass.h"
#include "StrUtly.h"			// for StrToInt(), Size_tToStr()

bool SIGNALCAUGHT = false;
void SignalHandle(int SigNum);		// Signal handler to set SIGNALCAUGHT to true



struct CmdOptions
{
	int VerboseLevel;
	string JobFilename;
	string DriveName;
	string VolumeName;
	bool DoAllVolumes;
	bool DoRetry;
	string FinishedJobFilename;
	string ProblemJobFilename;
	string DestinationDir;

	int StatusVal;		// zero means everything is ok
				// one means that a print of the help was requested
				// two means that there was a problem parsing the command line 
				// arguments or a request for syntax

	CmdOptions()
		:	VerboseLevel(0),
			JobFilename(""),
			DriveName(""),
			VolumeName(""),
			DoAllVolumes(false),
			DoRetry(false),
			FinishedJobFilename(""),
			ProblemJobFilename(""),
			DestinationDir("."),
			StatusVal(0)
	{
	}
};



void PrintSyntax()
{
	cerr << "shump [--jobfile | -J _JOBFILENAME_] [--dir | -D _DIRNAME_] [-V _VOLUMENAME_ | --all]\n"
	     << "      [--finjob | -f _FINISHED_JOBFILENAME_] [--probjob | -p _PROBLEM_JOBFILENAME_]\n"
	     << "      [--dest | -d _DESTINATIONDIR_] [--verbose | -v] [--retry | -r]\n"
	     << "      [--syntax | -x] [--help | -h]" << endl;
}

void PrintHelp()
{
	PrintSyntax();

	cerr << "Shump -- Super Helpful Universal Medium Processor\n";
	cerr << "\tThis program extracts files from volumes as indicated by the jobfile.  The arguements are" << endl;
	cerr << "\toptional, however, the exclusion of any of the first three options will cause" << endl;
	cerr << "\tthe program to prompt the user to supply that missing information." << endl;
	cerr << "\tUpon completion of a volume extraction, the finished jobfile and the problem jobfile" << endl;
	cerr << "\twill be updated." << endl;
	cerr << "\tThis program was designed to be 'thread-safe,' although I am not an expert on things\n"
	     << "\tlike that.  What you can do is have multiple instances of this" << endl;
	cerr << "\trunning simultaneously, even on the same jobfile." << endl;
	cerr << endl;
	cerr << endl;

	cerr << "\t--jobfile, -J " << endl;
	cerr << "\t\t_JOBFILENAME_ is the name of the file that has the job request information." << endl;
	cerr << endl;

	cerr << "\t--dir, -D" << endl;
	cerr << "\t\t_DIRNAME_ is the filename where the volume is accessible.  For tapes," << endl;
	cerr << "\t\tthis is the tape drive name.  For volumes on the fileserver, this is the" << endl;
	cerr << "\t\tdirectory in which the volumes are stored." << endl;
	cerr << "\t\tSpecify 'DEFAULT' for _DIRNAME_ if you want to use the\n"
	     << "\t\tdirectory name that is in the job file.\n";
	cerr << endl;

	cerr << "\t--volume, -V" << endl;
	cerr << "\t\t_VOLUMENAME_ is the name of the volume that you wish to extract files from," << endl;
	cerr << "\t\tin accordance with the jobfile.  If _VOLUMENAME_ is not in the jobfile, then" << endl;
	cerr << "\t\tnothing happens.  This arguement is mutually exclusive with the --all arguement." << endl;
	cerr << "\t\tMixing with --all is not allowed." << endl;
	cerr << endl;

	cerr << "\t--all, -a" << endl;
	cerr << "\t\tAttempt to do the entire job as indicated by the jobfile.  If _DRIVENAME_ is" << endl;
	cerr << "\t\tspecified, then the program assumes that all the volumes are at this location." << endl;
	cerr << "\t\tIf _DRIVENAME_ is not specified at the beginning, then the program will" << endl;
	cerr << "\t\tprompt the user for _DRIVENAME_ as each volume is extracted." << endl;
	cerr << "\t\tThis arguement is mutually exclusive with --volume.  Mixing is not allowed." << endl;
	cerr << endl;

	cerr << "\t--finjob, -f" << endl;
	cerr << "\t\t_FINISHED_JOBFILENAME_ is the name of the job file that contains what parts of" << endl;
	cerr << "\t\tthe job has been successfully completed.  By default, the name of this file is" << endl;
	cerr << "\t\tthe name of the original jobfile, with '_Finished' appened to it." << endl;
	cerr << endl;

	cerr << "\t--probjob, -p" << endl;
	cerr << "\t\t_PROBLEM_JOBFILENAME_ is the name of the job file that contains what parts of" << endl;
	cerr << "\t\tthe job could not be successfully completed.  By default, the name of this file is" << endl;
	cerr << "\t\tthe name of the original jobfile, with '_Problem' appened to it." << endl;
	cerr << endl;

	cerr << "\t--dest, -d" << endl;
	cerr << "\t\t_DESTINATIONDIR_ is the directory where you want the extracted files to be placed." << endl;
	cerr << "\t\tNote that any files extracted that contains a filepath will keep that filepath while" << endl;
	cerr << "\t\tbeing extracted to this directory." << endl;
	cerr << "\t\tBy default, the destination directory is the current working directory." << endl;
	cerr << endl;

	cerr << "\t--retry, -r\n";
	cerr << "\t\tWhen specified, shump will extract files, even if they appear\n";
	cerr << "\t\tin the problem job file.\n\n";

	cerr << "\t--verbose, -v" << endl;
	cerr << "\t\tUse multiple times on the command-line for more verbosity.\n";
	cerr << endl;

	cerr << "\t--help | -h" << endl;
	cerr << "\t\tPrints this help and exits" << endl;
	cerr << endl;
}

CmdOptions ParseCommandArgs(const int argc, char* argv[])
{
	CmdOptions OptionVals;
	
	int OptionIndex = 0;
	int OptionChar = 0;
	opterr = 0;

	
	static struct option TheLongOptions[] = {
		{"jobfile", 1, NULL, 'J'},
		{"drive", 1, NULL, 'D'},
		{"volume", 1, NULL, 'V'},
		{"finjob", 1, NULL, 'f'},
		{"probjob", 1, NULL, 'p'},
		{"dest", 1, NULL, 'd'},
		{"all", 0, NULL, 'a'},
		{"verbose", 0, NULL, 'v'},
		{"retry", 0, NULL, 'r'},
		{"syntax", 0, NULL, 'x'},
		{"help", 0, NULL, 'h'},
		{0, 0, 0, 0}
	};

	
	while ((OptionChar = getopt_long(argc, argv, "J:D:V:f:p:d:varxh", TheLongOptions, &OptionIndex)) != -1)
	{
		switch (OptionChar)
		{
		case 'J':
			OptionVals.JobFilename = optarg;
			break;
		case 'D':
			OptionVals.DriveName = optarg;
			break;
		case 'a':
			if (OptionVals.VolumeName.empty())
			{
				OptionVals.DoAllVolumes = true;
			}
			else
			{
				cerr << "ERROR: Incorrect syntax.  Cannot have both --volume and --all" << endl;
				OptionVals.StatusVal = 2;
			}
			break;
		case 'V':
			if (!OptionVals.DoAllVolumes)
			{
				OptionVals.VolumeName = optarg;
			}
			else
			{
				cerr << "ERROR: Incorrect syntax.  Cannot have both --volume and --all" << endl;
				OptionVals.StatusVal = 2;
			}
			break;
		case 'f':
			OptionVals.FinishedJobFilename = optarg;
			break;
		case 'p':
			OptionVals.ProblemJobFilename = optarg;
			break;
		case 'd':
			OptionVals.DestinationDir = optarg;
			break;
		case 'v':
			OptionVals.VerboseLevel++;
			break;
		case 'r':
			OptionVals.DoRetry = true;
			break;
		case 'x':
			OptionVals.StatusVal = 2;
			break;
		case 'h':
			OptionVals.StatusVal = 1;
			break;
		case '?':
			cerr << "ERROR: Unknown arguement: -" << (char) optopt << endl;
			OptionVals.StatusVal = 2;
			break;
		case ':':
			cerr << "ERROR: Missing value for arguement: -" << (char) optopt << endl;
			OptionVals.StatusVal = 2;
			break;
		default:
			cerr << "ERROR: Programming error... Unaccounted option: -" << (char) OptionChar << endl;
			OptionVals.StatusVal = 2;
			break;
		}
	}
	
	if (OptionVals.StatusVal == 1)
	{
		PrintHelp();
	}
	else if (OptionVals.StatusVal >= 2)
	{
		PrintSyntax();
	}
	

	return(OptionVals);
}




bool TestDriveName(const string &DriveName)
// Checks to see if the given directory is read-able or not.
{
	return(access(DriveName.c_str(), R_OK) == 0);
}



bool SuccessfulFileExtract(const string &FileName, const off_t ExpectedFileSize, const int VerboseLevel)
{
	struct stat FileStatus;

	if (VerboseLevel >= 3)
	{
		cout << "ExpectedFileSize: " << ExpectedFileSize << endl;
	}

	if (stat(FileName.c_str(), &FileStatus) == 0)
	{
		if (VerboseLevel >= 4)
		{
			cout << "Stat'ed filesize: " << FileStatus.st_size << endl;
		}

		return(FileStatus.st_size >= ExpectedFileSize);
	}
	else
	{
		if (VerboseLevel >= 3)
		{
			cout << "Could not stat " << FileName << endl;
		}

		return(false);
	}
}


VolumeInfo GetVolumeDetails(const JobClass &TheJob, const string &VolumeName)
{
	VolumeInfo TempInfo = TheJob.GiveVolume_Info(VolumeName);

	if (TempInfo.MediumType.empty())
	{
		TempInfo.MediumType = "DLT";
	}

	if (TempInfo.BlockSize == 0)
	{
		TempInfo.BlockSize = 1572864;
	}

	if (TempInfo.DirName.empty())
	{
		if (TempInfo.MediumType == "Disk")
		{
			TempInfo.DirName = ".";
		}
		else
		{
			TempInfo.DirName = "/dev/nst0";
		}
	}

	if (TempInfo.SysName.empty())
	{
		TempInfo.SysName = "127.0.0.1";
	}
	
	return(TempInfo);
}


void ExtractFromTapeVolume(const VolumeClass &RequestedVolume, const VolumeInfo &VolumeDetails,
			   JobClass &FinishedJobRequest, JobClass &ProblemJobRequest,
                           const string &DestDir,
			   const int VerboseLevel)
// There shouldn't be any issue with non-existant directories.  It appears that the tar command will create the
//	necessary directories as needed.  This isn't necessarrially the case with ExtractFromFileVolume()
{
	bool BadTapeIssues = false;

	for (FileGroupIter_Const AFileGroup = RequestedVolume.FileGroupBegin(); 
	     AFileGroup != RequestedVolume.FileGroupEnd() && !BadTapeIssues && !SIGNALCAUGHT; 
	     AFileGroup++)
	{
		const size_t FileGroupPosition = AFileGroup->first.GiveLocation();
		const int FileNum = open(VolumeDetails.DirName.c_str(), O_RDONLY);

	        if (FileNum < 0)
        	{
                	perror(VolumeDetails.DirName.c_str());
	                BadTapeIssues = true;
        	}

		
		struct mtget TapeGet;
		struct mtop TapeDo;

		if (ioctl(FileNum, MTIOCGET, &TapeGet) != 0)
                {
                        perror(VolumeDetails.DirName.c_str());
                        BadTapeIssues = true;
                }

                const size_t TapePosition = TapeGet.mt_fileno;


		if (VerboseLevel > 2)
                {
                        cout << "Going to Position: " << AFileGroup->first << endl;
                }

		// Move tape into position.
		if (TapePosition >= FileGroupPosition && !BadTapeIssues)
		{
			if (VerboseLevel > 3)
			{
				cout << "Moving tape back..." << endl;
			}

			if (FileGroupPosition == 0)
			{
				TapeDo.mt_op = MTREW;
				if (ioctl(FileNum, MTIOCTOP, &TapeDo) != 0)
                                {
                                        perror(VolumeDetails.DirName.c_str());
                                        BadTapeIssues = true;
				}
			}
			else
			{
				TapeDo.mt_op = MTBSFM;
				TapeDo.mt_count = TapePosition - FileGroupPosition + 1;
				if (ioctl(FileNum, MTIOCTOP, &TapeDo) != 0)
				{
					perror(VolumeDetails.DirName.c_str());
					BadTapeIssues = true;
				}
			}
		}
		else if (!BadTapeIssues)
		{
			if (VerboseLevel > 3)
			{
				cout << "Moving forward " << FileGroupPosition - TapePosition << " files..." << endl;
			}

			TapeDo.mt_op = MTFSF;
                        TapeDo.mt_count = FileGroupPosition - TapePosition;
                        if (ioctl(FileNum, MTIOCTOP, &TapeDo) != 0)
                        {
                                perror(VolumeDetails.DirName.c_str());
                                BadTapeIssues = true;
                        }
		}

		close(FileNum);



		if (BadTapeIssues)
		{
			cerr << "ERROR: The tape is not moving correctly to position: " << FileGroupPosition << endl;
                        
                        if (ProblemJobRequest.InsertFileGroup(VolumeDetails.VolumeName, AFileGroup->first, AFileGroup->second))
                        {
                        	cerr << "ERROR: could not add a problem filegroup to the ProblemJobRequest.  Filegroup: "
                                     << AFileGroup->first << '\n'
                                     << "Volume: " << VolumeDetails.VolumeName << '\n'
                                     << "This will have to be manually recorded..." << endl;
                        }

			continue;
		}

		if (SIGNALCAUGHT)
		{
			break;
		}

		// Do the tar extraction
		vector <FileType> TheFiles = RequestedVolume.GiveFiles(AFileGroup->first);
		vector <string> TheFileNames(TheFiles.size(), "");
		
		for (size_t FileIndex = 0; FileIndex < TheFiles.size(); FileIndex++)
		{
			TheFileNames[FileIndex] = TheFiles[FileIndex].GiveFileName();
		}

		

		const string SysCommand = "tar -b " + Size_tToStr(VolumeDetails.BlockSize / 512) + " -xvf '" 
				  + VolumeDetails.DirName + "' -C '" + DestDir + "' '"
				  + GiveDelimitedList(TheFileNames, "' '") + "'";

		int SysResult = system(SysCommand.c_str());
		if (SysResult != 0)
		{
			cerr << "ERROR: Seems to be an issue with the tape drive.  SysResult: " << SysResult << endl;
			BadTapeIssues = true;
		}

		for (size_t FileIndex = 0; FileIndex < TheFiles.size(); FileIndex++)
		{
			if (SuccessfulFileExtract( DestDir + '/' + TheFiles[FileIndex].GiveFileName(), 
						   TheFiles[FileIndex].GiveFileSize(), 
						   VerboseLevel ))
			{
				if (!FinishedJobRequest.InsertFile(VolumeDetails.VolumeName, 
								   AFileGroup->first, 
								   TheFiles[FileIndex]))
				{
					cerr << "ERROR: could not add an extracted file to the FinishedJob.  Filename: ";
					cerr << TheFiles[FileIndex].GiveFileName() << '\n';
					cerr << "Volume: " << VolumeDetails.VolumeName << "  FileGroupID: ";
					cerr << AFileGroup->first << '\n';
					cerr << "This will have to be manually recorded..." << endl;
				}

			}
			else
			{
				cerr << "ERROR: A File didn't extract: " << TheFiles[FileIndex].GiveFileName() << endl;
				BadTapeIssues = true;

				if (!ProblemJobRequest.InsertFile(VolumeDetails.VolumeName, 
								  AFileGroup->first, 
								  TheFiles[FileIndex]))
				{
					cerr << "ERROR: could not add a problem file to the ProblemJob.  Filename: ";
					cerr << TheFiles[FileIndex].GiveFileName() << '\n';
					cerr << "Volume: " << VolumeDetails.VolumeName << "  FileGroupID: ";
					cerr << AFileGroup->first << '\n';
					cerr << "This will have to be manually recorded..." << endl;
				}
			}
		}// end for each file in filegroup loop
	}// end for each filegroup in volume
}


void ExtractFromDiskVolume(const VolumeClass &RequestedVolume, const VolumeInfo &VolumeDetails,
			   JobClass &FinishedJobRequest, JobClass &ProblemJobRequest,
			   const string &DestDir,
			   const int VerboseLevel)
{
	for (FileGroupIter_Const AFileGroup = RequestedVolume.FileGroupBegin(); 
	     AFileGroup != RequestedVolume.FileGroupEnd() && !SIGNALCAUGHT;
	     AFileGroup++)
	{
		string SysCommand;
		string OldPath = "";

		for (FileIter_Const AFile = AFileGroup->second.FileBegin(); 
		     AFile != AFileGroup->second.FileEnd() && !SIGNALCAUGHT; 
		     AFile++)
		{
			const string FilePath = AFile->GiveFileName().substr(0, AFile->GiveFileName().rfind('/'));

			if (FilePath != OldPath)
			{
				// the above if statements prevents too many attempts to remake a directory that might already exist.
				SysCommand = "mkdir --parents '" + DestDir + '/' + FilePath + "'";

				if (system(SysCommand.c_str()) != 0)
				{
					cerr << "WARNING: Could not make the directory: " << DestDir + '/' + FilePath << endl;
					cerr << "         You probably won't be able to copy the file either!\n";
				}

				OldPath = FilePath;
			}

			SysCommand = "cp --reply=yes '" + VolumeDetails.DirName + "/File" + Size_tToStr(AFileGroup->first.GiveLocation()) + "/"
				     + AFile->GiveFileName() + "' '" + DestDir + '/' + AFile->GiveFileName() + "'";

			if (system(SysCommand.c_str()) == 0)
			{
				// Assume that if cp was successful, then all the bytes were copied

				if (!FinishedJobRequest.InsertFile(VolumeDetails.VolumeName, 
								   AFileGroup->first, 
								   *AFile))
				{
					cerr << "ERROR: could not add an extracted file to the FinishedJob.  Filename: ";
					cerr << AFile->GiveFileName() << endl;
					cerr << "Volume: " << VolumeDetails.VolumeName << "  FileGroupID: ";
					cerr << AFileGroup->first << endl;
					cerr << "This will have to be manually recorded..." << endl;
				}
			}
			else
			{
				if (!ProblemJobRequest.InsertFile(VolumeDetails.VolumeName, 
								  AFileGroup->first, 
								  *AFile))
				{
					cerr << "ERROR: could not add a problem file to the ProblemJob.  Filename: ";
					cerr << AFile->GiveFileName() << endl;
					cerr << "Volume: " << VolumeDetails.VolumeName << "  FileGroupID: ";
					cerr << AFileGroup->first << endl;
					cerr << "This will have to be manually recorded..." << endl;
				}
			}
		}
	}// end filegroup loop
}


void DoVolumeExtraction(const VolumeClass &VolumeRequest, const VolumeInfo &VolumeDetails, 
			JobClass &FinishedJobRequest, JobClass &ProblemJobRequest,
			const string DestinationDir, const int VerboseLevel)
{
	if (VolumeDetails.MediumType == "Disk")
	{
		ExtractFromDiskVolume( VolumeRequest, VolumeDetails, 
				       FinishedJobRequest, 
				       ProblemJobRequest, 
				       DestinationDir, VerboseLevel);
	}
	else if (VolumeDetails.MediumType == "DLT" || VolumeDetails.MediumType == "Ultrium")
	{
		ExtractFromTapeVolume( VolumeRequest, VolumeDetails,
				       FinishedJobRequest, 
				       ProblemJobRequest, 
				       DestinationDir, VerboseLevel);
	}
	else
	{
		cerr << "\n\t\tERROR: Unknown medium type: " << VolumeDetails.MediumType 
		     << " for volume: " << VolumeDetails.VolumeName << endl;
	}
}



JobClass LoadSpecialJobFile(const string &JobFilename)
{
	JobClass JobRequest;
	
	if (access(JobFilename.c_str(), F_OK) == 0)
	{
		if (!JobRequest.LoadJobFile(JobFilename))
		{
			throw("JobFile could not be loaded: " + JobFilename);
		}

		// don't worry about checking for .IsValid() because the request might be empty...
	}
	
	return(JobRequest);
}





string GetDriveName(string TempDriveName, const VolumeInfo &VolumeDetails)
{
	bool DriveExists = false;
	
	do
	{
		if (TempDriveName.empty())
		{
			cout << "Drive Name (QUIT to leave program safely): \n";
			cout << "VolumeName " << VolumeDetails.VolumeName << "\n";
			cout << "Defaullt Drive " << VolumeDetails.DirName << "\n --> ";
			cout.flush();
			getline(cin, TempDriveName);
		}
		
		if (TempDriveName == "DEFAULT" || TempDriveName.empty())
		{
			// Use the Default if DEFAULT is indicated, or user just presses enter.
			TempDriveName = VolumeDetails.DirName;
		}
		
		if (TempDriveName != "QUIT" && !(DriveExists = TestDriveName(TempDriveName)))
		{
			cerr << "ERROR: Could not read directory: " << TempDriveName << endl;
			TempDriveName = "";
		}
	} while (TempDriveName != "QUIT" && !DriveExists);
	
	return(TempDriveName);
}



string GetVolumeName(string TempVolumeName, const JobClass &JobRequest)
{
	bool VolumeExists = false;
	
	do
	{
		while (TempVolumeName.empty())
		{
			cout << "Volume Name (LIST to see list of available volumes or QUIT to leave program safely) --> ";
			cout.flush();
			getline(cin, TempVolumeName);
			
			if (TempVolumeName == "LIST")
			{
				for (VolumeIter_Const VolumeItem = JobRequest.VolumeBegin();
				     VolumeItem != JobRequest.VolumeEnd(); VolumeItem++)
				{
					cout << VolumeItem->first.GiveName() << '\n';
				}
				cout << endl;
			
				TempVolumeName = "";
			}
		}
		
		if (TempVolumeName != "QUIT" && !(VolumeExists = JobRequest.VolumeExist(TempVolumeName)))
		{
			cerr << "ERROR: Can't find volume " << TempVolumeName << " in the Job Request!\n"
			     << "Maybe it doesn't need to be done?" << endl;
			TempVolumeName = "";
		}
	} while (TempVolumeName != "QUIT" && !VolumeExists); 
	
	return(TempVolumeName);
}



//------------------------------------------- Main Program --------------------------------------------------------

int main(int argc, char *argv[])
{	
	CmdOptions OptionVals = ParseCommandArgs(argc, argv);

	if (OptionVals.StatusVal != 0)
	{
		// early exit from program because either a problem 
		// with parsing or help was requested
		return(OptionVals.StatusVal);
	}

	while (OptionVals.JobFilename.empty())
	{
		cout << "Job Filename --> ";
		cout.flush();
		getline(cin, OptionVals.JobFilename);
	}

	JobClass JobRequest;

	if (OptionVals.VerboseLevel > 3)
	{
		cout << "Trying to load Jobfile: " << OptionVals.JobFilename << endl;
	}

	if (!JobRequest.LoadJobFile(OptionVals.JobFilename))
	{
		cerr << "ERROR: Jobfile could not be loaded: " << OptionVals.JobFilename << endl;
		return(3);
	}

	if (OptionVals.VerboseLevel > 3)
	{
		cout << OptionVals.JobFilename << " was successfully loaded!" << endl;
	}	


	if (!JobRequest.IsValid())
	{
		cerr << "ERROR: Invalid jobfile!" << endl;
		return(3);
	}


	if (OptionVals.VerboseLevel >= 4)
	{
		cout << "Generating the finished job filename and the problem job filename" << endl;
	}

	if (OptionVals.FinishedJobFilename.empty())
	{
		OptionVals.FinishedJobFilename = OptionVals.JobFilename + "_Finished";
	}

	if (OptionVals.ProblemJobFilename.empty())
	{
		OptionVals.ProblemJobFilename = OptionVals.JobFilename + "_Problem";
	}

	JobClass FinishedJobRequest, ProblemJobRequest;

	try
	{
		FinishedJobRequest = LoadSpecialJobFile(OptionVals.FinishedJobFilename);
		ProblemJobRequest = LoadSpecialJobFile(OptionVals.ProblemJobFilename);
	}
	catch (const string &ErrStr)
	{
		cerr << "ERROR: Exception caught. " << ErrStr << endl;
		return(6);
	}
	catch (const exception &Err)
	{
		cerr << "ERROR: Exception caught. " << Err.what() << endl;
		return(6);
	}
	catch (...)
	{
		cerr << "ERROR: Unknown exception..." << endl;
		return(6);
	}

	JobRequest -= FinishedJobRequest;	// This removes anything that exists in both JobClasses from JobRequest.
						// JobRequest will not have any information on things that have already been tried.
						// Note that any other program that is actively doing extractions will update this
						// FinishedJobRequest file, and this process's JobRequest will never know it until
						// FinishedJobRequest is updated and this command is repeated.

	if (!OptionVals.DoRetry)
	{
		JobRequest -= ProblemJobRequest;	// Only want to avoid problem files if we are not doing a Retry.
	}
	
	
	
	if (!TestDriveName(OptionVals.DestinationDir))
	{
		cerr << "ERROR: Could not read the destination directory: " << OptionVals.DestinationDir << endl;
		return(7);
	}
	
	
	vector <string> VolumeNames_ToDo(0);

	if (OptionVals.DoAllVolumes)
	{
		for (VolumeIter_Const VolumeItem = JobRequest.VolumeBegin();
		     VolumeItem != JobRequest.VolumeEnd();
		     VolumeItem++)
		{
			VolumeNames_ToDo.push_back(VolumeItem->first.GiveName());
		}
	}
	else
	{
		const string TempVolName = GetVolumeName(OptionVals.VolumeName, JobRequest);
		if (TempVolName == "QUIT")
		{
			VolumeNames_ToDo.clear();
		}
		else
		{
			VolumeNames_ToDo.push_back(TempVolName);
		}
	}
	
	signal(SIGINT, SignalHandle);		// Let the SignalHandle() function change the
						// value of SIGNALCAUGHT to true to kick
						// the extraction functions out safely...
	
	for (vector<string>::const_iterator AVolName = VolumeNames_ToDo.begin(); 
	     AVolName != VolumeNames_ToDo.end() && !SIGNALCAUGHT; 
	     AVolName++)
	{
		VolumeInfo VolumeDetails = GetVolumeDetails(JobRequest, *AVolName);
	        VolumeDetails.DirName = GetDriveName(OptionVals.DriveName, VolumeDetails);
	
		if (VolumeDetails.DirName == "QUIT")
		{
			// break out of the for-loop....
			break;
		}
		
		
		if (OptionVals.VerboseLevel >= 3)
		{
			cout << "Doing volume in the request: " << *AVolName << endl;
		}

		
		DoVolumeExtraction(JobRequest.GiveVolume(*AVolName), VolumeDetails, 
				   FinishedJobRequest, ProblemJobRequest,
				   OptionVals.DestinationDir, OptionVals.VerboseLevel);


		// loads the file information, thus updating itself
	        //    then it writes the class back to the file
        	//    this employs file locking mechanisms to force other instances
                //    of this function (called by other programs, for example) to
	        //    wait for the file to be finished being used.
	        FinishedJobRequest.UpdateAndSaveJob(OptionVals.FinishedJobFilename);
        	ProblemJobRequest.UpdateAndSaveJob(OptionVals.ProblemJobFilename);


		
		JobRequest -= FinishedJobRequest;
		if (!OptionVals.DoRetry)
		{
			JobRequest -= ProblemJobRequest;
		}
	}

	return(0);
}



void SignalHandle(int SigNum)
{
	signal(SigNum, SIG_IGN);
	cerr << "------ Signal Caught -------\n";
	SIGNALCAUGHT = true;
}
