#ifndef _DATAFILE_C
#define _DATAFILE_C
using namespace std;

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cctype>		// for isspace()

#include "DataFile.h"
#include <StrUtly.h>		// for TakeDelimitedList(), RipWhiteSpace()
#include <ConfigUtly.h>		// for FoundStartTag(), FoundEndTag(), ReadNoComments()

DataFile::DataFile()
	:	mySearchString(""),
		myPatternExplains(0, ""),
		myAssignExpressions(0, ""),
		myExplainTypes(0, ""),
		myDescription(""),
		myKeyWords(0, ""),
		myFileTypeName("")
{
}

DataFile::DataFile(const DataFile &AFileType)
        :       mySearchString(AFileType.mySearchString),
                myPatternExplains(AFileType.myPatternExplains),
                myAssignExpressions(AFileType.myAssignExpressions),
		myExplainTypes(AFileType.myExplainTypes),
                myDescription(AFileType.myDescription),
                myKeyWords(AFileType.myKeyWords),
                myFileTypeName(AFileType.myFileTypeName)
{
}




//------------------------------------------------- Reporting Functions ------------------------------------
size_t DataFile::KeyWordCount() const
{
	return(myKeyWords.size());
}

size_t DataFile::PatternExplainCount() const
{
	return(myPatternExplains.size());
}

string DataFile::GiveKeyWord(const size_t &RefNum) const
{
	try
	{
		return(myKeyWords.at(RefNum));
	}
	catch (...)
	{
		cerr << "ERROR: Invalid reference number for KeyWords: " << RefNum << "  myKeyWords.size(): " << myKeyWords.size() << endl;
		return("");
	}
}

vector <string> DataFile::GiveAllKeyWords() const
{
	return(myKeyWords);
}

vector <string> DataFile::GiveAllPatternExplains() const
{
	return(myPatternExplains);
}

vector <string> DataFile::GiveAllAssignExpressions() const
{
	return(myAssignExpressions);
}

vector <string> DataFile::GiveAllExplainTypes() const
{
	return(myExplainTypes);
}

string DataFile::GiveFileTypeName() const
{
	return(myFileTypeName);
}

string DataFile::GiveSearchString() const
{
	return(mySearchString);
}

string DataFile::GivePatternExplain(const size_t &RefNum) const
{
	try
	{
		return(myPatternExplains.at(RefNum));
	}
	catch (...)
	{
		cerr << "ERROR: Invalid reference number for PatternExplains: " << RefNum 
		     << "  myPatternExplains.size(): " << myPatternExplains.size() << endl;
                return("");
	}
}

string DataFile::GiveAssignExpression(const size_t &RefNum) const
{
	try
	{
		return(myAssignExpressions.at(RefNum));
	}
	catch (...)
        {
                cerr << "ERROR: Invalid reference number for AssignExpressions: " << RefNum
                     << "  myAssignExpressions.size(): " << myAssignExpressions.size() << endl;
                return("");
        }

}

string DataFile::GiveExplainType(const size_t &RefNum) const
{
	try
        {
                return(myExplainTypes.at(RefNum));
        }
        catch (...)
        {
                cerr << "ERROR: Invalid reference number for ExplainTypes: " << RefNum
                     << "  myExplainTypes.size(): " << myExplainTypes.size() << endl;
                return("");
        }
}

string DataFile::GiveDescription() const
{
	return(myDescription);
}


//------------------------------------------------------------------------------------------------------------



//----------------------------------------------- Data Loading Functions -------------------------------------
bool DataFile::TakeFileTypeName(const string &StringLine)
{
	myFileTypeName = StringLine;
	return(true);
}

bool DataFile::TakeSearchString(const string &StringLine)
{
	mySearchString = StringLine;
	return(true);
}

bool DataFile::TakeWholePatternExplain(const string &StringLine)
{
	vector <string> Parts = TakeDelimitedList(StringLine, '=');
	if (Parts.size() == 2)
	{
		myPatternExplains.push_back(RipWhiteSpace(Parts[0]));
		myAssignExpressions.push_back(RipWhiteSpace(Parts[1]));
		myExplainTypes.push_back("");
		return(true);
	}
	else if (Parts.size() == 3)
	{
		myPatternExplains.push_back(RipWhiteSpace(Parts[0]));
                myAssignExpressions.push_back(RipWhiteSpace(Parts[1]));
                myExplainTypes.push_back(RipWhiteSpace(Parts[2]));
                return(true);
	}
	else
	{
		cerr << "ERROR: Something is wrong with the DataFile loading.  Here is the line: |" << StringLine << "|\n"
                     << "   Parts.size(): " << Parts.size() << endl;
                return(false);
	}
}

bool DataFile::TakeDescription(const string &StringLine)
{
	myDescription = StringLine;
	return(true);
}

bool DataFile::TakeKeyWordList(const string &StringLine)
{
	myKeyWords = TakeDelimitedList(StringLine, ',');
	StripWhiteSpace(myKeyWords);
	return(true);
}

