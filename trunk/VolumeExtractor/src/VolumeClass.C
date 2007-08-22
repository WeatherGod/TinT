#ifndef _VOLUMECLASS_C
#define _VOLUMECLASS_C
using namespace std;

#include <iostream>
#include <fstream>

#include <vector>
#include <map>
#include <string>

#include "VolumeClass.h"

#include "StrUtly.h"			// for RipWhiteSpace()
#include "ConfigUtly.h"			// for ReadNoComments(), StripTags(), FoundStartTag(), FoundEndTag()
#include "VectorUtly.h"			// for Merge(), FindList(), Unique(), SubsetList()
#include "FileGroup.h"			// for FileGroupContainer, FileGroupIter_Const and FileGroupIter are typedef'ed in FileGroup.h
#include "FileType.h"			// for FileContainer, FileIter_Const and FileIter are typedef'ed in FileType.h
#include "FileGroupID_t.h"
#include "FileID_t.h"
#include "VolumeInfo.h"			// for struct VolumeInfo

/*--------------------------------------------------------------------------------------*
 *				VolumeClass						*
 *--------------------------------------------------------------------------------------*/

//	***** Constructors *****
VolumeClass::VolumeClass()
	:	myVolumeInfo(),
		myFileGroups(),
		myIsConfigured(false),
		myDummyFileGroup()
{
}

VolumeClass::VolumeClass(const VolumeClass &AVol)
	:	myVolumeInfo(AVol.myVolumeInfo),
		myFileGroups(AVol.myFileGroups),
		myIsConfigured(AVol.myIsConfigured),
		myDummyFileGroup(AVol.myDummyFileGroup)
{
}

VolumeClass& VolumeClass::operator = (const VolumeClass &AVol)
{
	myVolumeInfo = AVol.myVolumeInfo;
	myFileGroups = AVol.myFileGroups;
        myIsConfigured = AVol.myIsConfigured;
	return(*this);
}


//	***** Loaders *****
VolID_t VolumeClass::ReadJobInfo(string &FileLine, fstream &ReadData)
{
	const vector <string> TheTagWords = InitTagWords();

	bool BadObject = false;
	string VolumeName = "";

	while (!FoundEndTag(FileLine, TheTagWords[0]) && !ReadData.eof())
	{
		if (!BadObject)
		{
			if (FoundStartTag(FileLine, TheTagWords[1]))	// VolumeName
			{
				VolumeName = RipWhiteSpace(StripTags(FileLine, TheTagWords[1]));
			}
			else if (FoundStartTag(FileLine, TheTagWords[2]))	// FileGroup
			{
				FileLine = ReadNoComments(ReadData);
				FileGroup TempGroup;
				FileGroupID_t GroupID = TempGroup.ReadJobInfo(FileLine, ReadData);

				if (!TempGroup.ValidConfig())
				{
					cerr << "Problem in VolumeClass object.  Bad FileGroup...\n";
					BadObject = true;
				}
				else
				{
					AddFileGroup(GroupID, TempGroup);
				}
			}
			else if (FoundStartTag(FileLine, TheTagWords[3]))	// MediumType
			{
				TakeMediumType(RipWhiteSpace(StripTags(FileLine, TheTagWords[3])));
			}
			else if (FoundStartTag(FileLine, TheTagWords[4]))	// BlockSize
			{
				TakeBlockSize(StrToSize_t(RipWhiteSpace(StripTags(FileLine, TheTagWords[4]))));
			}
			else if (FoundStartTag(FileLine, TheTagWords[5]))	// DirName
			{
				TakeDirName(RipWhiteSpace(StripTags(FileLine, TheTagWords[5])));
			}
			else if (FoundStartTag(FileLine, TheTagWords[6]))       // Status
                        {
                                TakeVolStatus(RipWhiteSpace(StripTags(FileLine, TheTagWords[6])));
                        }
			else if (FoundStartTag(FileLine, TheTagWords[7]))	// SysName
			{
				TakeSysName(RipWhiteSpace(StripTags(FileLine, TheTagWords[7])));
			}
			else
			{
				BadObject = true;
				cerr << "\nProblem in VolumeClass object... Here is the line: " << FileLine << endl;
			}
		}

		FileLine = ReadNoComments(ReadData);
	}// end while loop

	if (!ReadData.eof() && !VolumeName.empty() && !BadObject)
	{
		myIsConfigured = true;
	}

	return(VolumeName);
}


