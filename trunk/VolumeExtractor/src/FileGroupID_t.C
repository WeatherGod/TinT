#ifndef _FILEGROUPID_T_C
#define _FILEGROUPID_T_C
using namespace std;


#include <iostream>
#include <map>

#include "FileGroup.h"		// for FileGroupContainer, FileGroupIter_Const and FileGroupIter are typedef'ed in FileGroup.h
#include "FileGroupID_t.h"


FileGroupID_t::FileGroupID_t()
        :       myFileGroupLocation(string::npos)
{
}

FileGroupID_t::FileGroupID_t(const FileGroupID_t &FileGroupID)
        :       myFileGroupLocation(FileGroupID.myFileGroupLocation)
{
}

FileGroupID_t::FileGroupID_t(const size_t FileGroupLocation)
        :       myFileGroupLocation(FileGroupLocation)
{
}

size_t FileGroupID_t::GiveLocation() const
{
	return(myFileGroupLocation);
}

FileGroupIter_Const FileGroupID_t::Find(const FileGroupContainer &FileGroups) const
{
        if (myFileGroupLocation != string::npos)
        {
                return(FileGroups.find(*this));
        }

        return(FileGroups.end());
}

FileGroupIter FileGroupID_t::Find(FileGroupContainer &FileGroups) const
{
        if (myFileGroupLocation != string::npos)
        {
                return(FileGroups.find(*this));
        }

        return(FileGroups.end());
}


ostream& operator << (ostream &OutStream, const FileGroupID_t &GroupID)
{
	return(OutStream << GroupID.myFileGroupLocation);
}

istream& operator >> (istream &InStream, FileGroupID_t &GroupID)
{
	return(InStream >> GroupID.myFileGroupLocation);
}

FileGroupID_t& FileGroupID_t::operator = (const FileGroupID_t &RHS)
{
	myFileGroupLocation = RHS.myFileGroupLocation;
	return(*this);
}


bool operator == (const FileGroupID_t &LHS, const FileGroupID_t &RHS)
{
        return(LHS.myFileGroupLocation == RHS.myFileGroupLocation);
}

bool operator != (const FileGroupID_t &LHS, const FileGroupID_t &RHS)
{
        return(LHS.myFileGroupLocation != RHS.myFileGroupLocation);
}

bool operator > (const FileGroupID_t &LHS, const FileGroupID_t &RHS)
{
        return(LHS.myFileGroupLocation > RHS.myFileGroupLocation);
}

bool operator < (const FileGroupID_t &LHS, const FileGroupID_t &RHS)
{
        return(LHS.myFileGroupLocation < RHS.myFileGroupLocation);
}

bool operator >= (const FileGroupID_t &LHS, const FileGroupID_t &RHS)
{
        return(LHS.myFileGroupLocation >= RHS.myFileGroupLocation);
}

bool operator <= (const FileGroupID_t &LHS, const FileGroupID_t &RHS)
{
        return(LHS.myFileGroupLocation <= RHS.myFileGroupLocation);
}


#endif
