#ifndef _VOLID_T_C
#define _VOLID_T_C
using namespace std;

#include <iostream>
#include <string>
#include <map>

#include "VolumeClass.h"		// for VolContainer, VolumeIter_Const and VolumeIter are typedef'ed in VolumeClass.h
#include "VolID_t.h"

VolID_t::VolID_t()
        :       myVolumeName("")
{
}

VolID_t::VolID_t(const VolID_t &VolIDCopy)
	:	myVolumeName(VolIDCopy.myVolumeName)
{
}

VolID_t::VolID_t(const string &VolumeName)
        :       myVolumeName(VolumeName)
{
}


VolumeIter_Const VolID_t::Find(const VolContainer &Volumes) const
{
        if (!myVolumeName.empty())
        {
                return(Volumes.find(*this));
        }

        return(Volumes.end());
}

VolumeIter VolID_t::Find(VolContainer &Volumes) const
{
        if (!myVolumeName.empty())
        {
                return(Volumes.find(*this));
        }

        return(Volumes.end());
}

string VolID_t::GiveName() const
{
	return(myVolumeName);
}




ostream& operator << (ostream &OutStream, const VolID_t &VolID)
{
	return(OutStream << VolID.myVolumeName);
}

istream& operator >> (istream &InStream, VolID_t &VolID)
{
	return(InStream >> VolID.myVolumeName);
}

VolID_t& VolID_t::operator = (const VolID_t &RHS)
{
	myVolumeName = RHS.myVolumeName;
	return(*this);
}

bool operator == (const VolID_t &VolID_A, const VolID_t &VolID_B)
{
	return(VolID_A.myVolumeName == VolID_B.myVolumeName);
}

bool operator != (const VolID_t &VolID_A, const VolID_t &VolID_B)
{
        return(VolID_A.myVolumeName != VolID_B.myVolumeName);
}

bool operator < (const VolID_t &VolID_A, const VolID_t &VolID_B)
{
        return(VolID_A.myVolumeName < VolID_B.myVolumeName);
}

bool operator > (const VolID_t &VolID_A, const VolID_t &VolID_B)
{
        return(VolID_A.myVolumeName > VolID_B.myVolumeName);
}

bool operator <= (const VolID_t &VolID_A, const VolID_t &VolID_B)
{
        return(VolID_A.myVolumeName <= VolID_B.myVolumeName);
}

bool operator >= (const VolID_t &VolID_A, const VolID_t &VolID_B)
{
        return(VolID_A.myVolumeName >= VolID_B.myVolumeName);
}


#endif
