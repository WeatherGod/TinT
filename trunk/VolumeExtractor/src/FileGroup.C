#ifndef _FILEGROUP_C
#define _FILEGROUP_C
using namespace std;

#include <iostream>
#include <fstream>

#include <vector>
#include <string>
#include <ctime>

#include <algorithm>			// for lower_bound(), binary_search()

#include "FileGroup.h"
#include "FileType.h"			// for FileContainer, FileIter_Const and FileIter are typedef'ed in FileType.h
#include "FileID_t.h"

#include "StrUtly.h"			// for RipWhiteSpace(), StrToInt()
#include "ConfigUtly.h"			// for ReadNoComments(), StripTags(), FoundStartTag(), FoundEndTag()

/*----------------------------------------------------------------------------------------------*
 *					FileGroup Class						*
 *----------------------------------------------------------------------------------------------*/

//	***** Constructors *****
FileGroup::FileGroup()
	:	myFiles(),
		myIsConfigured(false),
		myDummyFileType()
{
}

FileGroup::FileGroup(const FileGroup &AGroup)
	:	myFiles(AGroup.myFiles),
		myIsConfigured(AGroup.myIsConfigured),
		myDummyFileType(AGroup.myDummyFileType)
{
}

FileGroup& FileGroup::operator = (const FileGroup &AGroup)
{
	myFiles = AGroup.myFiles;
	myIsConfigured = AGroup.myIsConfigured;
	myDummyFileType = AGroup.myDummyFileType;

	return(*this);
}

//	***** Loader Functions *****
FileGroupID_t FileGroup::ReadJobInfo(string &FileLine, fstream &ReadData)
{
	const vector <string> TheTagWords = InitTagWords();
	bool BadObject = false;
	FileGroupID_t GroupLocation;

	while (!FoundEndTag(FileLine, TheTagWords[0]) && !ReadData.eof())
	{
		if (!BadObject)
		{
			if (FoundStartTag(FileLine, TheTagWords[1]))	// FileLocation
			{
				GroupLocation = (FileGroupID_t) StrToSize_t(StripTags(FileLine, TheTagWords[1]));
			}
			else if (FoundStartTag(FileLine, TheTagWords[2]))	// Files
			{
				FileLine = ReadNoComments(ReadData);
				while (!FoundEndTag(FileLine, TheTagWords[2]) && !ReadData.eof())
				{
					vector <string> TempHold = TakeDelimitedList(FileLine, ',');
					if (TempHold.empty())
					{
						BadObject = true;
						cerr << "\nProblem in FileGroup object... Here is the line: " << FileLine << endl;
					}
					else if (TempHold.size() == 1)
					{
						FileType TempFile(RipWhiteSpace(TempHold[0]));

						if (!TempFile.ValidConfig())
						{
							cerr << "\nProblem in FileGroup object... Invalid FileType object..." << endl;
							BadObject = true;
						}
						else
						{
							AddFile(TempFile);
						}
					}
					else
					{
						// Filename and filesize
						FileType TempFile(RipWhiteSpace(TempHold[0]), StrToOff_t(TempHold[1]));
						if (!TempFile.ValidConfig())
						{
							cerr << "\nProblem in FileGroup object... Invalid FileType object..." << endl;
							BadObject = true;
						}
						else
						{
							AddFile(TempFile);
						}
					}
					
					FileLine = ReadNoComments(ReadData);
				}
			}
			else
			{
				BadObject = true;
				cerr << "\nProblem in FileGroup object... Here is the line: " << FileLine << endl;
			}
		}

		FileLine = ReadNoComments(ReadData);
	}// end while loop

	if (GroupLocation != (FileGroupID_t) string::npos && !BadObject)
	{
		myIsConfigured = true;
	}

	return(GroupLocation);
}

//	***** Validity Checkers	*****
bool FileGroup::ValidConfig() const
{
        return(myIsConfigured);
}

