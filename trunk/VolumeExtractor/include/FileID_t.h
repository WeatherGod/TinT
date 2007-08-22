#ifndef _FILEID_T_H
#define _FILEID_T_H

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>		// for binary_search(), lower_bound()

#include "FileType.h"

// FileContainer, FileIter_Const, and FileIter are typedef'ed in FileType.h

class FileID_t
{
        public:
                FileID_t();
                FileID_t(const FileID_t &FileID);
                FileID_t(const string &FileName);

                FileIter_Const Find(const FileContainer &Files) const;
                FileIter Find(FileContainer &Files) const;

		string GiveName() const;

		FileID_t& operator = (const FileID_t &IDCopy);

        private:
                string myFileName;

	friend ostream& operator << (ostream &OutStream, const FileID_t &FileID);
        friend istream& operator >> (istream &InStream, FileID_t &FileID);

        friend bool operator == (const FileID_t &FileID_A, const FileID_t &FileID_B);
        friend bool operator != (const FileID_t &FileID_A, const FileID_t &FileID_B);
        friend bool operator < (const FileID_t &FileID_A, const FileID_t &FileID_B);
        friend bool operator > (const FileID_t &FileID_A, const FileID_t &FileID_B);
        friend bool operator <= (const FileID_t &FileID_A, const FileID_t &FileID_B);
        friend bool operator >= (const FileID_t &FileID_A, const FileID_t &FileID_B);
};


#endif
