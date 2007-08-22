#ifndef _VOLUMEINFO_C
#define _VOLUMEINFO_C
using namespace std;

#include <string>
#include <cstddef>		// for size_t

#include "VolumeInfo.h"		// for the VolumeStatus_t enum as well.


VolumeInfo::VolumeInfo()
	:       VolumeName(""),
                MediumType(""),
                BlockSize(0),
                DirName(""),
		SysName(""),
		VolStatus(Catalogued_Vol)		// give benefit of the doubt.
{
}

VolumeInfo::VolumeInfo(const VolumeInfo &ACopy)
	:       VolumeName(ACopy.VolumeName),
                MediumType(ACopy.MediumType),
                BlockSize(ACopy.BlockSize),
                DirName(ACopy.DirName),
		SysName(ACopy.SysName),
		VolStatus(ACopy.VolStatus)
{
}

VolumeInfo& VolumeInfo::operator = (const VolumeInfo &ACopy)
{
	VolumeName = ACopy.VolumeName;
	MediumType = ACopy.MediumType;
	BlockSize = ACopy.BlockSize;
	DirName = ACopy.DirName;
	SysName = ACopy.SysName;
	VolStatus = ACopy.VolStatus;
}

void VolumeInfo::TakeStatusStr(const string &StrVal)
{
	if (StrVal == "New")
	{
		VolStatus = New_Vol;
	}
	else if (StrVal == "Listed")
	{
		VolStatus = Listed_Vol;
	}
	else if (StrVal == "Catalogued")
	{
		VolStatus = Catalogued_Vol;
	}
	else if (StrVal == "Damaged")
	{
		VolStatus = Damaged_Vol;
	}
	else if (StrVal == "Destroyed")
	{
		VolStatus = Destroyed_Vol;
	}
}

string VolumeInfo::GiveStatusStr() const
{
	string ReturnStr;
	switch (VolStatus)
	{
	case New_Vol:
		ReturnStr = "New";
		break;
	case Listed_Vol:
		ReturnStr = "Listed";
		break;
	case Catalogued_Vol:
		ReturnStr = "Catalogued";
		break;
	case Damaged_Vol:
		ReturnStr = "Damaged";
		break;
	case Destroyed_Vol:
		ReturnStr = "Destroyed";
		break;
	default:
		ReturnStr = "";
		break;
	}

	return(ReturnStr);
}

#endif