bool FileGroup::IsValid() const
// I think I need a better definition of Valid than this...
{
        return(!myFiles.empty());
}


//	***** Member Values Returners *****
FileType FileGroup::GiveFile(const FileID_t &FileID) const
{
	FileIter_Const FileMatch = FileID.Find(myFiles);
	if (FileMatch == myFiles.end())
	{
		return(myDummyFileType);
	}
	else
	{
		return(*FileMatch);
	}
}


vector <FileType> FileGroup::GiveFiles() const
{
	return(myFiles);
}

FileIter_Const FileGroup::FileBegin() const
{
	return(myFiles.begin());
}

FileIter_Const FileGroup::FileEnd() const
{
	return(myFiles.end());
}

vector <string> FileGroup::GiveFileNames() const
{
	vector <string> FileNames(myFiles.size());
	vector<string>::iterator AName = FileNames.begin();
	for (FileIter_Const AFile = myFiles.begin(); AFile != myFiles.end(); AFile++, AName++)
	{
		*AName = AFile->GiveFileName();
	}

	return(FileNames);
}

vector <time_t> FileGroup::GiveFileTimes() const
{
	vector <time_t> FileTimes(myFiles.size());
	vector<time_t>::iterator ATime = FileTimes.begin();
	for (FileIter_Const AFile = myFiles.begin(); AFile != myFiles.end(); AFile++, ATime++)
	{
		*ATime = AFile->GiveFileTime();
	}

	return(FileTimes);
}

vector <off_t> FileGroup::GiveFileSizes() const
{
	vector <off_t> FileSizes(myFiles.size());
	vector<off_t>::iterator ASize = FileSizes.begin();
	for (FileIter_Const AFile = myFiles.begin(); AFile != myFiles.end(); AFile++, ASize++)
	{
		*ASize = AFile->GiveFileSize();
	}

	return(FileSizes);
}

time_t FileGroup::GiveFileTime(const FileID_t &FileID) const
// NEED SOMETHING BETTER THAN (time_t)-1 !!!!
// maybe throwing errors instead?
{
	FileIter_Const FileMatch = FileID.Find(myFiles);
	if (FileMatch == myFiles.end())
	{
		return((time_t) -1);
	}
	else
	{
		return(FileMatch->GiveFileTime());
	}
}

off_t FileGroup::GiveFileSize(const FileID_t &FileID) const
// maybe throwing errors instead?
{
        FileIter_Const FileMatch = FileID.Find(myFiles);
        if (FileMatch == myFiles.end())
	{
                return(-1);
        }
	else
	{
        	return(FileMatch->GiveFileSize());
	}
}


bool FileGroup::RemoveFile(const FileID_t &FileID)
// Throw error instead
{
	FileIter FileMatch = FileID.Find(myFiles);
        if (FileMatch == myFiles.end())
	{
		return(false);
	}
	else
	{
		myFiles.erase(FileMatch);
		return(true);
	}
}

bool FileGroup::RemoveFiles(const vector <FileID_t> &FileIDs)
{
	for (vector<FileID_t>::const_iterator AnID = FileIDs.begin(); AnID != FileIDs.end(); AnID++)
	{
		FileIter FileMatch = AnID->Find(myFiles);
	        if (FileMatch == myFiles.end())
	        {
			cerr << "ERROR: Couldn't match FileID for removing multiple files.\n"
			     << "FileID name: " << AnID->GiveName() << endl;
			return(false);
		}

       	        myFiles.erase(FileMatch);
	}

	return(true);
}