//	***** Status Checkers *****
bool VolumeClass::ValidConfig() const
{
        return(myIsConfigured);
}

bool VolumeClass::IsValid() const
// Need a better definition of validity.
{
        return(!myFileGroups.empty());
}

//	***** Member Value Providers *****

string VolumeClass::GiveMediumType() const
{
	return(myVolumeInfo.MediumType);
}

size_t VolumeClass::GiveBlockSize() const
{
	return(myVolumeInfo.BlockSize);
}

string VolumeClass::GiveDirName() const
{
	return(myVolumeInfo.DirName);
}

string VolumeClass::GiveSysName() const
{
	return(myVolumeInfo.SysName);
}

VolumeStatus_t VolumeClass::GiveVolStatus() const
{
	return(myVolumeInfo.VolStatus);
}

string VolumeClass::GiveStatusStr() const
{
	return(myVolumeInfo.GiveStatusStr());
}

VolumeInfo VolumeClass::GiveVolumeInfo() const
{
	return(myVolumeInfo);
}


void VolumeClass::TakeMediumType(const string &MediumType)
{
        myVolumeInfo.MediumType = MediumType;
}

void VolumeClass::TakeBlockSize(const size_t &BlockSize)
{
        myVolumeInfo.BlockSize = BlockSize;
}

void VolumeClass::TakeDirName(const string &DirName)
{
        myVolumeInfo.DirName = DirName;
}

void VolumeClass::TakeSysName(const string &SysName)
{
	myVolumeInfo.SysName = SysName;
}

void VolumeClass::TakeVolStatus(const string &StatusStr)
{
	myVolumeInfo.TakeStatusStr(StatusStr);
}

void VolumeClass::TakeVolStatus(const VolumeStatus_t &StatusVal)
{
	myVolumeInfo.VolStatus = StatusVal;
}

void VolumeClass::TakeVolumeInfo(const VolumeInfo &TheInfo)
{
	myVolumeInfo = TheInfo;
}








vector <FileGroup> VolumeClass::GiveFileGroups() const
{
	vector <FileGroup> TheFileGroups( myFileGroups.size() );
	vector <FileGroup>::iterator AGroup = TheFileGroups.begin();
	for (FileGroupIter_Const SourceGroup = myFileGroups.begin(); SourceGroup != myFileGroups.end(); SourceGroup++, AGroup++)
	{
		*AGroup = SourceGroup->second;
	}

	return(TheFileGroups);
}

FileGroup VolumeClass::GiveFileGroup(const FileGroupID_t &FileGroupID) const
{
	FileGroupIter_Const GroupMatch = FileGroupID.Find(myFileGroups);
        if (GroupMatch == myFileGroups.end())
        {
                return(myDummyFileGroup);
        }

        return(GroupMatch->second);
}

FileGroupIter_Const VolumeClass::FileGroupBegin() const
{
	return(myFileGroups.begin());
}

FileGroupIter_Const VolumeClass::FileGroupEnd() const
{
	return(myFileGroups.end());
}

vector <size_t> VolumeClass::GiveFileGroupLocations() const
{
	vector <size_t> TheLocations( myFileGroups.size() );
	vector<size_t>::iterator ALocation = TheLocations.begin();

	for (FileGroupIter_Const AGroup = myFileGroups.begin(); AGroup != myFileGroups.end(); AGroup++, ALocation++)
	{
		*ALocation = AGroup->first.GiveLocation();
	}

	return(TheLocations);
}



vector <FileType> VolumeClass::GiveFiles(const FileGroupID_t &FileGroupID) const
{
	FileGroupIter_Const GroupMatch = FileGroupID.Find(myFileGroups);
        if (GroupMatch == myFileGroups.end())
        {
                return(myDummyFileGroup.GiveFiles());
        }
	else
	{
		return(GroupMatch->second.GiveFiles());
	}
}

FileType VolumeClass::GiveFile(const FileGroupID_t &FileGroupID, const FileID_t &FileID) const
{
	FileGroupIter_Const GroupMatch = FileGroupID.Find(myFileGroups);
        if (GroupMatch == myFileGroups.end())
        {
                return(myDummyFileGroup.GiveFile(FileID));
        }
	else
	{
	        return(GroupMatch->second.GiveFile(FileID));
	}
}



