#ifndef _DEAFUTIL_C
#define _DEAFUTIL_C
using namespace std;

#include <iostream>
#include <string>
#include <cstdlib>      // for getenv()
#include <list>

#include <sys/types.h>	// for DIR
#include <sys/stat.h>	// for stat, mkdir()
#include <dirent.h>
#include <errno.h>

#include <boost/regex.hpp>

#include <mysql++/mysql++.h>
#include <unistd.h>		// for getpass()

#include <deafUtil.h>


bool EstablishConnection(mysqlpp::Connection &Connect_A, const string &HostName, const string &UserName, const unsigned int PortNum)
{
	char* ThePass = getpass(("Enter MySQL password for user " + UserName + " on host " + HostName + ": ").c_str());

	if (ThePass == NULL)
	{
		cerr << "ERROR: Failure obtaining password..." << endl;
		return(false);
	}

	try
	{
		Connect_A.connect("", HostName.c_str(), UserName.c_str(), ThePass, PortNum);
	}
	catch (...)
	{
		while (*ThePass != '\0')
		{
			*ThePass++ = '\0';
		}

		throw;
	}

	while (*ThePass != '\0')
	{
		*ThePass++ = '\0';
	}
		

	return(true);
}

bool EstablishConnection(mysqlpp::Connection &Connect_A, mysqlpp::Connection &Connect_B, 
			 const string &HostName, const string &UserName, const unsigned int PortNum)
{
        char* ThePass = getpass(("Enter MySQL password for user " + UserName + " on host " + HostName + ": ").c_str());

        if (ThePass == NULL)
        {
                cerr << "ERROR: Failure obtaining password..." << endl;
                return(false);
        }

        try
        {
                Connect_A.connect("", HostName.c_str(), UserName.c_str(), ThePass, PortNum);
		Connect_B.connect("", HostName.c_str(), UserName.c_str(), ThePass, PortNum);
        }
        catch (...)
        {
                while (*ThePass != '\0')
                {
                        *ThePass++ = '0';
                }

                throw;
        }


	while (*ThePass != '\0')
	{
		*ThePass++ = '0';
	}

	return(true);
}




list <string> FindFileMatches(const string &BasePath, const string &DirName, boost::regex &FilenameRegEx)
/* This function can throw exceptions, so always wrap your call with a try/catch.
  BasePath is the base pathname where this function is being called from -- the starting point, if you will.
  DirName is the directory name that is being searched (without the BasePath name attached).
  FilenameRegEx is the boost regular expression that will be used to find any filename matches.
  The comparison is done against the string built from concatenating DirName, the forward slash,
  and the filename or just the filename if DirName is empty.
  A string list of matching filenames is returned.
  This function is recursive, and currently has no way of recognizing if it is in an infinite recursion.

  Also, while this function will do match-checking against hidden files, it will not enter into hidden directories.
*/
{
	errno = 0;
	DIR* DirPointer;

	if ((DirPointer = opendir((BasePath + '/' + DirName).c_str())) == NULL)
	{
		cerr << "FindFileMatches(): Could not open directory " + BasePath + '/' + DirName + "\n";
		throw(errno);		
	}

	struct dirent* EntryPointer;
	struct stat StatBuff;

	list <string> FileNames(0);
	errno = 0;

	while ((EntryPointer = readdir(DirPointer)) != NULL)
	{
		if (strcmp("..", EntryPointer->d_name) == 0 || strcmp(".", EntryPointer->d_name) == 0)
		{
			// If you don't ignore these directories, you will end up in an
			// infinite recursion.
			continue;
		}

		const string FullName = (DirName == "" ? EntryPointer->d_name : DirName + '/' + EntryPointer->d_name);

		if (stat((BasePath + '/' + FullName).c_str(), &StatBuff) == -1)
		{
			closedir(DirPointer);
			throw("FindFileMatches(): Could not stat " + BasePath + '/' + FullName);
		}

		if (regex_match(FullName, FilenameRegEx))
		{
			FileNames.push_back(FullName);
		}

		if (S_ISDIR(StatBuff.st_mode) && 			// If it is a directory
		    ((string) EntryPointer->d_name)[0] != '.')         	// If it isn't a hidden directory
		{
			// try a partial match to see if the directory is worth pursuing.
			// note that this partial match algorithm tries to see if the entire string matches
			// with parts of the regex.  In other words, not all of the regex was a positve match.
			if (regex_match(FullName, FilenameRegEx, boost::match_default | boost::match_partial))
			{
				try
				{
					list <string> MoreFiles = FindFileMatches(BasePath, FullName, FilenameRegEx);
					FileNames.splice(FileNames.end(), MoreFiles);
				}
				catch (int &ErrorNum)
				{
					// It really isn't an error if the file isn't readable during recursive search.
					// just skip it like it isn't there.
					if (ErrorNum != EACCES)
					{
						perror((BasePath + '/' + FullName).c_str());
						closedir(DirPointer);
						throw("FindFileMatches(): Problems searching directory " + BasePath + '/' + FullName + '\n');
					}
					else
					{
						errno = 0;
					}
				}
				catch (...)
				{
					closedir(DirPointer);
					throw;
				}
			}// end if partial match
		}// end if directory

		// Might need to add support for symbolic links, don't know...		
	}// end while loop for reading directory contents

	if (errno != 0)
        {
		closedir(DirPointer);
                throw("FindFileMatches(): Problem reading directory: " + BasePath + '/' + DirName);
        }


	if (closedir(DirPointer) != 0)
	{
		cerr << "ERROR: Problem closing directory: " + BasePath + '/' + DirName << endl;
		perror((BasePath + '/' + DirName).c_str());
	}

	errno = 0;
	return(FileNames);
}


