#ifndef _DATAFILE_H
#define _DATAFILE_H

#include <fstream>
#include <string>
#include <vector>

class DataFile
{
	public:
		DataFile();
		DataFile(const DataFile &AFileType);

		string GiveKeyWord(const size_t &RefNum) const;
		string GiveSearchString() const;
		string GivePatternExplain(const size_t &RefNum) const;
		string GiveAssignExpression(const size_t &RefNum) const;
		string GiveExplainType(const size_t &RefNum) const;
		string GiveDescription() const;
		string GiveFileTypeName() const;
		vector <string> GiveAllKeyWords() const;
		vector <string> GiveAllPatternExplains() const;
		vector <string> GiveAllAssignExpressions() const;
		vector <string> GiveAllExplainTypes() const;
		vector <string> GiveDescriptors() const;
		vector <string> GiveDeclarations() const;
		
		size_t KeyWordCount() const;
		size_t PatternExplainCount() const;

		bool TakeKeyWordList(const string &StringLine);
		bool TakeSearchString(const string &StringLine);
		bool TakeWholePatternExplain(const string &StringLine);
		bool TakeDescription(const string &StringLine);
		bool TakeFileTypeName(const string &StringLine);
		
		// These two functions are the "neglected" functions.  I keep them around, but
		// they are probably not good enough for production code use.
		void ReportInfo() const;
		void PrintDescription() const;

		bool GetEntryInfo(ifstream &ReadData);

		// By default, BaseIndex is set to 1.
		bool WriteEntryInfo(ofstream &WriteData, const size_t BaseIndent) const;

	private:
		string mySearchString;
		vector <string> myPatternExplains;
		vector <string> myAssignExpressions;
		vector <string> myExplainTypes;
		string myDescription;
		string myFileTypeName;
		vector <string> myKeyWords;

		vector <string> InitTagWords() const;

	friend vector <string> GiveAllDescriptors(const vector <DataFile> &Files);
};

#endif