vector <string> VolumeClass::GiveFileNames(const FileGroupID_t &FileGroupID) const
{
	FileGroupIter_Const GroupMatch = FileGroupID.Find(myFileGroups);
        if (GroupMatch == myFileGroups.end())
	{
		return(myDummyFileGroup.GiveFileNames());
	}
	else
	{
		return(GroupMatch->second.GiveFileNames());
	}
}


vector <off_t> VolumeClass::GiveFileSizes(const FileGroupID_t &FileGroupID) const
{
        FileGroupIter_Const GroupMatch = FileGroupID.Find(myFileGroups);
        if (GroupMatch == myFileGroups.end())
        {
                return(myDummyFileGroup.GiveFileSizes());
        }
	else
	{
	        return(GroupMatch->second.GiveFileSizes());
	}
}

off_t VolumeClass::GiveFileSize(const FileGroupID_t &FileGroupID, const FileID_t &FileID) const
{
        FileGroupIter_Const GroupMatch = FileGroupID.Find(myFileGroups);
        if (GroupMatch == myFileGroups.end())
        {
                return(myDummyFileGroup.GiveFileSize(FileID));
        }
	else
	{
        	return(GroupMatch->second.GiveFileSize(FileID));
	}
}



vector <time_t> VolumeClass::GiveFileTimes(const FileGroupID_t &FileGroupID) const
{
        FileGroupIter_Const GroupMatch = FileGroupID.Find(myFileGroups);
        if (GroupMatch == myFileGroups.end())
        {
                return(myDummyFileGroup.GiveFileTimes());
        }
	else
	{
        	return(GroupMatch->second.GiveFileTimes());
	}
}

time_t VolumeClass::GiveFileTime(const FileGroupID_t &FileGroupID, const FileID_t &FileID) const
{
        FileGroupIter_Const GroupMatch = FileGroupID.Find(myFileGroups);
        if (GroupMatch == myFileGroups.end())
        {
                return(myDummyFileGroup.GiveFileTime(FileID));
        }
	else
	{
        	return(GroupMatch->second.GiveFileTime(FileID));
	}
}


bool VolumeClass::RemoveFileGroup(const FileGroupID_t &FileGroupID)
// Removes the entire FileGroup identified by FileGroupID from the Volume
// Eventually, the failure to match may throw an exception.
{
	FileGroupIter GroupMatch = FileGroupID.Find(myFileGroups);
        if (GroupMatch == myFileGroups.end())
	{
		return(false);
	}
	else
	{
		myFileGroups.erase(GroupMatch);
		return(true);
	}
}



bool VolumeClass::RemoveFileGroup(const FileGroupID_t &FileGroupID, const FileGroup &RemovalGroup)
/* Removes the intersection between the set of files in the Volume's FileGroup, identified by FileGroupID,
   and the set of files in the RemovalGroup FileGroup.
   True is returned if the RemovalGroup is a valid FileGroup and if FileGroupID matches a filegroup
   within the Volume.  False is returned for the failure of either.
*/
/* Eventually, the failure to match may throw an exception.*/
{
	if (RemovalGroup.IsValid())
	{
		FileGroupIter GroupMatch = FileGroupID.Find(myFileGroups);
	        if (GroupMatch == myFileGroups.end())
		{
			return(false);
		}
		
		GroupMatch->second -= RemovalGroup;
		return(true);
	}
	else
	{
		return(false);
	}
}

bool VolumeClass::AddFileGroup(const FileGroupID_t &FileGroupID, const FileGroup &NewFileGroup)
/* Adds FileGroup information from NewFileGroup that does not exists in the Volume's FileGroup, identified by FileGroupID,
   and updates any information that both shares.
   True is returned if the NewFileGroup is a valid FileGroup.  False is returned for an Invalid NewFileGroup.
*/
/* Eventually, the failure to match may throw an exception.*/
{
	if (NewFileGroup.IsValid())
	{
		FileGroupIter GroupMatch = FileGroupID.Find(myFileGroups);
		if (GroupMatch == myFileGroups.end())
		{
			myFileGroups.insert(make_pair(FileGroupID, NewFileGroup));
		}
		else
		{
			// NewFileGroup's ID is the same as the Location of one of this Volume's FileGroups.
			// We will now try to update this particular FileGroup with NewFileGroup's information.  No duplicate information will
			// be created by this process.
			GroupMatch->second += NewFileGroup;
		}

		return(true);
	}
	else
	{
		return(false);
	}
}


