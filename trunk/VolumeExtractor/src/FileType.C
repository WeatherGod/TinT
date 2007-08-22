#ifndef _FILETYPE_C
#define _FILETYPE_C
using namespace std;

#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <algorithm>			// binary_search(), lower_bound()

#include "FileType.h"
#include "FileID_t.h"
#include <sys/types.h>			// for off_t type for filesizes



//**************************************************************************************************************************
//--------------------------------------------------------------------------------------------------------------------------
//**************************************************************************************************************************

FileType::FileType()
	:	myFileName(""),
		myFileTime(-1),
		myFileSize(-1),
		myIsConfigured(false)
{
}

FileType::FileType(const FileType &AFile)
	:	myFileName(AFile.myFileName),
		myFileTime(AFile.myFileTime),
		myFileSize(AFile.myFileSize),
		myIsConfigured(AFile.myIsConfigured)
{
}

FileType::FileType(const string &FileName, const off_t FileSize, const time_t FileTime)
	:	myFileName(FileName),
		myFileTime(FileTime),
		myFileSize(FileSize),
		myIsConfigured(false)
{
	if (!FileName.empty())
	{
		myIsConfigured = true;
	}
}

FileType& FileType::operator = (const FileType &AFile)
{
	myFileName = AFile.myFileName;
	myFileTime = AFile.myFileTime;
	myFileSize = AFile.myFileSize;
	myIsConfigured = AFile.myIsConfigured;

	return(*this);
}
string FileType::GiveFileName() const
{
	return(myFileName);
}

time_t FileType::GiveFileTime() const
{
	return(myFileTime);
}

off_t FileType::GiveFileSize() const
{
	return(myFileSize);
}

bool FileType::IsValid() const
{
	return(myIsConfigured);
}

bool FileType::ValidConfig() const
{
	return(myIsConfigured);
}

//**************************************************************************************************************************************
//------------------------------------- Special Comparison Functions and operators -----------------------------------------------------
//**************************************************************************************************************************************
//	These functions are meant to deal with special situations where a more strict
//	definition of comparison is needed.  Given are boolean functions to actually do
//	comparisons as normal functions.  With them are special kinds of structures that
//	will act as a binary function (remember that a binary function has arguements on
//	either side of the function, instead of a comma-delimited arguement list).

//	The structures are only given for compatibility with various vector-based
//	functions provided by <algorithm.h> like "includes()".  

bool IsCompletelyIdentical(const FileType &AFile, const FileType &BFile)
{
	return(AFile.myFileName == BFile.myFileName && AFile.myFileTime == BFile.myFileTime && AFile.myFileSize == BFile.myFileSize);
}

bool IsCompletelyLessThan(const FileType &AFile, const FileType &BFile)
// if the names are the same, then is the file 'older'?
{
	return(AFile.myFileName < BFile.myFileName || (AFile.myFileName == BFile.myFileName && AFile.myFileSize < BFile.myFileSize));
}

bool IsCompletelyGreaterThan(const FileType &AFile, const FileType &BFile)
// if the names are the same, then is the file 'newer'?
{
        return(AFile.myFileName > BFile.myFileName || (AFile.myFileName == BFile.myFileName && AFile.myFileSize > BFile.myFileSize));
}


bool CompletelyIdentical::operator() (const FileType &AFile, const FileType&BFile)
{
	return(IsCompletelyIdentical(AFile, BFile));
}


bool CompletelyLessThan::operator() (const FileType &AFile, const FileType &BFile)
{
        return(IsCompletelyLessThan(AFile, BFile));
}


bool CompletelyGreaterThan::operator() (const FileType &AFile, const FileType &BFile)
{
        return(IsCompletelyGreaterThan(AFile, BFile));
}

//****************************************************************************************************************
//----------------------------------------------------------------------------------------------------------------
//****************************************************************************************************************




FileID_t GenerateID(const FileType &AFile)
{
	return(AFile.GiveFileName());
}

vector <FileID_t> GenerateIDs(const vector <FileType> &Files)
{
	vector <FileID_t> TheIDs( Files.size() );
	vector<FileID_t>::iterator AnID = TheIDs.begin();

	for (vector<FileType>::const_iterator AFile = Files.begin(); AFile != Files.end(); AFile++, AnID++)
	{
		*AnID = (FileID_t) AFile->GiveFileName();
	}

	return(TheIDs);
}




//****************************************************************************************************************
//--------------------------------------------- Normal Comparison Operators --------------------------------------
//****************************************************************************************************************

// FileTypes are to be compared using the filename ONLY!
// See rational in the comments for JobClass::OptimizeJob().
// Use these comparisons for sorting purposes.

bool operator == (const FileType &AFile, const FileType &BFile)
{
	return(AFile.myFileName == BFile.myFileName);
}

bool operator != (const FileType &AFile, const FileType &BFile)
{
        return(AFile.myFileName != BFile.myFileName);
}

bool operator > (const FileType &AFile, const FileType &BFile)
{
	return(AFile.myFileName > BFile.myFileName);
}

bool operator < (const FileType &AFile, const FileType &BFile)
{
        return(AFile.myFileName < BFile.myFileName);
}

bool operator >= (const FileType &AFile, const FileType &BFile)
{
        return(AFile.myFileName >= BFile.myFileName);
}

bool operator <= (const FileType &AFile, const FileType &BFile)
{
        return(AFile.myFileName <= BFile.myFileName);
}


bool operator == (const FileType &AFile, const FileID_t &SomeFileID)
{
	return(AFile.myFileName == SomeFileID.GiveName());
}

bool operator != (const FileType &AFile, const FileID_t &SomeFileID)
{
        return(AFile.myFileName != SomeFileID.GiveName());
}

bool operator > (const FileType &AFile, const FileID_t &SomeFileID)
{
        return(AFile.myFileName > SomeFileID.GiveName());
}

bool operator < (const FileType &AFile, const FileID_t &SomeFileID)
{
        return(AFile.myFileName < SomeFileID.GiveName());
}

bool operator >= (const FileType &AFile, const FileID_t &SomeFileID)
{
        return(AFile.myFileName >= SomeFileID.GiveName());
}

bool operator <= (const FileType &AFile, const FileID_t &SomeFileID)
{
        return(AFile.myFileName <= SomeFileID.GiveName());
}

bool operator == (const FileID_t &SomeFileID, const FileType &AFile)
{
        return(SomeFileID.GiveName() == AFile.myFileName);
}

bool operator != (const FileID_t &SomeFileID, const FileType &AFile)
{
        return(SomeFileID.GiveName() != AFile.myFileName);
}

bool operator > (const FileID_t &SomeFileID, const FileType &AFile)
{
        return(SomeFileID.GiveName() > AFile.myFileName);
}

bool operator < (const FileID_t &SomeFileID, const FileType &AFile)
{
        return(SomeFileID.GiveName() < AFile.myFileName);
}

bool operator >= (const FileID_t &SomeFileID, const FileType &AFile)
{
        return(SomeFileID.GiveName() >= AFile.myFileName);
}

bool operator <= (const FileID_t &SomeFileID, const FileType &AFile)
{
        return(SomeFileID.GiveName() == AFile.myFileName);
}
//***************************************************************************************************************
//---------------------------------------------------------------------------------------------------------------
//***************************************************************************************************************
#endif
