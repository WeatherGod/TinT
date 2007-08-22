#ifndef _FILETYPE_H
#define _FILETYPE_H

#include <string>
#include <vector>
#include <ctime>
#include <sys/types.h>		// for off_t

// Stub declaration of the FileID_t class.  The definition of the class requires the
// declaration of the FileType class.  The FileType class also requires the declaration
// of the FileID_t class.  So, start by declaring an empty FileID_t class...
class FileID_t;

class FileType
{
	public:
		FileType();
		FileType(const FileType &AFile);
		explicit FileType(const string &FileName, const off_t FileSize = -1, const time_t FileTime = -1);
		// FileSize and FileTime are optional arguements, defaulting to a value of -1 each.
		FileType& operator = (const FileType& AFile);

		string GiveFileName() const;
		time_t GiveFileTime() const;
		off_t GiveFileSize() const;

		bool IsValid() const;
		bool ValidConfig() const;

	private:
		string myFileName;
		time_t myFileTime;
		off_t myFileSize;

		bool myIsConfigured;

	friend FileID_t GenerateID(const FileType &AFile);
	friend vector <FileID_t> GenerateIDs(const vector <FileType> &Files);

	// For using both the name and the filesize for comparison.  Filename takes precedence.
	friend bool IsCompletelyIdentical(const FileType &AFile, const FileType &BFile);
	friend bool IsCompletelyLessThan(const FileType &AFile, const FileType &BFile);
	friend bool IsCompletelyGreaterThan(const FileType &AFile, const FileType &BFile);

	// FileTypes are defined to be compared, value-wise, using the filename ONLY!
	// These booleans are meant to be used for sorting purposes ONLY!
	// see comments in JobClass::OptimizeJob for rational.
	friend bool operator == (const FileType &AFile, const FileType &BFile);
	friend bool operator != (const FileType &AFile, const FileType &BFile);
	friend bool operator > (const FileType &AFile, const FileType &BFile);
	friend bool operator < (const FileType &AFile, const FileType &BFile);
	friend bool operator >= (const FileType &AFile, const FileType &BFile);
	friend bool operator <= (const FileType &AFile, const FileType &BFile);

	friend bool operator == (const FileType &AFile, const FileID_t &FileID);
	friend bool operator != (const FileType &AFile, const FileID_t &FileID);
	friend bool operator > (const FileType &AFile, const FileID_t &FileID);
	friend bool operator < (const FileType &AFile, const FileID_t &FileID);
	friend bool operator >= (const FileType &AFile, const FileID_t &FileID);
	friend bool operator <= (const FileType &AFile, const FileID_t &FileID);
	friend bool operator == (const FileID_t &FileID, const FileType &AFile);
	friend bool operator != (const FileID_t &FileID, const FileType &AFile);
	friend bool operator > (const FileID_t &FileID, const FileType &AFile);
	friend bool operator < (const FileID_t &FileID, const FileType &AFile);
	friend bool operator >= (const FileID_t &FileID, const FileType &AFile);
	friend bool operator <= (const FileID_t &FileID, const FileType &AFile);
};

bool IsCompletelyIdentical(const FileType &AFile, const FileType &BFile);
bool IsCompletelyLessThan(const FileType &AFile, const FileType &BFile);
bool IsCompletelyGreaterThan(const FileType &AFile, const FileType &BFile);


struct CompletelyIdentical : public binary_function<const FileType&, const FileType&, bool>
{
        bool operator() (const FileType &AFile, const FileType &BFile);
};

struct CompletelyLessThan : public binary_function<const FileType&, const FileType&, bool>
{
	bool operator() (const FileType &AFile, const FileType &BFile);
};


struct CompletelyGreaterThan : public binary_function<const FileType&, const FileType&, bool>
{
        bool operator() (const FileType &AFile, const FileType &BFile);
};


FileID_t GenerateID(const FileType &AFile);
vector <FileID_t> GenerateIDs(const vector <FileType> &Files);

typedef vector <FileType> FileContainer;
typedef FileContainer::const_iterator FileIter_Const;
typedef FileContainer::iterator FileIter;

// Now that the FileType has been declared, I need to fully declare the FileID_t class
// before going on to define the FileType class.
#include "FileID_t.h"

#endif