bool VolumeClass::RemoveFile(const FileGroupID_t &FileGroupID, const FileID_t &FileID)
/*	Completely removes the file from the Volume's FileGroup identified by FileGroupID,
	that matches the given FileID.
	Use this if you only need to remove one file.  If you have a list of files to
	to remove from the same filegroup, use the RemoveFiles() function since
	it will only search for the FileGroup once.
	True is returned when FileGroupID matches and when FileID matches.
	False is returned for the failure of either of these and no changes made.
*/
// Eventually, the failure to match will throw an exception, instead.
{
        FileGroupIter GroupMatch = FileGroupID.Find(myFileGroups);
        if (GroupMatch == myFileGroups.end())
        {
                return(false);
        }
	else
	{
		return(GroupMatch->second.RemoveFile(FileID));
	}
}


bool VolumeClass::RemoveFiles(const FileGroupID_t &FileGroupID, const vector <FileID_t> &TheFileIDs)
/*      Completely removes the files from the Volume's FileGroup identified by FileGroupID,
        that matches the given FileIDs.
        Use this only if you need to remove several files from the same FileGroup.
        To remove one file, use the RemoveFile() function.
        True is returned when FileGroupID matches and when FileIDs matches.
        False is returned for the failure of either of these.
	Changes may or may not have occurred if false is returned.
*/
// Eventually, the failure to match will throw an exception, instead.
// Note, do not change the declaration type of the vector of FileType to FileContainer.
// It should always be a vector.
{
        FileGroupIter GroupMatch = FileGroupID.Find(myFileGroups);
        if (GroupMatch == myFileGroups.end())
        {
                return(false);
        }
	else
	{
		return(GroupMatch->second.RemoveFiles(TheFileIDs));
	}
/*
        for (vector<FileID_t>::const_iterator AFileID = TheFileIDs.begin(); AFileID != TheFileIDs.end(); AFileID++)
        {
                if (!GroupMatch->second.RemoveFile(*AFileID))
                {
                        cerr << "Could not remove file: " << AFile->GiveFileName() << "  from file group: "
                             << GroupMatch->second.GiveLocation() << endl;
                        return(false);
                }
        }

        return(true);
*/
}


bool VolumeClass::AddFile(const FileGroupID_t &FileGroupID, const FileType &NewFile)
/*	Adds the given file to the Volume's FileGroup indicated by FileGroupID.
	If the FileGroupID does not match, false is returned with no changes.
	If the NewFile matches one already in the FileGroup, the 'newer' of the two
	is kept.  If NewFile is not Valid, false is also returned with no changes.
	True is returned otherwise.
	See documentation in FileType.h for definition of 'newer'.
*/
// Eventually, the failure to match will throw an exception, instead.
{
	FileGroupIter GroupMatch = FileGroupID.Find(myFileGroups);
        if (GroupMatch == myFileGroups.end())
	{
		return(false);
	}
	else
	{
		return(GroupMatch->second.AddFile(NewFile));
	}
}

bool VolumeClass::AddFiles(const FileGroupID_t &FileGroupID, const vector <FileType> &NewFiles)
/*      Adds the given files to the Volume's FileGroup indicated by FileGroupID.
        If the FileGroupID does not match, false is returned with no changes.
        If the a NewFile matches one already in the FileGroup, the 'newer' of the two
        is kept.  If the NewFile is not Valid, false is also returned with no changes.
        True is returned otherwise.
        See documentation in FileType.h for definition of 'newer'.
*/
// Eventually, the failure to match will throw an exception, instead.
{
        FileGroupIter GroupMatch = FileGroupID.Find(myFileGroups);
        if (GroupMatch == myFileGroups.end())
        {
                return(false);
        }
	else
	{
		GroupMatch->second.AddFiles(NewFiles);
		return(true);
	}
}


