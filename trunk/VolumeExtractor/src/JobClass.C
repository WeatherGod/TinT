#ifndef _JOBCLASS_C
#define _JOBCLASS_C
using namespace std;

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>

#include <unistd.h>			// for access()
#include "FlockUtly.h"			// for FileLockControl(), TruncateFile()
#include <fcntl.h>			// for F_UNLCK, F_WRLCK, F_RDLCK

#include "JobClass.h"
#include "ConfigUtly.h"			// for ReadNoComments(), FoundStartTag(), FoundEndTag(), StripTags()
#include "TimeUtly.h"			// for GetTimeUTC()
#include "StrUtly.h"			// for TakeDelimitedList(), GiveDelimitedList(), StrToSize_t(), StrToOff_t()
#include "VectorUtly.h"			// for Merge()

#include "VolumeClass.h"		// for VolContainer, VolumeIter_Const and VolumeIter are typedef'ed in VolumeClass.h
#include "FileGroup.h"
#include "FileType.h"
#include "VolID_t.h"
#include "FileGroupID_t.h"
#include "FileID_t.h"



JobClass::JobClass()
	:	myVolumes(),
		myIsConfigured(false),
		myDummyVolume()
{
}

JobClass::JobClass(const JobClass &AJob)
	:	myVolumes(AJob.myVolumes),
		myIsConfigured(AJob.myIsConfigured),
		myDummyVolume(AJob.myDummyVolume)
{
}


JobClass& JobClass::operator = (const JobClass &AJob)
{
	myVolumes = AJob.myVolumes;
	myIsConfigured = AJob.myIsConfigured;
	myDummyVolume = AJob.myDummyVolume;

	return(*this);
}





bool JobClass::LoadJobFile(fstream &JobStream)
{
	const vector <string> TheTagWords = InitTagWords();

        string FileLine = "";

        FileLine = ReadNoComments(JobStream);

        while (JobStream.good() && !FoundStartTag(FileLine, TheTagWords[0]))     // Job
        {
                FileLine = ReadNoComments(JobStream);
        }

        FileLine = ReadNoComments(JobStream);
        bool BadObject = false;

        while (JobStream.good() && !FoundEndTag(FileLine, TheTagWords[0]))       // Job
        {
                if (!BadObject)
                {
                        if (FoundStartTag(FileLine, TheTagWords[1]))     // Volume
                        {
                                FileLine = ReadNoComments(JobStream);
                                VolumeClass TempVolume;
                                VolID_t VolumeID = TempVolume.ReadJobInfo(FileLine, JobStream);

				if (!TempVolume.ValidConfig())
				{
					cerr << "Problem in JobClass object... Bad Volume..." << endl;
					BadObject = true;
				}
				else
				{
					AddVolume(VolumeID, TempVolume);
				}
                        }
                        else
                        {
                                cerr << "Unknown tags in JobClass object...   Here is the line: " << FileLine << endl;
                                BadObject = true;
			}
                }

                FileLine = ReadNoComments(JobStream);
        }

        if (JobStream.eof())
        {
                cerr << "Early end reading job file." << endl;
        }

	myIsConfigured = !BadObject;
	
	return(myIsConfigured);
}



bool JobClass::LoadJobFile(const string &JobFilename)
{
        fstream JobStream(JobFilename.c_str(), ios::in);     // open the file for reading.

        if (!JobStream.is_open())
        {
	        cerr << "ERROR: Could not open the job file: " << JobFilename << endl;
                return(false);
        }

	if (FileLockControl(JobStream, F_RDLCK) != 0)
        {
        	// need more error controling here.  I need to check errno to see if the error indicates that it is ok to continue.
                // right now, I am just taking a paranoid approach for the development.
                cerr << "Problem with locking: " << JobFilename << endl;
                JobStream.close();
                return(false);
	}

        bool LoadingResult = LoadJobFile(JobStream);

       	FileLockControl(JobStream, F_UNLCK);
        JobStream.close();
        return(LoadingResult);
}// end LoadJobFile()


bool JobClass::LoadCatalogueFile(const string &CatalogueFilename)
{
	fstream CataInput(CatalogueFilename.c_str(), ios::in);

	if (!CataInput.is_open())
	{
		cerr << "Error opening catalogue file: " << CatalogueFilename << endl;
                return(false);
	}

	if (FileLockControl(CataInput, F_RDLCK) != 0)
	{
		cerr << "ERROR: unable to set read lock on catalogue file: " << CatalogueFilename << endl;
		CataInput.close();
		return(false);
	}
	
	string LineRead = "";
	getline(CataInput, LineRead);

	while (CataInput.good())
	{
		vector <string> TempHold = TakeDelimitedList(LineRead, ',');
		if (TempHold.size() < 6)
		{
			cerr << "ERROR: Error reading the line from the catalogue: " << CatalogueFilename << endl;
                        cerr << "Bad line: " << LineRead << endl;
			myIsConfigured = false;
                        FileLockControl(CataInput, F_UNLCK);
                        CataInput.close();
                        return(false);
		}

		// Element 0 has filename
		// Element 1 has filegroup number
		// Element 2 has volumename
		// Element 3 has filesize
		// Element 4 has filing index
		// Element 5 has filetime
			
		if (!InsertFile((VolID_t) TempHold[2],
				(FileGroupID_t) StrToSize_t(TempHold[1]),
				FileType(TempHold[0], (off_t) StrToOff_t(TempHold[3]), GetTimeUTC(TempHold[5]))))
		{
			cerr << "ERROR: Problem inserting file into the job from the catalogue!" << endl;
			cerr << "The catalogue: " << CatalogueFilename << endl;
                        cerr << "Bad line: " << LineRead << endl;
			myIsConfigured = false;
       	                FileLockControl(CataInput, F_UNLCK);
               	        CataInput.close();
			return(false);
		}

		getline(CataInput, LineRead);
	}

	FileLockControl(CataInput, F_UNLCK);
	myIsConfigured = CataInput.eof();
	CataInput.close();

	return(myIsConfigured);
}

