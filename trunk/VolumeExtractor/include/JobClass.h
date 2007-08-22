#ifndef _JOBCLASS_H
#define _JOBCLASS_H

#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <cstddef>			// for size_t, off_t

#include "VolumeClass.h"		// for VolContainer, VolumeIter_Const and VolumeIter are typedef'ed in VolumeClass.h
#include "FileGroup.h"
#include "FileType.h"
#include "VolID_t.h"
#include "FileGroupID_t.h"
#include "FileID_t.h"

#include "VolumeInfo.h"			// a struct for holding basic volume information


class JobClass
{
	public:
		JobClass();
		JobClass(const JobClass &AJob);
		JobClass& operator = (const JobClass &AJob);

		bool LoadJobFile(const string &JobFilename);		// Note that these loading algorithms will append any information in the
		bool LoadCatalogueFile(const string &CataFilename);	// given file to the current structure, NOT overwrite the information.
									// No information is removed from the structure during these functions, either.

		// This function will take information from the container,
		// and use it to supply more information to the Volumes contained in this job.
		void LoadVolumeInfos(const map<string, VolumeInfo> &AvailVolumes);

		bool IsValid() const;
		bool ValidConfig() const;

		bool VolumeExist(const VolID_t &VolumeID) const;
		

		size_t GiveVolumeCount() const;

		vector <VolumeClass> GiveVolumes() const;
		VolumeClass GiveVolume(const VolID_t &VolID) const;
		vector <string> GiveVolumeNames() const;
		VolumeIter_Const VolumeBegin() const;
		VolumeIter_Const VolumeEnd() const;

		string GiveVolume_MediumType(const VolID_t &VolID) const;
		size_t GiveVolume_BlockSize(const VolID_t &VolID) const;
		string GiveVolume_DirName(const VolID_t &VolID) const;
		VolumeInfo GiveVolume_Info(const VolID_t &VolID) const;
		


		size_t GiveFileGroupCount() const;
		size_t GiveFileGroupCount(const VolID_t &VolumeID) const;

		vector <FileGroup> GiveFileGroups(const VolID_t &VolumeID) const;
		FileGroup GiveFileGroup(const VolID_t &VolumeID, const FileGroupID_t &FileGroupID) const;
		vector <size_t> GiveFileGroupLocations(const VolID_t &VolumeID) const;


		size_t GiveFileCount() const;
		size_t GiveFileCount(const VolID_t &VolumeID) const;
		size_t GiveFileCount(const VolID_t &VolumeID, const FileGroupID_t &FileGroupID) const;

		vector <FileType> GiveFiles(const VolID_t &VolumeID, const FileGroupID_t &FileGroupID) const;
		FileType GiveFile(const VolID_t &VolumeID, const FileGroupID_t &FileGroupID, const FileID_t &FileID) const;

		vector <time_t> GiveFileTimes(const VolID_t &VolumeID, const FileGroupID_t &FileGroupID) const;
		time_t GiveFileTime(const VolID_t &VolumeID, const FileGroupID_t &FileGroupID, const FileID_t &FileID) const;

                vector <off_t> GiveFileSizes(const VolID_t &VolumeID, const FileGroupID_t &FileGroupID) const;
		off_t GiveFileSize(const VolID_t &VolumeID, const FileGroupID_t &FileGroupID, const FileID_t &FileID) const;

		vector <string> GiveFileNames(const VolID_t &VolumeID, const FileGroupID_t &FileGroupID) const;



		JobClass FindFiles(const FileID_t &FileID) const;
		JobClass FindFiles(const vector <FileID_t> &FileIDs) const;



		bool RemoveVolume(const VolID_t &VolumeID);
		bool RemoveVolume(const VolID_t &VolumeID, const VolumeClass &RemovalVolume);
		bool AddVolume(const VolID_t &VolumeID, const VolumeClass &NewVolume);

		// Note, this replaces entries in the current job with the contents of the single replacement volume.
		bool ReplaceVolume(const VolID_t &VolumeID, const VolumeClass &ReplacementVolume);
		

		bool RemoveFileGroup(const VolID_t &VolumeID, const FileGroupID_t &FileGroupID);
		bool RemoveFileGroup(const VolID_t &VolumeID, const FileGroupID_t &FileGroupID, const FileGroup &RemovalFileGroup);
		bool AddFileGroup(const VolID_t &VolumeID, const FileGroupID_t &FileGroupID, const FileGroup &NewFileGroup);
		bool InsertFileGroup(const VolID_t &VolumeID, const FileGroupID_t &FileGroupID, const FileGroup &NewFileGroup);



		bool RemoveFile(const VolID_t &VolumeID, const FileGroupID_t &FileGroupID, const FileID_t &FileID);

		// consider deprecating...
		bool RemoveFiles(const VolID_t &VolumeID, const FileGroupID_t &FileGroupID, const vector <FileID_t> &FileIDs);

		bool AddFile(const VolID_t &VolumeID, const FileGroupID_t &FileGroupID, const FileType &NewFile);

		// consider deprecating...
		bool AddFiles(const VolID_t &VolumeID, const FileGroupID_t &FileGroupID, const vector <FileType> &NewFiles);

		bool InsertFile(const VolID_t &VolumeID, const FileGroupID_t &FileGroupID, const FileType &NewFile);

		void OptimizeJob();
		
		bool WriteJobFile(const string &JobFilename) const;
		bool UpdateAndSaveJob(const string &JobFilename);

		void CleanUp();

	private:
		VolContainer myVolumes;
		bool myIsConfigured;
		VolumeClass myDummyVolume;

		vector <string> InitTagWords() const;

		bool LoadJobFile(fstream &JobStream);
		bool WriteJobFile(fstream &JobStream) const;

	friend JobClass& operator -= (JobClass &OrigJob, const JobClass &RemoveJob);
	friend JobClass& operator += (JobClass &OrigJob, const JobClass &AdditionalJob);
	friend JobClass& operator - (JobClass JobA, const JobClass &JobB);
        friend JobClass& operator + (JobClass JobB, const JobClass &JobB);
};

#endif