bool VolumeClass::InsertFile(const FileGroupID_t &FileGroupID, const FileType &NewFile)
/* Behaves much like AddFile(), except that if FileGroupID does not match,
   then a new FileGroup is automatically created for the insertion of the NewFile.
   False is returned for the failure of adding the file into the filegroup, with no changes.
   Also, false is returned if NewFile is not Valid.
   True is returned for successful insertion of the file.
*/
// Eventually, failure to match FileGroupID will throw an exception that will need to be caught here.
{
        FileGroupIter GroupMatch = FileGroupID.Find(myFileGroups);
        if (GroupMatch == myFileGroups.end())
        {
                FileGroup TempGroup;
                TempGroup.AddFile(NewFile);

                return(AddFileGroup(FileGroupID, TempGroup));
        }
        else
        {
                return(GroupMatch->second.AddFile(NewFile));
        }
}

	
size_t VolumeClass::GiveFileGroupCount() const
{
	return(myFileGroups.size());
}

size_t VolumeClass::GiveFileCount() const
{
	size_t FileCount = 0;
	for (FileGroupIter_Const AGroup = myFileGroups.begin(); AGroup != myFileGroups.end(); AGroup++)
	{
		FileCount += AGroup->second.GiveFileCount();
	}

	return(FileCount);
}

size_t VolumeClass::GiveFileCount(const FileGroupID_t &FileGroupID) const
// Eventually, the failure to match the FileGroupID will throw an exception.
{
	FileGroupIter_Const GroupMatch = FileGroupID.Find(myFileGroups);
        if (GroupMatch == myFileGroups.end())
	{
		return(string::npos);
	}
	else
	{
		return(GroupMatch->second.GiveFileCount());
	}
}

VolumeClass VolumeClass::FindFiles(const FileID_t &FileID) const
/* Duplicate Files are defined as files that have the same FileID.
   'Completely Duplicate' Files are defined as files that have the same
   FileID and filesize.
   This function only looks for files with the same FileID as the given FileID.
   The resulting set of Files are returned in a fully-qualified VolumeClass
   with FileGroups separating out each duplicate files.
*/
{
	VolumeClass SubVolume;

	for (FileGroupIter_Const AFileGroup = myFileGroups.begin(); AFileGroup != myFileGroups.end(); AFileGroup++)
	{
		// By definition, a FileGroup cannot have duplicate files within itself.
		// So I expect, at most, one file to match the file name for each filegroup.
		// Eventually, failure to match FileID will throw an exception, which will
		// need to be caught here.
		const FileType FileMatch = AFileGroup->second.GiveFile(FileID);

		// when exception handling is introduced, this if statement will be useless
		// since any FileMatches returned without thrown exceptions will be valid.
		if (FileMatch.IsValid())
		{
			FileGroup TempFileGroup;
			TempFileGroup.AddFile(FileMatch);
			SubVolume.AddFileGroup(AFileGroup->first, TempFileGroup);
		}
	}

	return(SubVolume);
}

VolumeClass VolumeClass::FindFiles(const vector <FileID_t> &FileIDs) const
/* Duplicate Files are defined as files that have the same FileID.
   'Completely Duplicate' Files are defined as files that have the same
   FileID and filesize.
   This function only looks for files with the same FileID as the given FileIDs.
   The resulting set of Files are returned in a fully-qualified VolumeClass
   with FileGroups separating out each duplicate files.
*/
{
        VolumeClass SubVolume;

	for (vector<FileID_t>::const_iterator AFileID = FileIDs.begin(); AFileID != FileIDs.end(); AFileID++)
	{
	        for (FileGroupIter_Const AFileGroup = myFileGroups.begin(); AFileGroup != myFileGroups.end(); AFileGroup++)
        	{
                	// By definition, a FileGroup cannot have duplicate files within itself.
	                // So I expect, at most, one file to match the file name for each filegroup.
			// Eventually, failure to match FileID will throw an exception, which will
	                // need to be caught here.
        	        const FileType FileMatch = AFileGroup->second.GiveFile(*AFileID);
			
			// when exception handling is introduced, this if statement will be useless
	                // since any FileMatches returned without thrown exceptions will be valid.
                	if (FileMatch.IsValid())
	                {
        	                FileGroup TempFileGroup;
                	        TempFileGroup.AddFile(FileMatch);
                        	SubVolume.AddFileGroup(AFileGroup->first, TempFileGroup);
                	}
        	}
	}

        return(SubVolume);
}