void JobClass::LoadVolumeInfos(const map<string, VolumeInfo> &AvailVolumes)
// This adds information to the volumes you already have.  Therefore,
// this function must be called AFTER loading up the Job object with volumes.
// Any volumes loaded into this Job object after calling this function will not
// gain any benefit of this information.
{
	for (VolumeIter AVol = myVolumes.begin(); AVol != myVolumes.end(); AVol++)
	{
		map<string, VolumeInfo>::const_iterator AMatch = AvailVolumes.find(AVol->first.GiveName());

		if (AMatch != AvailVolumes.end())
		{
			AVol->second.TakeMediumType(AMatch->second.MediumType);
			AVol->second.TakeBlockSize(AMatch->second.BlockSize);
			AVol->second.TakeDirName(AMatch->second.DirName);
			AVol->second.TakeVolStatus(AMatch->second.VolStatus);
		}
	}
}




bool JobClass::IsValid() const
// need a better definition.
// Possibly a loop checking the validity of all the volumes within it?
{
	return(!myVolumes.empty());
}

bool JobClass::ValidConfig() const
{
	return(myIsConfigured);
}


VolumeClass JobClass::GiveVolume(const VolID_t &VolumeID) const
// Eventually, failure to match will throw an exception.
{
	VolumeIter_Const VolMatch = VolumeID.Find(myVolumes);
	if (VolMatch == myVolumes.end())
        {
                return(myDummyVolume);
        }

	return(VolMatch->second);
}

vector <VolumeClass> JobClass::GiveVolumes() const
{
        vector <VolumeClass> TheVolumes( myVolumes.size() );
	vector<VolumeClass>::iterator AVol = TheVolumes.begin();

	for (VolumeIter_Const SourceVol = myVolumes.begin(); SourceVol != myVolumes.end(); AVol++, SourceVol++)
	{
		*AVol = SourceVol->second;
	}

        return(TheVolumes);
}

string JobClass::GiveVolume_MediumType(const VolID_t &VolumeID) const
{
	VolumeIter_Const VolMatch = VolumeID.Find(myVolumes);
        if (VolMatch == myVolumes.end())
        {
                return(myDummyVolume.GiveMediumType());
        }

        return(VolMatch->second.GiveMediumType());
}

size_t JobClass::GiveVolume_BlockSize(const VolID_t &VolumeID) const
{
        VolumeIter_Const VolMatch = VolumeID.Find(myVolumes);
        if (VolMatch == myVolumes.end())
        {
                return(myDummyVolume.GiveBlockSize());
        }

        return(VolMatch->second.GiveBlockSize());
}

string JobClass::GiveVolume_DirName(const VolID_t &VolumeID) const
{
        VolumeIter_Const VolMatch = VolumeID.Find(myVolumes);
        if (VolMatch == myVolumes.end())
        {
                return(myDummyVolume.GiveDirName());
        }

        return(VolMatch->second.GiveDirName());
}


VolumeInfo JobClass::GiveVolume_Info(const VolID_t &VolumeID) const
{
	VolumeIter_Const VolMatch = VolumeID.Find(myVolumes);
        if (VolMatch == myVolumes.end())
        {
		VolumeInfo Dummy;
                return(Dummy);
        }

	VolumeInfo TempInfo;
	TempInfo.VolumeName = VolumeID.GiveName();
	TempInfo.DirName = VolMatch->second.GiveDirName();
	TempInfo.MediumType = VolMatch->second.GiveMediumType();
	TempInfo.BlockSize = VolMatch->second.GiveBlockSize();

	return(TempInfo);
}




VolumeIter_Const JobClass::VolumeBegin() const
{
	return(myVolumes.begin());
}

VolumeIter_Const JobClass::VolumeEnd() const
{
	return(myVolumes.end());
}

bool JobClass::VolumeExist(const VolID_t &VolumeID) const
{
	VolumeIter_Const VolMatch = VolumeID.Find(myVolumes);
        return(VolMatch != myVolumes.end());
}

size_t JobClass::GiveVolumeCount() const
{
        return(myVolumes.size());
}

vector <string> JobClass::GiveVolumeNames() const
// Soon, match failure will throw an exception.
{
        vector <string> VolumeNames(myVolumes.size());
        vector<string>::iterator AName = VolumeNames.begin();

        for (VolumeIter_Const AVol = myVolumes.begin(); AVol != myVolumes.end(); AVol++, AName++)
        {
                *AName = AVol->first.GiveName();
        }

        return(VolumeNames);
}




/***********************************************************************************
 *                       FileGroup Access functions                                *
 ***********************************************************************************/
vector <size_t> JobClass::GiveFileGroupLocations(const VolID_t &VolumeID) const
// Soon, match failure will throw an exception.
{
	VolumeIter_Const VolMatch = VolumeID.Find(myVolumes);
        if (VolMatch == myVolumes.end())
        {
                return(vector<size_t>(0));
        }
	else
	{
		return(VolMatch->second.GiveFileGroupLocations());
	}
}

vector <FileGroup> JobClass::GiveFileGroups(const VolID_t &VolumeID) const
// Soon, match failure will throw an exception.
{
        VolumeIter_Const VolMatch = VolumeID.Find(myVolumes);
        if (VolMatch == myVolumes.end())
        {
                return(vector<FileGroup>(0));
        }
	else
	{
        	return(VolMatch->second.GiveFileGroups());
	}
}

