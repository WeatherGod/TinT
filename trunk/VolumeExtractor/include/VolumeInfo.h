#ifndef _VOLUMEINFO_H
#define _VOLUMEINFO_H

#include <string>
#include <cstddef>		// for size_t

enum VolumeStatus_t {New_Vol, Listed_Vol, Catalogued_Vol, Damaged_Vol, Destroyed_Vol};

// May change this into a fully qualified class...
struct VolumeInfo
{
        string VolumeName;
        string MediumType;
        size_t BlockSize;
        string DirName;
	string SysName;
	VolumeStatus_t VolStatus;

        VolumeInfo();
        VolumeInfo(const VolumeInfo &ACopy);
	VolumeInfo& operator = (const VolumeInfo &ACopy);

	void TakeStatusStr(const string &StrVal);
	string GiveStatusStr() const;
};

#endif