//-----------------------------------------------------------------------------------------------------------

vector <string> DataFile::GiveDescriptors() const
{
	vector <string> Descriptors(0);

	Descriptors.push_back("DateTime_Info");

	for (vector<string>::const_iterator APattern = myPatternExplains.begin(); APattern != myPatternExplains.end(); APattern++)
	{
		// if the descriptor STARTS with a $Date or $Time, then it is part of the DateTime_Info descriptor, which has been included.
		// So, find all other descriptors.
		if ((APattern->find("$Date") != 0) && (APattern->find("$Time") != 0))
		{
			// Don't include the first character, the dollar sign...
			Descriptors.push_back(APattern->substr(1));
		}
	}

	return(Descriptors);
}

vector <string> DataFile::GiveDeclarations() const
{
	vector <string> Declarations(0);

	Declarations.push_back("DATETIME NOT NULL");

	for (vector<string>::const_iterator APattern( myPatternExplains.begin() ), AnExplainType( myExplainTypes.begin() );
	     APattern != myPatternExplains.end(); APattern++, AnExplainType++)
	{
		if ((APattern->find("$Date") != 0) && (APattern->find("$Time") != 0))
		{
			if (*AnExplainType != "")
			{
				Declarations.push_back(*AnExplainType);
			}
			else
			{
				cerr << "DataFile::GiveDeclarations() WARNING: Unspecified declaration type.  Using default of VARCHAR(255)..." << endl;
				Declarations.push_back("VARCHAR(255)");
			}
		}
	}

	return(Declarations);
}



// This is a friend function....
vector <string> ListAllDescriptors(const vector <DataFile> &Files)
{
	vector <string> DescriptorNames(0);

	DescriptorNames.push_back("DateTime_Info");

	for (vector<DataFile>::const_iterator AFile = Files.begin(); AFile != Files.end(); AFile++)
	{
		const vector <string> MiniList = AFile->GiveDescriptors();

		// I am starting at .begin() + 1 because all the datafiles have a DateTime_Info descriptor, which should be first...
		// There should always be at least one element available, so this increment by 1 should be ok.
		DescriptorNames.insert(DescriptorNames.end(), MiniList.begin() + 1, MiniList.end());
	}

	return(DescriptorNames);
}
//----------------------------------------------------------------------------------------------------------

//---------------------------------------- neglected functions, I haven't bothered with updating these -------------------

void DataFile::PrintDescription() const
// Prints out the description string in a nice and neat fashion, supposedly.
// It is a pretty dumb algorithm.  Oh, well...
{
	size_t NeatnessCounter = 0;
	for (size_t i = 0; i < myDescription.length(); i++)
	{
		if (NeatnessCounter == 80)
		{
			// Time to end this line abruptly.  No hypens or anything here
			cout << myDescription[i] << "\n\t";
			NeatnessCounter = 0;
		}
		else if ((NeatnessCounter >= 60) && isspace(myDescription[i]))
		{
			// I found a good spot to stop this line and move to the next line.
			cout << "\n\t";
			NeatnessCounter = 0;
		}
		else
		{
			// just print another letter
			cout << myDescription[i];
			NeatnessCounter++;
		}
	}
}


void DataFile::ReportInfo() const
{
	cout << "File Type: " << myFileTypeName << endl;
	cout << "Basic Search String\t\t\tPattern Explaination\n";
	
	cout << mySearchString << endl;
	
	cout << myFileTypeName << " Description:\n\t";
	PrintDescription();
	cout << "\n\n";
	cout << "Key words associated with " << myFileTypeName << ": ";

	int NeatnessCounter = 0;
	for (size_t i = 0; i < myKeyWords.size(); i++)
	{
		cout << myKeyWords[i];
		
		if (i != (myKeyWords.size() - 1))
		{
			cout << ", ";
		}

		if (NeatnessCounter == 5)
		{
			cout << "\n\t";
			NeatnessCounter = 0;
		}
		else
		{
			NeatnessCounter++;
		}

	}
	
	cout << endl;
}
//------------------------- end neglected functions ----------------------------------------------