FileGroup JobClass::GiveFileGroup(const VolID_t &VolumeID, const FileGroupID_t &FileGroupID) const
// Soon, match failure will throw an exception.
{
        VolumeIter_Const VolMatch = VolumeID.Find(myVolumes);
        if (VolMatch == myVolumes.end())
        {
                return(myDummyVolume.GiveFileGroup(string::npos));
        }
	else
	{
        	return(VolMatch->second.GiveFileGroup(FileGroupID));
	}
}

size_t JobClass::GiveFileGroupCount() const
{
        size_t FileGroupTotal = 0;

        for (VolumeIter_Const AVol = myVolumes.begin(); AVol != myVolumes.end(); AVol++)
        {
                FileGroupTotal += AVol->second.GiveFileGroupCount();
        }

        return(FileGroupTotal);
}

size_t JobClass::GiveFileGroupCount(const VolID_t &VolumeID) const
// Soon, match failure will throw an exception.
{
        VolumeIter_Const VolMatch = VolumeID.Find(myVolumes);
        if (VolMatch == myVolumes.end())
        {
                return(string::npos);
        }
	else
	{
	        return(VolMatch->second.GiveFileGroupCount());
	}
}




/***********************************************************************************
 *                       FileType Access functions                                 *
 ***********************************************************************************/
vector <FileType> JobClass::GiveFiles(const VolID_t &VolumeID, const FileGroupID_t &FileGroupID) const
// Soon, match failure will throw an exception.
{
	VolumeIter_Const VolMatch = VolumeID.Find(myVolumes);
        if (VolMatch == myVolumes.end())
        {
                return(vector<FileType>(0));
        }
	else
	{
		return(VolMatch->second.GiveFiles(FileGroupID));
	}
}

FileType JobClass::GiveFile(const VolID_t &VolumeID, const FileGroupID_t &FileGroupID, const FileID_t &FileID) const
// Soon, match failure will throw an exception.
{
	VolumeIter_Const VolMatch = VolumeID.Find(myVolumes);
        if (VolMatch == myVolumes.end())
        {
                return(FileType(""));
        }
	else
	{
	        return(VolMatch->second.GiveFile(FileGroupID, FileID));
	}
}



vector <time_t> JobClass::GiveFileTimes(const VolID_t &VolumeID, const FileGroupID_t &FileGroupID) const
// Soon, match failure will throw an exception.
{
        VolumeIter_Const VolMatch = VolumeID.Find(myVolumes);
        if (VolMatch == myVolumes.end())
        {
                return(vector<time_t>(0));
        }
	else
	{
        	return(VolMatch->second.GiveFileTimes(FileGroupID));
	}
}

time_t JobClass::GiveFileTime(const VolID_t &VolumeID, const FileGroupID_t &FileGroupID, const FileID_t &FileID) const
// Soon, match failure will throw an exception.
{
        VolumeIter_Const VolMatch = VolumeID.Find(myVolumes);
        if (VolMatch == myVolumes.end())
        {
                return((time_t) -1);
        }
	else
	{
        	return(VolMatch->second.GiveFileTime(FileGroupID, FileID));
	}
}


vector <off_t> JobClass::GiveFileSizes(const VolID_t &VolumeID, const FileGroupID_t &FileGroupID) const
// Soon, match failure will throw an exception.
{
        VolumeIter_Const VolMatch = VolumeID.Find(myVolumes);
        if (VolMatch == myVolumes.end())
        {
                return(vector<off_t>(0));
        }
	else
	{
        	return(VolMatch->second.GiveFileSizes(FileGroupID));
	}
}

off_t JobClass::GiveFileSize(const VolID_t &VolumeID, const FileGroupID_t &FileGroupID, const FileID_t &FileID) const
// Soon, match failure will throw an exception.
{
        VolumeIter_Const VolMatch = VolumeID.Find(myVolumes);
        if (VolMatch == myVolumes.end())
        {
                return((off_t) -1);
        }
	else
	{
        	return(VolMatch->second.GiveFileSize(FileGroupID, FileID));
	}
}


vector <string> JobClass::GiveFileNames(const VolID_t &VolumeID, const FileGroupID_t &FileGroupID) const
// Soon, match failure will throw an exception.
{
        VolumeIter_Const VolMatch = VolumeID.Find(myVolumes);
        if (VolMatch == myVolumes.end())
        {
                return(vector<string>(0));
        }
	else
	{
	        return(VolMatch->second.GiveFileNames(FileGroupID));
	}
}


size_t JobClass::GiveFileCount() const
{
	size_t FileTotal = 0;

	for (VolumeIter_Const AVol = myVolumes.begin(); AVol != myVolumes.end(); AVol++)
	{
		FileTotal += AVol->second.GiveFileCount();
	}

	return(FileTotal);
}

size_t JobClass::GiveFileCount(const VolID_t &VolumeID) const
// Soon, match failure will throw an exception.
{
	VolumeIter_Const VolMatch = VolumeID.Find(myVolumes);
        if (VolMatch == myVolumes.end())
	{
		return(string::npos);
	}
	else
	{
		return(VolMatch->second.GiveFileCount());
	}
}

size_t JobClass::GiveFileCount(const VolID_t &VolumeID, const FileGroupID_t &FileGroupID) const
// Soon, match failure will throw an exception.
{
        VolumeIter_Const VolMatch = VolumeID.Find(myVolumes);
        if (VolMatch == myVolumes.end())
        {
                return(string::npos);
        }
	else
	{
        	return(VolMatch->second.GiveFileCount(FileGroupID));
	}
}


JobClass JobClass::FindFiles(const FileID_t &FileID) const
// need to throw an exception if the AddVolume fails.
{
        JobClass SubJob;

        for (VolumeIter_Const AVol = myVolumes.begin(); AVol != myVolumes.end(); AVol++)
        {
                const VolumeClass VolumeResult = AVol->second.FindFiles(FileID);
                SubJob.AddVolume(AVol->first, VolumeResult);
        }

        return(SubJob);
}

