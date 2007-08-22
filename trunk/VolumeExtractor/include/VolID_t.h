#ifndef _VOLID_T_H
#define _VOLID_T_H

#include <iostream>
#include <string>
#include <map>

#include "VolumeClass.h"		// for VolContainer, VolumeIter_Const and VolumeIter are typedef'ed in VolumeClass.h

class VolID_t
{
        public:
                VolID_t();
		VolID_t(const VolID_t &VolIDCopy);
                VolID_t(const string &VolumeName);

                VolumeIter_Const Find(const VolContainer &Volumes) const;
                VolumeIter Find(VolContainer &Volumes) const;

		string GiveName() const;

		VolID_t& operator = (const VolID_t &RHS);

        private:
                string myVolumeName;

	friend ostream& operator << (ostream &OutStream, const VolID_t &VolID);
	friend istream& operator >> (istream &InStream, VolID_t &VolID);
//	friend VolID_t& operator = (VolID_t &LHS, const VolID_t &RHS);

	friend bool operator == (const VolID_t &VolID_A, const VolID_t &VolID_B);
	friend bool operator != (const VolID_t &VolID_A, const VolID_t &VolID_B);
	friend bool operator < (const VolID_t &VolID_A, const VolID_t &VolID_B);
	friend bool operator > (const VolID_t &VolID_A, const VolID_t &VolID_B);
	friend bool operator <= (const VolID_t &VolID_A, const VolID_t &VolID_B);
	friend bool operator >= (const VolID_t &VolID_A, const VolID_t &VolID_B);
};

#endif