// ---------------------------------- config loading and saving -----------------------------
bool DataFile::GetEntryInfo(ifstream &ReadData)
{
	if (ReadData.eof())
	{
		cerr << "ERROR: Premature end to the config file in a DataFile entry..." << endl;
		return(false);
	}

	string FileLine = ReadNoComments(ReadData);
	const vector <string> EntryTagWords = InitTagWords();
	bool BadObject = false;

	while ( !ReadData.eof() && !FoundEndTag(FileLine, EntryTagWords[0]) )	// </Data>
	{
		if (!BadObject)
		{
			if (FoundStartTag(FileLine, EntryTagWords[1]))		// <DataTypeName>
			{
				FileLine = ReadNoComments(ReadData);

				if ( !FoundEndTag(FileLine, EntryTagWords[1]) );
	                        {
        	                        if (!TakeFileTypeName(FileLine))
					{
						BadObject = true;
					}
                	        }

				FileLine = ReadNoComments(ReadData);	// reads the end tag
			}
			else if (FoundStartTag(FileLine, EntryTagWords[2]))	// <Reg_ex>
			{
				FileLine = ReadNoComments(ReadData);

				if ( !FoundEndTag(FileLine, EntryTagWords[2]) )
	                        {
        	                        if (!TakeSearchString(FileLine))
					{
						BadObject = true;
					}
                	        }

				FileLine = ReadNoComments(ReadData);		// reads the end tag
			}
			else if (FoundStartTag(FileLine, EntryTagWords[3]))     // <Pattern_Explain>
                        {
                                FileLine = ReadNoComments(ReadData);

                                while ( !FoundEndTag(FileLine, EntryTagWords[3]) )
                                {
                                        if (!TakeWholePatternExplain(FileLine))
					{
						BadObject = true;
					}

					FileLine = ReadNoComments(ReadData);
                                }
                        }
			else if (FoundStartTag(FileLine, EntryTagWords[4]))     // <Description>
                        {
                                FileLine = ReadNoComments(ReadData);
				string Builder = "";

                                while ( !FoundEndTag(FileLine, EntryTagWords[4]) )
                                {
					Builder += (RipWhiteSpace(FileLine) + ' ');
					FileLine = ReadNoComments(ReadData);
				}

				if (!TakeDescription(RipWhiteSpace(Builder)))
				{
					BadObject = true;
				}
                        }
			else if (FoundStartTag(FileLine, EntryTagWords[5]))        // <Key_Words>
	                {
        	                FileLine = ReadNoComments(ReadData);
                	        string Builder = "";

                        	while ( !FoundEndTag(FileLine, EntryTagWords[5]) )
	                        {
        	                        Builder += RipWhiteSpace(FileLine);
                	                FileLine = ReadNoComments(ReadData);
                        	}

	                        if (!TakeKeyWordList(RipWhiteSpace(Builder)))
				{
					BadObject = true;
				}
                	}
			else
			{
				cerr << "ERROR: Unknown information!  Here is the line: " << FileLine << endl;
				BadObject = true;
			}
		}// end if !BadObject

		FileLine = ReadNoComments(ReadData);
	}// end while loop

	if (ReadData.eof() && !FoundEndTag(FileLine, EntryTagWords[0]))
	{
		cerr << "ERROR: Appears to be a premature end to the DataFile entry in the config file..." << endl;
		BadObject = true;
	}

	return(!BadObject);
}

bool DataFile::WriteEntryInfo(ofstream &WriteData, const size_t BaseIndent = 1) const
{
	const string BaseSpace(3 * BaseIndent, ' ');
	const vector <string> TagWords = InitTagWords();

	WriteData << BaseSpace << '<' << TagWords[0] << ">\n";

	WriteData << BaseSpace << "   " << '<' << TagWords[1] << ">\n";
	WriteData << BaseSpace << "      " << myFileTypeName << "\n";
	WriteData << BaseSpace << "   " << "</" << TagWords[1] << ">\n";

	WriteData << BaseSpace << "   " << '<' << TagWords[2] << ">\n";
        WriteData << BaseSpace << "      " << mySearchString << "\n";
        WriteData << BaseSpace << "   " << "</" << TagWords[2] << ">\n";

	WriteData << BaseSpace << "   " << '<' << TagWords[3] << ">\n";
	for (size_t Index = 0; Index < myPatternExplains.size(); Index++)
	{
		WriteData << BaseSpace << "      " << myPatternExplains[Index] << " = " << myAssignExpressions[Index];

		if (!myExplainTypes[Index].empty())
		{
			WriteData << " = " << myExplainTypes[Index];
		}

		WriteData << "\n";
	}
        WriteData << BaseSpace << "   " << "</" << TagWords[3] << ">\n";

	WriteData << BaseSpace << "   " << '<' << TagWords[4] << ">\n";
        WriteData << BaseSpace << "      " << myDescription << "\n";
        WriteData << BaseSpace << "   " << "</" << TagWords[4] << ">\n";

	WriteData << BaseSpace << "   " << '<' << TagWords[5] << ">\n";
        WriteData << BaseSpace << "      " << GiveDelimitedList(myKeyWords, ',') << "\n";
        WriteData << BaseSpace << "   " << "</" << TagWords[5] << ">\n";

	WriteData << BaseSpace << "</" << TagWords[0] << ">\n";

	return(WriteData.good());
}


vector <string> DataFile::InitTagWords() const
{
	vector <string> TheTagWords(6);

	TheTagWords[0] = "Data";
	TheTagWords[1] = "DataTypeName";
	TheTagWords[2] = "Reg_ex";
	TheTagWords[3] = "Pattern_Explain";
	TheTagWords[4] = "Description";
	TheTagWords[5] = "Key_words";

	return(TheTagWords);
}

//---------------------------------------------------------------------------------------------------

#endif