JobClass JobClass::FindFiles(const vector <FileID_t> &FileIDs) const
// need to throw an exception if the AddVolume fails.
{
        JobClass SubJob;

        for (VolumeIter_Const AVol = myVolumes.begin(); AVol != myVolumes.end(); AVol++)
        {
                const VolumeClass VolumeResult = AVol->second.FindFiles(FileIDs);
                SubJob.AddVolume(AVol->first, VolumeResult);
        }

        return(SubJob);
}





/***********************************************************************************
 *                       JobClass Modifier functions                               *
 ***********************************************************************************/

// ----------------------- Volumes -----------------------------------
bool JobClass::RemoveVolume(const VolID_t &VolumeID)
// Soon, match failure will throw an exception.
{
	VolumeIter VolMatch = VolumeID.Find(myVolumes);
        if (VolMatch == myVolumes.end())
	{
		return(false);
	}
	else
	{
		myVolumes.erase(VolMatch);
		return(true);
	}
}

bool JobClass::RemoveVolume(const VolID_t &VolumeID, const VolumeClass &RemovalVol)
{
	if (!RemovalVol.IsValid())
	{
		return(false);
	}
	
	VolumeIter VolMatch = VolumeID.Find(myVolumes);
       	if (VolMatch != myVolumes.end())
        {
       	        VolMatch->second -= RemovalVol;
        }

	return(true);
}


bool JobClass::AddVolume(const VolID_t &VolumeID, const VolumeClass &NewVolume)
// Soon, match failure will throw an exception.
{
	if (!NewVolume.IsValid())
	{
		return(false);
	}

	VolumeIter VolMatch = VolumeID.Find(myVolumes);
	if (VolMatch == myVolumes.end())
	{
		myVolumes.insert(make_pair(VolumeID, NewVolume));
	}
	else
	{
		// This Volume has the same VolumeID as one that is already in this class.  We will attempt to add its information into the
		// current Volume in this class.  All functions are designed to only add in new information, so no duplicate Volumes are formed,
		// no duplicate filegroups within a Volume are formed, no duplicate files within a filegroup are formed.
		VolMatch->second += NewVolume;
	}

	return(true);
}

bool JobClass::ReplaceVolume(const VolID_t &VolumeID, const VolumeClass &ReplacementVolume)
{
	if (!ReplacementVolume.IsValid())
	{
		return(false);
	}

	vector <FileID_t> TheFileIDs(0);

	for (FileGroupIter_Const AFileGroup = ReplacementVolume.FileGroupBegin();
	     AFileGroup != ReplacementVolume.FileGroupEnd();
	     AFileGroup++)
	{
		const vector <string> TempNames = AFileGroup->second.GiveFileNames();
		TheFileIDs.insert(TheFileIDs.end(), TempNames.begin(), TempNames.end());
	}

	const JobClass TempJob = FindFiles(TheFileIDs);

	*this -= TempJob;

	return(AddVolume(VolumeID, ReplacementVolume));
}


//------------------------------ FileGroup -----------------------------------------
bool JobClass::RemoveFileGroup(const VolID_t &VolumeID, const FileGroupID_t &FileGroupID)
// Soon, match failure will throw an exception.
{
	VolumeIter VolMatch = VolumeID.Find(myVolumes);
        if (VolMatch == myVolumes.end())
	{
		return(false);
	}
	else
	{
		return(VolMatch->second.RemoveFileGroup(FileGroupID));
	}
}


bool JobClass::RemoveFileGroup(const VolID_t &VolumeID, const FileGroupID_t &FileGroupID, const FileGroup &RemovalGroup)
// Soon, match failure will throw an exception.
{
        VolumeIter VolMatch = VolumeID.Find(myVolumes);
        if (VolMatch == myVolumes.end())
        {
        	return(false);
        }
	else
	{
		return(VolMatch->second.RemoveFileGroup(FileGroupID, RemovalGroup));
	}
}



bool JobClass::AddFileGroup(const VolID_t &VolumeID, const FileGroupID_t &FileGroupID, const FileGroup &NewFileGroup)
// AddFileGroup will add the new filegroup to the Volume indicated.  If a match
// fails for the Volume, this will fail with no changes, possibly (need to check).
// Soon, match failure will throw an exception.
{
	VolumeIter VolMatch = VolumeID.Find(myVolumes);
        if (VolMatch == myVolumes.end())
	{
		return(false);
	}
	else
	{
		return(VolMatch->second.AddFileGroup(FileGroupID, NewFileGroup));
	}
}


bool JobClass::InsertFileGroup(const VolID_t &VolumeID, const FileGroupID_t &FileGroupID, const FileGroup &NewFileGroup)
// InsertFileGroup behaves like AddFileGroup except that in the case where the Volume does
// not match, a new volume will be created.
// Soon, match failure will throw an exception, which will need to be caught.
{
        VolumeIter VolMatch = VolumeID.Find(myVolumes);
        if (VolMatch == myVolumes.end())
        {
		VolumeClass TempVolume;
		TempVolume.AddFileGroup(FileGroupID, NewFileGroup);

		return(AddVolume(VolumeID, TempVolume));
        }
	else
	{
		return(VolMatch->second.AddFileGroup(FileGroupID, NewFileGroup));
	}
}


//------------------------------ FileType -----------------------------------------
bool JobClass::RemoveFile(const VolID_t &VolumeID, const FileGroupID_t &FileGroupID, const FileID_t &FileID)
// Soon, match failure will throw an exception.
{
	VolumeIter VolMatch = VolumeID.Find(myVolumes);
        if (VolMatch == myVolumes.end())
        {
                return(false);
        }
	else
	{
		return(VolMatch->second.RemoveFile(FileGroupID, FileID));
	}
}