bool FileGroup::AddFile(const FileType &NewFile)
// Never will there be duplicate files within a FileGroup...
// It is just not gonna be allowed by definition.
{
	if (NewFile.IsValid())
	{
		if (!binary_search(myFiles.begin(), myFiles.end(), NewFile))
		{
			myFiles.insert(lower_bound(myFiles.begin(), myFiles.end(), NewFile), 
				       NewFile);
		}
		else
		{
			// If the filenames are the same, then check to see which one is bigger (filesize-wise)
			// and keep that one.
			FileIter FileIter = lower_bound(myFiles.begin(), myFiles.end(), NewFile);
			if (NewFile.GiveFileSize() > FileIter->GiveFileSize())
			{
				// The NewFile's filesize is larger than the one I already have.  Replace it!
				*FileIter = NewFile;
			}
		}

		return(true);
	}
	else
	{
		return(false);
	}
}

bool FileGroup::AddFiles(const vector <FileType> &NewFiles)
// Never will there be duplicate files within a FileGroup...
// It is just not gonna be allowed by definition.
{
	for (vector<FileType>::const_iterator AFile = NewFiles.begin(); AFile != NewFiles.end(); AFile++)
	{
		if (!AddFile(*AFile))
		{
			cerr << "\nProblem adding a file to the filegroup..." << endl;
			cerr << "File: " << AFile->GiveFileName() << "  " << AFile->GiveFileSize() << endl;
			return(false);
		}
	}

	return(true);
}


size_t FileGroup::GiveFileCount() const
{
	return(myFiles.size());
}

void FileGroup::WriteJobList(const FileGroupID_t &GroupID, fstream &JobRequestList) const
{
	if (myFiles.size() > 0)
	{
		const vector <string> TheTagWords = InitTagWords();

		JobRequestList << "\t<" << TheTagWords[0] << ">" << endl;
		JobRequestList << "\t\t<" << TheTagWords[1] << ">" << GroupID << "</" << TheTagWords[1] << ">" << endl;
		JobRequestList << "\t\t<" << TheTagWords[2] << ">" << endl;

		for (FileIter_Const AFile = myFiles.begin(); AFile != myFiles.end(); AFile++)
		{
			JobRequestList << "\t\t\t" << AFile->GiveFileName() << ", " << AFile->GiveFileSize() << endl;
		}

		JobRequestList << "\t\t</" << TheTagWords[2] << ">" << endl;
		JobRequestList << "\t</" << TheTagWords[0] << ">" << endl;
	}
}

vector <string> FileGroup::InitTagWords() const
{
	vector <string> TheTagWords(0);
	TheTagWords.push_back("FileGroup");
	TheTagWords.push_back("FileLocation");
	TheTagWords.push_back("Files");
	return(TheTagWords);
}


/*----------------------------------------------------------------------------------------*
 *      Overloaded Math operators                                                         *
 *----------------------------------------------------------------------------------------*/
FileGroup& operator -= (FileGroup &OrigGroup, const FileGroup &RemovalGroup)
{
	// Any files that don't exist in OrigGroup that is in RemovalGroup will cause RemoveFiles to return
	// a value of false, but we don't check for it.  Also, when error handling is introduced, we will need
	// to catch the exception thrown for the failure to match the ID.
        for (FileIter_Const AFile = RemovalGroup.FileBegin(); AFile != RemovalGroup.FileEnd(); AFile++)
        {
                OrigGroup.RemoveFile(GenerateID(*AFile));
        }

	return(OrigGroup);
}

FileGroup& operator += (FileGroup &OrigGroup, const FileGroup &AdditionalGroup)
{
	// Right now, it is pretty dumb.  nothing will be thrown if there are any problems.
        for (FileIter_Const AFile = AdditionalGroup.FileBegin(); AFile != AdditionalGroup.FileEnd(); AFile++)
        {
                OrigGroup.AddFile(*AFile);
        }

	return(OrigGroup);
}

FileGroup& operator - (FileGroup GroupA, const FileGroup &GroupB)
{
        return(GroupA -= GroupB);
}

FileGroup& operator + (FileGroup GroupA, const FileGroup &GroupB)
{
        return(GroupA += GroupB);
}
//---------------------------------------------------------------------------------------------------

#endif
