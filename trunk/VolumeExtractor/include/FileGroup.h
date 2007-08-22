#ifndef _FILEGROUP_H
#define _FILEGROUP_H

#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <ctime>

#include "StrUtly.h"
#include "ConfigUtly.h"

#include "FileType.h"
#include "FileID_t.h"

// Note: FileContainer, FileIter_Const and FileIter are typedef'ed in FileType.h

// declaration of this class to deal with circular dependency.
// This circular dependency should be eliminated shortly.
class FileGroupID_t;

class FileGroup
{
	public:
		FileGroup();
		FileGroup(const FileGroup &AGroup);
		FileGroup& operator = (const FileGroup &AGroup);
		

		// These functions work to UPDATE the current class.  No duplicate files are stored.
		// See JobClass::OptimizeJob() for definition of duplicate files.  If, while loading,
		// a duplicate file is encountered, the larger (filesize-wise) will be kept.
		FileGroupID_t ReadJobInfo(string &FileLine, fstream &ReadData);

		// Dumb validity checkers.  Need to make it smarter.
		bool ValidConfig() const;
		bool IsValid() const;

		size_t GiveFileCount() const;

		// Need to think about rules regarding what member functions are allowed...
		vector <FileType> GiveFiles() const;
		FileType GiveFile(const FileID_t &FileID) const;
		FileIter_Const FileBegin() const;
		FileIter_Const FileEnd() const;

		vector <string> GiveFileNames() const;
		vector <time_t> GiveFileTimes() const;
		vector <off_t> GiveFileSizes() const;

		time_t GiveFileTime(const FileID_t &FileID) const;
		off_t GiveFileSize(const FileID_t &FileID) const;

		bool RemoveFile(const FileID_t &FileID);
		bool RemoveFiles(const vector <FileID_t> &FileIDs);
		bool AddFile(const FileType &NewFile);
		bool AddFiles(const vector <FileType> &NewFiles);

		void WriteJobList(const FileGroupID_t &FileGroupID, fstream &JobRequestList) const;
		
	private:
		FileContainer myFiles;
		FileType myDummyFileType;

		bool myIsConfigured;

		vector <string> InitTagWords() const;

	friend FileGroup& operator += (FileGroup &GroupA, const FileGroup &GroupB);
	friend FileGroup& operator -= (FileGroup &GroupA, const FileGroup &GroupB);
	friend FileGroup& operator + (FileGroup GroupA, const FileGroup &GroupB);
	friend FileGroup& operator - (FileGroup GroupA, const FileGroup &GroupB);	

	/*
	// Need to more fully test these boolean functions!!!!!
	friend bool operator == (const FileGroup &AFileGroup, const FileGroup &BFileGroup);
	friend bool operator != (const FileGroup &AFileGroup, const FileGroup &BFileGroup);
	friend bool operator > (const FileGroup &AFileGroup, const FileGroup &BFileGroup);
	friend bool operator < (const FileGroup &AFileGroup, const FileGroup &BFileGroup);
	friend bool operator >= (const FileGroup &AFileGroup, const FileGroup &BFileGroup);
	friend bool operator <= (const FileGroup &AFileGroup, const FileGroup &BFileGroup);
	*/
};

typedef map<FileGroupID_t, FileGroup> FileGroupContainer;
typedef FileGroupContainer::const_iterator FileGroupIter_Const;
typedef FileGroupContainer::iterator FileGroupIter;

#include "FileGroupID_t.h"

#endif