vector <string> VolumeClass::InitTagWords() const
{
	vector <string> TheTagWords(0);
	TheTagWords.push_back("Volume");
	TheTagWords.push_back("VolumeName");
	TheTagWords.push_back("FileGroup");
	TheTagWords.push_back("MediumType");
	TheTagWords.push_back("BlockSize");
	TheTagWords.push_back("DirName");
	TheTagWords.push_back("Status");
	TheTagWords.push_back("SysName");

	return(TheTagWords);
}

void VolumeClass::WriteJobList(const VolID_t &VolumeID, fstream &JobRequestList) const
{
	if (!myFileGroups.empty())
	{
		const vector <string> TheTagWords = InitTagWords();
		JobRequestList << "<" << TheTagWords[0] << ">\n";
		JobRequestList << "\t<" << TheTagWords[1] << ">" << VolumeID << "</" << TheTagWords[1] << ">\n";

		if (!GiveMediumType().empty())
		{
			JobRequestList << "\t<" << TheTagWords[3] << ">" << GiveMediumType() << "</" << TheTagWords[3] << ">\n";
		}

		if (GiveBlockSize() != string::npos && GiveBlockSize() != 0)
		{
			JobRequestList << "\t<" << TheTagWords[4] << ">" << GiveBlockSize() << "</" << TheTagWords[4] << ">\n";
		}

		if (!GiveDirName().empty())
		{
			JobRequestList << "\t<" << TheTagWords[5] << ">" << GiveDirName() << "</" << TheTagWords[5] << ">\n";
		}

		if (!GiveSysName().empty())
		{
			JobRequestList << "\t<" << TheTagWords[7] << ">" << GiveSysName() << "</" << TheTagWords[7] << ">\n";
		}

		JobRequestList << "\t<" << TheTagWords[6] << ">" << GiveStatusStr() << "</" << TheTagWords[6] << ">\n";
		
		
		for (FileGroupIter_Const AGroup = myFileGroups.begin(); AGroup != myFileGroups.end(); AGroup++)
		{
			AGroup->second.WriteJobList(AGroup->first, JobRequestList);
		}
		

		JobRequestList << "</" << TheTagWords[0] << ">" << endl;

	}
}

void VolumeClass::OptimizeJob()
// does a first-order simplification of the job, removing duplicate files within the volume
// I am also not worrying too much about speed here by switching back and forth between index
// referencing and interator referencing.  This application does not need to be lightning-fast...
{
	// By definition of how the FileGroup class operates, there are NO duplicate files within a FileGroup.
	// Find the definition of Duplicate Files in the comments of JobClass::OptimizeJob().

	for (FileGroupIter FileGroup_A = myFileGroups.begin(); FileGroup_A != myFileGroups.end(); FileGroup_A++)
	{
		vector <FileType> FileGroupA_Files = FileGroup_A->second.GiveFiles();
		// Compare FileGroupA_Files with files in the FileGroups ahead of FileGroup_A.
                // So, for a Volume with 5 FileGroups:
                //      FileGroup1 gets checked against FileGroup2, FileGroup3, FileGroup4, FileGroup5
                //      FileGroup2 gets checked against FileGroup3, FileGroup4, FileGroup5
                //      FileGroup3 gets checked against FileGroup4, FileGroup5
                //      FileGroup4 gets checked against FileGroup5
		
		FileGroupIter FileGroup_B = FileGroup_A;
		FileGroup_B++;

		for (; FileGroup_B != myFileGroups.end(); FileGroup_B++)
		{
			vector <FileType> FileGroupB_Files = FileGroup_B->second.GiveFiles();

			// Now, I will find the intersection of the two lists of files.  Equality is determined by filename ONLY!
			// see comments in JobClass::OptimizeJob() for rational.

			// Note that the intersection of two vectors can never be larger than the smallest of the two vectors.
                        vector <FileType> FileGroupIntersect(  min(FileGroupA_Files.size(), FileGroupB_Files.size())  ); // vector initialized to size
                                                                                                                         // of smallest vector of Files
                        vector <FileType>::iterator EndSpot;

                        // set_intersection will find files that have equivalent names.  It does NOT compare the filetimes or file sizes!!!
                        EndSpot = set_intersection(FileGroupA_Files.begin(), FileGroupA_Files.end(),
                                                   FileGroupB_Files.begin(), FileGroupB_Files.end(), FileGroupIntersect.begin());

                        // I do not bother to resize FileGroupIntersect to IntersectSize because it is a temporary vector that doesn't take
                        // too much space.  I would take a larger penalty spending the time to resize it back than if I just left it as is.
                        // JUST BE SURE TO USE "EndSpot" or "IntersectSize" instead of "FileGroupIntersect.end()"!!!!
                        // So, DON'T SEND "FileGroupIntersect" INTO ANY FUNCTIONS THAT USES ".end()" or ".size()" TO CONTROL THEM!!!
			for (FileIter_Const FileItem = FileGroupIntersect.begin(); FileItem != EndSpot; FileItem++)
			{
				if (IsCompletelyGreaterThan(FileGroup_A->second.GiveFile(FileItem->GiveFileName()), 
							    FileGroup_B->second.GiveFile(FileItem->GiveFileName())))
				{
					FileGroup_B->second.RemoveFile(FileItem->GiveFileName());
				}
				else
				{
					// Possibly add in code here to deal with situation of equivalant files.
					FileGroup_A->second.RemoveFile(FileItem->GiveFileName());
				}
			}// end for loop over list of duplicate files
		}// end for loop for FileGroup_B
	}// end for loop for FileGroup_A

	CleanUp();	// removes any filegroups that are now empty.

	// At this point, this volume contains no duplicate files
}

