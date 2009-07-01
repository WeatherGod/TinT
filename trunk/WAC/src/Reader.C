#ifndef _READER_C
#define _READER_C
using namespace std;

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "DataFile.h"
#include <ConfigUtly.h>			// For FoundStartTag(), FoundEndTag(), ReadNoComments()

#include "Reader.h"

Reader::Reader()
{
}

vector <string> Reader::InitEntryTagWords() const
{
	vector <string> EntryTagWords(2);
	EntryTagWords[0] = "DataDescriptionList";
       	EntryTagWords[1] = "Data";

	return(EntryTagWords);
}

vector <DataFile> Reader::GetEntryInfo(ifstream &ReadData)
{
	const vector <string> EntryTagWords = InitEntryTagWords();
	vector <DataFile> TheDataFiles(0);
	vector <string> TypeNames(0);		// to keep track of any possible duplicate names of <File>

	string FileLine = ReadNoComments(ReadData);

	while (!ReadData.eof() && !FoundStartTag(FileLine, EntryTagWords[0]))	// DataDescriptionList
	{
		FileLine = ReadNoComments(ReadData);
	}

	if (ReadData.eof())
	{
		cerr << "ERROR: Premature end to the config file..." << endl;
		return(vector<DataFile>(0));
	}

	FileLine = ReadNoComments(ReadData);
	bool BadObject = false;

        while ( !ReadData.eof() && !FoundEndTag(FileLine, EntryTagWords[0]) )			// keep going until the end tag is found
        {
		if (!BadObject)
		{
			if (FoundStartTag(FileLine, EntryTagWords[1]))		// <Data>
			{
				DataFile TempType;
				
				if (!TempType.GetEntryInfo(ReadData))
				{
					cerr << "ERROR: Problem in Reader class... Bad DataFile entry..." << endl;
					BadObject = true;
				}
				else
				{
					if (binary_search(TypeNames.begin(), TypeNames.end(), TempType.GiveFileTypeName()))
					{
						cerr << "ERROR: Another entry by this name, " << TempType.GiveFileTypeName()
						     << ", already exists!  Skipping entry..." << endl;
					}
					else
					{
						TheDataFiles.push_back(TempType);
						TypeNames.insert(lower_bound(TypeNames.begin(), TypeNames.end(), TempType.GiveFileTypeName()),
								 TempType.GiveFileTypeName());
					}
				}
			}
			else
			{
				cerr << "ERROR: Unknown tags in DataDescriptionList object!  Here is the line: " << FileLine << endl;
				BadObject = true;
			}
		}

		FileLine = ReadNoComments(ReadData);
	}

	if (ReadData.eof() && !FoundEndTag(FileLine, EntryTagWords[0]))
	{
		cerr << "ERROR: Premature end to the config file..." << endl;
		BadObject = true;
	}

	if (BadObject)
	{
		return(vector<DataFile>(0));
	}
	else
	{
		return(TheDataFiles);
	}
}

vector <DataFile> Reader::ProcessEntryDefinitions(const string &Filename)
{
	ifstream ReadData(Filename.c_str());
	
	if (!ReadData.is_open())
	{
		cerr << "ERROR: Cannot open the table file: " << Filename << endl;
		return(vector<DataFile>(0));
	}

	vector <DataFile> TheDataFiles = GetEntryInfo(ReadData);

	if (TheDataFiles.empty())
	{
		cerr << "WARNING: Empty data files list being returned.  Possible problem with file: " << Filename << endl;
	}

	ReadData.close();

	return(TheDataFiles);
}

#endif