bool JobClass::RemoveFiles(const VolID_t &VolumeID, const FileGroupID_t &FileGroupID, const vector <FileID_t> &FileIDs)
/* Note, the true or false that is returned by this function only refers to the validity of the VolumeID and the FileGroupID,
	NOT the validity of the removal of the files.
   In other words, files will be removed from the Job if they match those indicated by the IDs.
   Only those files that are the intersection of the set of files in the Job and the set of files ID'ed by the FileIDs are removed.
*/
// Soon, match failure will throw an exception.
{
	VolumeIter VolMatch = VolumeID.Find(myVolumes);
        if (VolMatch == myVolumes.end())
        {
                return(false);
        }
	else
	{
		return(VolMatch->second.RemoveFiles(FileGroupID, FileIDs));
	}
}

bool JobClass::AddFile(const VolID_t &VolumeID, const FileGroupID_t &FileGroupID, const FileType &NewFile)
// AddFile will add the file into the matching Volume and FileGroup.  
// Will fail with no changes if the Volume and/or the FileGroup does not exist.
// Soon, match failure will throw an exception.
{
	VolumeIter VolMatch = VolumeID.Find(myVolumes);
        if (VolMatch == myVolumes.end())
	{
		return(false);
	}
	else
	{
		return(VolMatch->second.AddFile(FileGroupID, NewFile));
	}
}

bool JobClass::AddFiles(const VolID_t &VolumeID, const FileGroupID_t &FileGroupID, const vector <FileType> &NewFiles)
// AddFiles will add the file into the matching Volume and FileGroup.  Will fail if the
// Volume and/or the FileGroup does not exist.  Also, if any of the files is not Valid,
// then the function will return false without any changes.
// Soon, match failure will throw an exception.
{
	VolumeIter VolMatch = VolumeID.Find(myVolumes);
        if (VolMatch == myVolumes.end())
        {
                return(false);
        }
	else
	{
		return(VolMatch->second.AddFiles(FileGroupID, NewFiles));
	}
}

bool JobClass::InsertFile(const VolID_t &VolumeID, const FileGroupID_t &FileGroupID, const FileType &NewFile)
// InsertFile will behaves like AddFile except that if the Volume isn't found or the FileGroup isn't found,
// then they will be created as well.  False is returned if NewFile is not Valid, or some other failure occured
// in the process of inserting the file.  No changes are made if false is returned.
// True for success.
// Soon, match failure will throw an exception, which will need to be caught.
{
	VolumeIter VolMatch = VolumeID.Find(myVolumes);
        if (VolMatch == myVolumes.end())
        {
		VolumeClass TempVolume;
		TempVolume.InsertFile(FileGroupID, NewFile);

		return(AddVolume(VolumeID, TempVolume));
	}
	else
	{
		return(VolMatch->second.InsertFile(FileGroupID, NewFile));
	}
}





/************************************************************************
 *                 Maintainence and Administrative			*
 ************************************************************************/
bool JobClass::WriteJobFile(fstream &JobStream) const
{
	const vector <string> TheTagWords = InitTagWords();
	JobStream << "<" << TheTagWords[0] << ">\n";

        for (VolumeIter_Const AVol = myVolumes.begin(); AVol != myVolumes.end(); AVol++)
        {
        	AVol->second.WriteJobList(AVol->first, JobStream);
        }

        JobStream << "</" << TheTagWords[0] << ">\n";
	JobStream.flush();		// makes sure everything has been written
	return(JobStream.good());
}



bool JobClass::WriteJobFile(const string &JobFilename) const
{
	fstream JobStream(JobFilename.c_str(), ios::out);
	
	if (!JobStream.is_open())
	{
		cerr << "ERROR: Could not open the Job file: " << JobFilename << endl;
		return(false);
	}

	if (FileLockControl(JobStream, F_WRLCK) != 0)
	{
		cerr << "ERROR: Problem locking the file: " << JobFilename << endl;
		JobStream.close();
		return(false);
	}

	if (TruncateFile(JobStream) != 0)
        {
                // Need more error control.  Right now, I am in paranoid mode.
                cerr << "ERROR: Problem with resetting the file: " << JobFilename << endl;
                FileLockControl(JobStream, F_UNLCK);
                JobStream.close();
	        return(false);
        }

        JobStream.seekp(0);

	bool GoodWrite = WriteJobFile(JobStream);
	
	FileLockControl(JobStream, F_UNLCK);
	JobStream.close();

	return(GoodWrite);
}