vector <string> GetDeafDirs()
{
	char* TempHold = getenv("DEAF_HOME");

	vector <string> DeafDirs(0);

        if (TempHold == NULL)
        {
                cerr << "WARNING!  DEAF_HOME environment variable is not set.  Using '.'" << endl;
                DeafDirs.push_back(".");
		return(DeafDirs);
        }

        char* EndPoint = TempHold;
	bool IsEscaped = false;
		
	while (*EndPoint != '\0')
	{
		if (*EndPoint == ':')
		{
			if (IsEscaped) 
			{
				IsEscaped = false;
			}
			else
			{
				string ADir(TempHold, EndPoint - TempHold);
				DeafDirs.push_back(ADir);
				TempHold = EndPoint + 1;
			}
		}
		else if (*EndPoint == '\\')
		{
			IsEscaped = !IsEscaped;
		}
		else if (IsEscaped)
		{
			// There should be no other reason for an escape...
			cerr << "ERROR!  Bad value for DEAF_HOME environment variable: " << TempHold << endl;
			break;
		}
				
		EndPoint++;
	}

	if (TempHold != EndPoint)
	{
		string ADir(TempHold, EndPoint - TempHold);
                DeafDirs.push_back(ADir);
	}

	for (vector<string>::iterator ADir = DeafDirs.begin(); ADir != DeafDirs.end(); ADir++)
	{
		while (ADir->find("\\\\") != string::npos)
		{
			string::iterator StartPos = ADir->begin() + ADir->find("\\\\");
			ADir->replace(StartPos, StartPos + 2, "\\");
		}

		while (ADir->find("\\:") != string::npos)
                {
                        string::iterator StartPos = ADir->begin() + ADir->find("\\:");
                        ADir->replace(StartPos, StartPos + 2, ":");
                }
	}

	return(DeafDirs);
}


bool MakeDirectory(const string &DirectoryPath)
// Returns false if the directory does not exist by the time
// the function finishes.  In other words, this function
// returns true if the directory could be made, or if the
// directory already existed.
// This function uses a recursive algorithm to make all
// directories in the filepath, as needed.
{
	if (DirectoryPath.empty())
	{
		cerr << "ERROR::MakeDirectory(): No directory specified!\n";
		return(false);
	}

	
	const int OldErrno = errno;
	errno = 0;

	// Check to see if DirectoryPath already exists...
	if (access(DirectoryPath.c_str(), F_OK) != 0)
	{
		// Check to see if there is a filepath.  Also don't care if it is root directory.
		if (DirectoryPath.rfind('/') != string::npos && DirectoryPath.rfind('/') != 0)
		{
			// Make sure that the filepath exists before trying to make DirectoryPath
			if (!MakeDirectory(DirectoryPath.substr(0, DirectoryPath.rfind('/'))))
			{
				errno = OldErrno;
				return(false);
			}
		}

		// If execution reaches here, then the directories in the filepath of DirectoryPath exists.


		// This directory does not exist.  Make it!
		if (mkdir(DirectoryPath.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0)
		{
			// Directory-making failed.  Print error message and return false.
			perror(DirectoryPath.c_str());
			errno = OldErrno;
			return(false);
		}
	}

	// At this point, DirectoryPath exists!	
	errno = OldErrno;
	return(true);
}


#endif