void VolumeClass::CleanUp()
{
	for (FileGroupIter GroupA = myFileGroups.begin(); GroupA != myFileGroups.end(); )
	{
		if (GroupA->second.GiveFileCount() == 0)
		{
			FileGroupIter TempIter = GroupA;
			GroupA++;
			myFileGroups.erase(TempIter);
		}
		else
		{
			GroupA++;
		}
	}
}


/*----------------------------------------------------------------------------------------*
 *      Overloaded Math operators							  *
 *----------------------------------------------------------------------------------------*/
VolumeClass& operator -= (VolumeClass &OrigVol, const VolumeClass &RemovalVol)
{
	for (FileGroupIter_Const AGroup = RemovalVol.FileGroupBegin(); AGroup != RemovalVol.FileGroupEnd(); AGroup++)
	{
		OrigVol.RemoveFileGroup(AGroup->first, AGroup->second);
	}

	OrigVol.CleanUp();

	return(OrigVol);
}

VolumeClass& operator += (VolumeClass &OrigVol, const VolumeClass &AdditionalVol)
{
	for (FileGroupIter_Const AGroup = AdditionalVol.FileGroupBegin(); AGroup != AdditionalVol.FileGroupEnd(); AGroup++)
        {
                OrigVol.AddFileGroup(AGroup->first, AGroup->second);
        }

	return(OrigVol);
}

VolumeClass& operator - (VolumeClass VolA, const VolumeClass &VolB)
{
	return(VolA -= VolB);
}

VolumeClass& operator + (VolumeClass VolA, const VolumeClass &VolB)
{
	return(VolA += VolB);
}
//---------------------------------------------------------------------------------------------------

/*---------------------------------------------------------------------------------------------------*
 * Efficiency comparison functions								     *
 *---------------------------------------------------------------------------------------------------*/
// May want to expand this a little more to give a better definition of efficiency.
// possibly include a ratio of the number of files per filegroup?
bool IsMoreEfficient(const VolumeClass &VolA, const VolumeClass &VolB)
{
	return((((float) VolA.GiveFileGroupCount()) / ((float) VolA.GiveFileCount())) < 
		(((float) VolB.GiveFileGroupCount()) / ((float) VolB.GiveFileCount())));
}

bool IsLessEfficient(const VolumeClass &VolA, const VolumeClass &VolB)
{
	return((((float) VolA.GiveFileGroupCount()) / ((float) VolA.GiveFileCount())) >
                (((float) VolB.GiveFileGroupCount()) / ((float) VolB.GiveFileCount())));
}

bool IsAsEfficient(const VolumeClass &VolA, const VolumeClass &VolB)
// Might be a little useless because chances are, there won't be two volumes with the same name,
// but it is included here for completeness's sake.
{
        return((((float) VolA.GiveFileGroupCount()) / ((float) VolA.GiveFileCount())) ==
                (((float) VolB.GiveFileGroupCount()) / ((float) VolB.GiveFileCount())));
}
//-------------------------------------------------------------------------------------------------------------------------------------



#endif
