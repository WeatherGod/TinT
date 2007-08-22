#ifndef _FILEGROUPID_T_H
#define _FILEGROUPID_T_H

#include <iostream>
#include <map>

#include "FileGroup.h"

// Note: FileGroupContainer, FileGroupIter_Const and FileGroupIter are typedef'ed in FileGroup.h

class FileGroupID_t
{
        public:
                FileGroupID_t();
		FileGroupID_t(const FileGroupID_t &GroupID);
                FileGroupID_t(const size_t FileGroupLocation);

                FileGroupIter_Const Find(const FileGroupContainer &FileGroups) const;
                FileGroupIter Find(FileGroupContainer &FileGroups) const;

		size_t GiveLocation() const;

		FileGroupID_t& operator = (const FileGroupID_t &RHS);

        private:
                size_t myFileGroupLocation;

	friend ostream& operator << (ostream &OutStream, const FileGroupID_t &GroupID);
	friend istream& operator >> (istream &InStream, FileGroupID_t &GroupID);
//	friend FileGroupID_t& operator = (FileGroupID_t &LHS, const FileGroupID_t &RHS);

        friend bool operator == (const FileGroupID_t &LHS, const FileGroupID_t &RHS);
        friend bool operator != (const FileGroupID_t &LHS, const FileGroupID_t &RHS);
        friend bool operator > (const FileGroupID_t &LHS, const FileGroupID_t &RHS);
        friend bool operator < (const FileGroupID_t &LHS, const FileGroupID_t &RHS);
        friend bool operator >= (const FileGroupID_t &LHS, const FileGroupID_t &RHS);
        friend bool operator <= (const FileGroupID_t &LHS, const FileGroupID_t &RHS);

};

#endif
