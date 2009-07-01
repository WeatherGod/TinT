#ifndef _READER_H
#define _READER_H

#include <fstream>
#include <vector>
#include "DataFile.h"
#include <string>


class Reader
{
	public:
		Reader();
		vector <DataFile> ProcessEntryDefinitions(const string &Filename);

	private:
		vector <DataFile> GetEntryInfo(ifstream &ReadData);
		vector <string> InitEntryTagWords() const;
};

#endif