bool JobClass::UpdateAndSaveJob(const string &JobFilename)
// There is a BIG DIFFERENCE between the member functions WriteJobFile() and UpdateAndSaveJob()
// WriteJobFile just takes the current information in this JobClass and puts it to file, blindly.
// UpdateAndSaveJob will read the file specified, incorporate its information into the current class,
// and then write the class's information back out to the same file, minding any file locks.
{
	if (access(JobFilename.c_str(), R_OK | W_OK) == 0)
	{
		// The JobFile does exist, so lock it up, read it in and write it out.
		fstream JobStream(JobFilename.c_str(), ios::in | ios::out);	// open the file for both reading and writing.

		if (!JobStream.is_open())
		{
			cerr << "ERROR: Could not open the job file: " << JobFilename << endl;
			return(false);
		}
		

		if (FileLockControl(JobStream, F_WRLCK) != 0)		// setting a write lock. Will wait if someone else has a read or write lock already
		{
			// need more error controling here.  I need to check errno to see if the error indicates that it is ok to continue.
			// right now, I am just taking a paranoid approach for the development.
			cerr << "ERROR: Problem with locking: " << JobFilename << endl;
			JobStream.close();
			return(false);
		}


		if (!LoadJobFile(JobStream))
		{
			cerr << "ERROR: Problem with loading the JobFile: " << JobFilename << endl;
			FileLockControl(JobStream, F_UNLCK);
			JobStream.close();
			return(false);
		}

		// At this point, the JobFile has been loaded and this JobClass has been updated with its information.
		// Now, I need to reset the file, truncate it to zero-length, and then write out this class's information back out.
		// Note that I am still locked up.  Any process using JobClass to handle the same JobFile is still waiting back at
		// their fcntl call.

		JobStream.clear();		// clears any EOF flags and such, allowing for further stuff to be done on the stream.
		
		if (TruncateFile(JobStream) != 0)
		{
			// Need more error control.  Right now, I am in paranoid mode.
			cerr << "ERROR: Problem with resetting the file: " << JobFilename << endl;
			FileLockControl(JobStream, F_UNLCK);
			JobStream.close();
                        return(false);
		}

		JobStream.seekp(0);				// brings the filewrite "cursor" back to the beginning.

		// Now, I am ready to write!

		if (!WriteJobFile(JobStream))
		{
			cerr << "Could not write to the jobfile: " << JobFilename << endl;
			FileLockControl(JobStream, F_UNLCK);
                        JobStream.close();
                        return(false);
		}

		FileLockControl(JobStream, F_UNLCK);
                JobStream.close();
                
		return(true);
	}
	else
	{
		// the jobfile does not exist already, so there is no information from which we can update from!
		// so, just lock up the file and write to it.

		if (!WriteJobFile(JobFilename))
		{
			cerr << "ERROR: Could not write to the jobfile: " << JobFilename << endl;
                        return(false);
                }
		else
		{
                	return(true);
		}
	}
}

vector <string> JobClass::InitTagWords() const
{
	vector <string> TheTagWords(0);

	TheTagWords.push_back("Job");
	TheTagWords.push_back("Volume");

	return(TheTagWords);
}



//----------------------------------------------------------------------------------------------

void JobClass::OptimizeJob()
// This function optimizes the job by removing redundent files
// only a first-order optimization.
// Additional code can be implemented to figure out how to minimize the number of volumes and maximize the number of files per volume

// Also, this code checks the status of the volumes and removes any volumes that are deemed "Missing" before optimizing.
// During the optimizing process, a Volume that is deemed "Damaged" will lose any preference.

