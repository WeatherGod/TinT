#ifndef _FILEID_T_C
#define _FILEID_T_C
using namespace std;

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>            // for binary_search(), lower_bound()

#include "FileType.h"		// for FileContainer, FileIter_Const, and FileIter are typedef'ed in FileType.h

#include "FileID_t.h"

FileID_t::FileID_t()
        :       myFileName("")
{
}

FileID_t::FileID_t(const FileID_t &FileID)
        :       myFileName(FileID.myFileName)
{
}

FileID_t::FileID_t(const string &FileName)
        :       myFileName(FileName)
{
}

string FileID_t::GiveName() const
{
        return(myFileName);
}

FileIter_Const FileID_t::Find(const FileContainer &Files) const
{
        if (!myFileName.empty())
        {
                if (binary_search(Files.begin(), Files.end(), myFileName))
                {
                        return(lower_bound(Files.begin(), Files.end(), myFileName));
                }
        }

        return(Files.end());
}


FileIter FileID_t::Find(FileContainer &Files) const
{
        if (!myFileName.empty())
        {
                if (binary_search(Files.begin(), Files.end(), myFileName))
                {
                        return(lower_bound(Files.begin(), Files.end(), myFileName));
                }
        }

        return(Files.end());
}


ostream& operator << (ostream &OutStream, const FileID_t &FileID)
{
        return(OutStream << FileID.myFileName);
}

istream& operator >> (istream &InStream, FileID_t &FileID)
{
        return(InStream >> FileID.myFileName);
}

FileID_t& FileID_t::operator = (const FileID_t &RHS)
{
        myFileName = RHS.myFileName;
        return(*this);
}

bool operator == (const FileID_t &FileID_A, const FileID_t &FileID_B)
{
        return(FileID_A.myFileName == FileID_B.myFileName);
}

bool operator != (const FileID_t &FileID_A, const FileID_t &FileID_B)
{
        return(FileID_A.myFileName != FileID_B.myFileName);
}

bool operator < (const FileID_t &FileID_A, const FileID_t &FileID_B)
{
        return(FileID_A.myFileName < FileID_B.myFileName);
}

bool operator > (const FileID_t &FileID_A, const FileID_t &FileID_B)
{
        return(FileID_A.myFileName > FileID_B.myFileName);
}

bool operator <= (const FileID_t &FileID_A, const FileID_t &FileID_B)
{
        return(FileID_A.myFileName <= FileID_B.myFileName);
}

bool operator >= (const FileID_t &FileID_A, const FileID_t &FileID_B)
{
        return(FileID_A.myFileName >= FileID_B.myFileName);
}



#endif
