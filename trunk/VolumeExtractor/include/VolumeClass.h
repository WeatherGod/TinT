#ifndef _VOLUMECLASS_H
#define _VOLUMECLASS_H

#include <fstream>
#include <vector>
#include <string>

#include "FileGroup.h"			// for FileGroupIter_Const and FileGroupIter are typedef'ed in FileGroup.h
#include "FileType.h"
#include "FileGroupID_t.h"
#include "FileID_t.h"
#include "VolumeInfo.h"			// for struct VolumeInfo



class VolID_t;

class VolumeClass
{
	public:
		VolumeClass();
		VolumeClass(const VolumeClass &AVol);
		VolumeClass& operator = (const VolumeClass &AVol);

		VolID_t ReadJobInfo(string &FileLine, fstream &ReadData);

		bool ValidConfig() const;
		bool IsValid() const;

		string GiveMediumType() const;
		size_t GiveBlockSize() const;
		string GiveDirName() const;
		string GiveSysName() const;
		VolumeStatus_t GiveVolStatus() const;
		string GiveStatusStr() const;
		VolumeInfo GiveVolumeInfo() const;

		void TakeMediumType(const string &TypeName);
		void TakeBlockSize(const size_t &BlockSize);
		void TakeDirName(const string &DirName);
		void TakeSysName(const string &SysName);
		void TakeVolStatus(const VolumeStatus_t &StatusVal);
		void TakeVolStatus(const string &VolStr);
		void TakeVolumeInfo(const VolumeInfo &VolInfoCopy);

		vector <size_t> GiveFileGroupLocations() const;
		vector <FileGroup> GiveFileGroups() const;
		FileGroup GiveFileGroup(const FileGroupID_t &FileGroupID) const;
		FileGroupIter_Const FileGroupBegin() const;
		FileGroupIter_Const FileGroupEnd() const;

		vector <FileType> GiveFiles(const FileGroupID_t &FileGroupID) const;
		FileType GiveFile(const FileGroupID_t &FileGroupID, const FileID_t &FileID) const;

		vector <string> GiveFileNames(const FileGroupID_t &FileGroupID) const;

		vector <off_t> GiveFileSizes(const FileGroupID_t &FileGroupID) const;
		off_t GiveFileSize(const FileGroupID_t &FileGroupID, const FileID_t &FileID) const;

		vector <time_t> GiveFileTimes(const FileGroupID_t &FileGroupID) const;
		time_t GiveFileTime(const FileGroupID_t &FileGroupID, const FileID_t &FileID) const;

		VolumeClass FindFiles(const FileID_t &FileID) const;
		VolumeClass FindFiles(const vector <FileID_t> &FileIDs) const;

		bool RemoveFileGroup(const FileGroupID_t &FileGroupID);
		bool RemoveFileGroup(const FileGroupID_t &FileGroupID, const FileGroup &RemovalGroup);
		bool AddFileGroup(const FileGroupID_t &FileGroupID, const FileGroup &NewFileGroup);

		bool RemoveFile(const FileGroupID_t &FileGroupID, const FileID_t &FileID);
		bool RemoveFiles(const FileGroupID_t &FileGroupID, const vector <FileID_t> &FileIDs);
		bool AddFile(const FileGroupID_t &FileGroupID, const FileType &NewFile);
		bool AddFiles(const FileGroupID_t &FileGroupID, const vector <FileType> &NewFiles);
		bool InsertFile(const FileGroupID_t &FileGroupID, const FileType &NewFile);


		size_t GiveFileGroupCount() const;

		size_t GiveFileCount() const;
		size_t GiveFileCount(const FileGroupID_t &FileGroupID) const;

		void WriteJobList(const VolID_t &VolID, fstream &JobRequestList) const;

		void OptimizeJob();
		void CleanUp();

	private:
		VolumeInfo myVolumeInfo;
		FileGroupContainer myFileGroups;
		bool myIsConfigured;

		FileGroup myDummyFileGroup;

		vector <string> InitTagWords() const;

	friend VolumeClass& operator += (VolumeClass &VolA, const VolumeClass &VolB);
	friend VolumeClass& operator -= (VolumeClass &VolA, const VolumeClass &VolB);
	friend VolumeClass& operator + (VolumeClass VolA, const VolumeClass &VolB);
        friend VolumeClass& operator - (VolumeClass VolA, const VolumeClass &VolB);

	friend bool IsMoreEfficient(const VolumeClass &VolA, const VolumeClass &VolB);
	friend bool IsLessEfficient(const VolumeClass &VolA, const VolumeClass &VolB);
	friend bool IsAsEfficient(const VolumeClass &VolA, const VolumeClass &VolB);
};

typedef map<VolID_t, VolumeClass> VolContainer;
typedef VolContainer::iterator VolumeIter;
typedef VolContainer::const_iterator VolumeIter_Const;

#include "VolID_t.h"


#endif