// DEFINITION OF DUPLICATE FILE:
//	A duplicate file is one where it has the same name and time as another file.  The file size is NOT
//	used to determine the identity of two files.  The rational behind this is:
//	  1.  You cannot have two different files with the same name in the same place.
//	  	Therefore, it is already inherent in the file system that there will
//		not be two identically named files in the same directory or tar archive.
//	      Therefore, the FileGroup class must take steps to ensure similar behavior.
//		If the FileGroup class receives duplicate files during the loading of the
//		FileType objects, it will keep the FileType object that has a larger value
//		for its member variable "myFileSize".
//	      This still leads open the possibility that files in different FileGroup objects
//		or even different Volume objects can be duplicates of each other.  
//		THIS IS ACCEPTABLE.  It makes sense.  We can easily have a copy of a 
//		file in another directory, there is no conflict of definition here.
//	  2.  For the purposes of this JobClass, two files with the same name should mean
//		that they are both files conceptually containing the same data.  For example,
//		a file named "npwmo.020530" contained in Volume A contains npwmo data for
//		May 30, 2002.  A file named "npwmo.020530" contained in Volume B should
//		also contain (conceptually) the same data.  It should not contain Radar data,
//		for example.
//	      However, the two files may have different sizes.  This can happen if there was
//		corruption on the tape or some other issues.  The files have, contextually,
//		the same data, but one has physically more data than the other.
//
//	So, for optimization of the JobRequest, the lesser of a pair of duplicate files
//	must be removed.  The boolean comparisons for the FileType class has been designed
//	so that only comparisons by the name of the file is done.
//	Therefore, when a duplicate file is identified, steps then need to be taken to
//	directly compare the filesizes of the two files.
{
	for (VolumeIter VolumeItem = myVolumes.begin(); VolumeItem != myVolumes.end(); )
	{
		// Check to see if the Volume is deemed to be "Destroyed", in which case, don't bother with
		// this volume.  Remove it outright without with prejudice.
		if (VolumeItem->second.GiveVolStatus() == Destroyed_Vol)
		{
			VolumeIter TempIter = VolumeItem;		// Avoid invalidating my iterator during looping process.
			VolumeItem++;
			myVolumes.erase(TempIter);
			continue;
		}


		// first works to minimize the number of files within each Volume
		//  i.e. - removes duplicate files within the same volume
		//  Sometimes duplicate files appear because there was
		//     a failure writing the first one to the volume, so a re-attempt was made.
		VolumeItem->second.OptimizeJob();
		VolumeItem++;
	}

	// At this point, each Volume in the Job should be individually minimized.
	// No duplicate files exist within any one Volume, so Merging of the files
	// within a Volume into a single vector is fine (i.e. - will produce a unique set with no duplicates).
	
	// Now, I want to remove duplicate files that exists across the volumes.
	// To accomplish this, I will first identify sets of COMPLETELY IDENTICAL
	// files between any two Volumes.  Completely Identical means that the filename,
	// and the file size are both equal.

	// If a set of files in a Volume Completely Identifies with a subset of another Volume's files,
	// then the Volume is redundant and is not needed.  If the two Volumes Completely Identifies with
	// each other, then the one that has fewer FileGroups will be kept.  If the number of
	// FileGroups is the same, then the one that has a lower VolumeName will remain in the job.

	for (VolumeIter Volume_A = myVolumes.begin(); Volume_A != myVolumes.end(); )
	{
		bool DeletedVol_A = false;

		vector <FileType> AVolume_Files(0);
		for (FileGroupIter_Const AGroup = Volume_A->second.FileGroupBegin();
		     AGroup != Volume_A->second.FileGroupEnd(); AGroup++)
		{
			const vector <FileType> TempHold = AGroup->second.GiveFiles();
			AVolume_Files.insert(AVolume_Files.end(), TempHold.begin(), TempHold.end());
		}
		sort(AVolume_Files.begin(), AVolume_Files.end());

		VolumeIter Volume_B = Volume_A;
		Volume_B++;
		for (; Volume_B != myVolumes.end(); )
		{
			// Volume1 is checked against Volume2, Volume3, Volume4
			// Volume2 is checked against Volume3, Volume4
			// Volume3 is checked against Volume4
			// Volume4 will have Volume_B iterator set to myVolumes.end() and this inner loop will not execute.

			vector <FileType> BVolume_Files(0);
			for (FileGroupIter_Const AGroup = Volume_B->second.FileGroupBegin();
	                     AGroup != Volume_B->second.FileGroupEnd(); AGroup++)
			{
				const vector <FileType> TempHold = AGroup->second.GiveFiles();
	                        BVolume_Files.insert(BVolume_Files.end(), TempHold.begin(), TempHold.end());
                	}
                	sort(BVolume_Files.begin(), BVolume_Files.end());

			// includes() will return true if all elements of BVolume_Files also exist in AVolume_Files.
			// Comparison is done differently, using both the filename and the file size.  Normally, the
			// function uses the default "lessthan" comparison, which would be using just the filename.
			// Completely is a special struct that has a binary function defined for its comparisons.
			// So, CompletelyLessThan means that it will do a lessthan comparison using both filename and file size.

			// Note that within each Volume, there are no duplicate files, so the vectors satisfies the
			// sort using the more strict comparison function as well.  If there were duplicate files
			// within the volume, this would not necessarrially be true and this function would not work
			// as expected.
			if (includes(AVolume_Files.begin(), AVolume_Files.end(), 
				     BVolume_Files.begin(), BVolume_Files.end(), CompletelyLessThan() ))
			{
				// All the files in Volume_B also exists in Volume_A,
				// So, Volume_B might be unnecessary, but only if
				// Volume_A is undamaged or both are damaged.
					
				if (AVolume_Files.size() == BVolume_Files.size())
				{
					// the two volumes have the same files.
					
					// If one of them is "Damaged", then just get rid of that one.
					// Otherwise, if neither or both are "Damaged", then just figure
					// out which is more 'efficient', and keep it.

					if ( (Volume_A->second.GiveVolStatus() != Damaged_Vol
                                             || Volume_B->second.GiveVolStatus() == Damaged_Vol)
					    && IsMoreEfficient(Volume_A->second, Volume_B->second))
					{
						// Assigning and incrementing the Volume_B iterator before erasure to
						// prevent iterator invalidation.  If this isn't done,
						// undefined behavior occurs at the iteration after the erase.
						// This isn't usually an issue with just one iterator on the map, but arises with two.
						VolumeIter TempIter = Volume_B;
						Volume_B++;
						myVolumes.erase(TempIter);
					}
					else
					{
						// Because Volume_A is being removed, I will break out of the inner for loop.
						// just as above, I will assign and increment the Volume_A iterator to prevent
						// iterator invalidation when erase occurs.
						DeletedVol_A = true;
						VolumeIter TempIter = Volume_A;
						Volume_A++;
						myVolumes.erase(TempIter);
						break;
					}
				}
				else
				{
					if (Volume_A->second.GiveVolStatus() != Damaged_Vol
					    || Volume_B->second.GiveVolStatus() == Damaged_Vol)
					{
						// since the file counts are different, and all the files in Volume_B exists in Volume_A,
						// then we don't need Volume_B, unless Volume_A was damaged while Volume_B isn't

						// Note the above logic.  This if statement is true if Volume_A is not damaged at all
						// or, if both Volume_A and Volume_B are damaged.

						VolumeIter TempIter = Volume_B;
						Volume_B++;
						myVolumes.erase(TempIter);
					}
				}
			}
			else if (includes(BVolume_Files.begin(), BVolume_Files.end(),
                        	          AVolume_Files.begin(), AVolume_Files.end(), CompletelyLessThan() ))
			{
				// All the files in Volume_A exists in Volume_B ....
				// And since we already did the test above, we know that the
				// set of files in Volume_B is NOT the same as the set of file in Volume_A.
				// So, the set of files in Volume_A is completely redundant, so remove Volume_A
				// ...unless Volume_B is damaged and Volume_A is not.
                                
				if (Volume_B->second.GiveVolStatus() != Damaged_Vol
				    || Volume_A->second.GiveVolStatus() == Damaged_Vol)
				{
					DeletedVol_A = true;
					VolumeIter TempIter = Volume_A;
					Volume_A++;
					myVolumes.erase(TempIter);
                        	        break;			// break out of inner for loop
				}
                	}
			else
			{
				// No need to delete either, just move on to the next Volume.
				Volume_B++;
			}
		}// ends inner for-loop

		if (!DeletedVol_A)
		{
			// advancing this iterator only if the A volume was never deleted.
			// if it was deleted, then it iterator has already been advanced.
			Volume_A++;
		}
	}// ends outer for-loop

	CleanUp();
			
	// At this point, all redundent Volumes have been removed.
	// Unless there were damage issues.

	// Next, we deal with the issue of redundent files across the Volumes.  Lets say that Volume1 has files A B and C in a FileGroup.
	// And we have Volume2 with files B C and D in 2 FileGroups.  The last process would have kept both Volumes in the Job.  Files will
	// be removed such that, first, the larger of the two (filesize-wise) will be kept, then, if they are the same, the file will be removed
	// such that a minimum number of FileGroups are used to obtain files A B C and D.

	// However, any Volumes that are damaged loses priority.

	// Upon completion, there should be no duplicate files within the Job, with (approximately) the minimum number of Volumes used and
	// the minimum number of FileGroups used...


	for (VolumeIter AVolume = myVolumes.begin(); AVolume != myVolumes.end(); AVolume++)
	{
		vector <FileType> AVolume_Files(0);
		for (FileGroupIter_Const AGroup = AVolume->second.FileGroupBegin();
                     AGroup != AVolume->second.FileGroupEnd(); AGroup++)
                {
                        const vector <FileType> TempHold = AGroup->second.GiveFiles();
                        AVolume_Files.insert(AVolume_Files.end(), TempHold.begin(), TempHold.end());
                }
                sort(AVolume_Files.begin(), AVolume_Files.end());


		// Compare AVolume_Files with files in Volumes Ahead of it.
		// So, for a Job with 5 Volumes:
		//      Volume1 gets checked against Volume2, Volume3, Volume4, Volume5
		//      Volume2 gets checked against Volume3, Volume4, Volume5
		//      Volume3 gets checked against Volume4, Volume5
		//      Volume4 gets checked against Volume5
		
		VolumeIter BVolume = AVolume;
		BVolume++;
		for (; BVolume != myVolumes.end(); BVolume++)
		{
			vector <FileType> BVolume_Files(0);
	                for (FileGroupIter_Const AGroup = BVolume->second.FileGroupBegin();
	                     AGroup != BVolume->second.FileGroupEnd(); AGroup++)
        	        {
                	        const vector <FileType> TempHold = AGroup->second.GiveFiles();
	                        BVolume_Files.insert(BVolume_Files.end(), TempHold.begin(), TempHold.end());
                	}
                	sort(BVolume_Files.begin(), BVolume_Files.end());


			
			// Note that the intersection of two vectors can never be larger than the smallest of the two vectors.
			vector <FileType> VolumeIntersect(  min(AVolume_Files.size(), BVolume_Files.size())  );		// vector initialized to size
															// of smallest vector of Files
			vector <FileType>::iterator EndSpot;

			// set_intersection will find files that have equivalent FileIDs and filetimes.  It does NOT compare the file sizes!!!
			EndSpot = set_intersection(AVolume_Files.begin(), AVolume_Files.end(), 
						   BVolume_Files.begin(), BVolume_Files.end(), VolumeIntersect.begin());
			
			if (EndSpot != VolumeIntersect.begin())
			{
				VolumeIntersect.resize(EndSpot - VolumeIntersect.begin());
				const vector <FileID_t> FileIDs = GenerateIDs(VolumeIntersect);
				const VolumeClass TempVolumeA = AVolume->second.FindFiles(FileIDs);
				const VolumeClass TempVolumeB = BVolume->second.FindFiles(FileIDs);

				if ( (AVolume->second.GiveVolStatus() != Damaged_Vol
				      || BVolume->second.GiveVolStatus() == Damaged_Vol)
				     && IsMoreEfficient(TempVolumeA, TempVolumeB))
				{
					BVolume->second -= TempVolumeB;
				}
				else
				{
					AVolume->second -= TempVolumeA;

					// Need to update AVolume_Files because there has been a change to myVolume[AVolumeIndex]
                                        //                                                      a.k.a. - AVolume
					AVolume_Files.clear();
					for (FileGroupIter_Const AGroup = AVolume->second.FileGroupBegin();
			                     AGroup != AVolume->second.FileGroupEnd(); AGroup++)
                			{
                        			const vector <FileType> TempHold = AGroup->second.GiveFiles();
			                        AVolume_Files.insert(AVolume_Files.end(), TempHold.begin(), TempHold.end());
                			}
			                sort(AVolume_Files.begin(), AVolume_Files.end());

				} // end of if IsMoreEfficient()
			}// end of if the size of VolumeIntersect > 0
		}// end of BVolume loop
	}// end of AVolume loop

	CleanUp();

	// By now, the Job *Should* be optimized (Rough, uncalculated estimate is that this Job will be within 80-90% of the theoretical optimum)!
	// Also, any Destroyed volumes should have been completely removed and any Damaged volumes should have been avoided.

}

void JobClass::CleanUp()
{
	for (VolumeIter AVol = myVolumes.begin(); AVol != myVolumes.end(); )
	{
		AVol->second.CleanUp();
		if (AVol->second.GiveFileCount() == 0)
		{
			VolumeIter TempIter = AVol;
			AVol++;
			myVolumes.erase(TempIter);
		}
		else
		{
			AVol++;
		}
	}
}


/*----------------------------------------------------------------------------------------*
 *      Overloaded Math operators                                                         *
 *----------------------------------------------------------------------------------------*/
JobClass& operator -= (JobClass &OrigJob, const JobClass &JobToRemove)
{
	for (VolumeIter_Const AVol = JobToRemove.VolumeBegin(); AVol != JobToRemove.VolumeEnd(); AVol++)
	{
		OrigJob.RemoveVolume(AVol->first, AVol->second);
	}

	OrigJob.CleanUp();

	return(OrigJob);
}

JobClass& operator += (JobClass &OrigJob, const JobClass &JobToAdd)
{
        for (VolumeIter_Const AVol = JobToAdd.VolumeBegin(); AVol != JobToAdd.VolumeEnd(); AVol++)
        {
                OrigJob.AddVolume(AVol->first, AVol->second);
        }

	return(OrigJob);
}

JobClass& operator - (JobClass JobA, const JobClass &JobB)
{
	return(JobA -= JobB);
}

JobClass& operator + (JobClass JobA, const JobClass &JobB)
{
        return(JobA += JobB);
}
//---------------------------------------------------------------------------------------


#endif
