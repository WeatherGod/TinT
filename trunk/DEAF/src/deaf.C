using namespace std;

#include <iostream>
#include <list>
#include <vector>
#include <string>

#include <unistd.h>             // for optarg, opterr, optopt, access()
#include <getopt.h>             // for getopt_long()

#include <csignal>              // for some signal handling.  I am very weak in this area, but this should be sufficient.
#include <setjmp.h>

#include <mysql++/mysql++.h>
#include <boost/regex.hpp>

#include "DeafPlugin.h"
#include "DeafBean.h"
#include "DeafInfo_t.h"		// for deafInfo_t class

#include "DeafConf.h"		// for DeafConf_t class

#include "deafUtil.h"		// for FindFileMatches(), GetDeafDirs(), EstablishConnection(), MakeDirectory()

/* Compile notes:
	For a plugin:
		gcc -fPIC -c TestPlugin.C -o TestPlugin.o -I /usr/include/mysql
		gcc -fPIC -c TestBean.C -o TestBean.o
		gcc -shared TestPlugin.o TestBean.o -o libTestPlugin.so -L /home/meteo/bvroot/usr/lib -l mysqlpp -l stdc++

	For the Deaf server:
		gcc -rdynamic deaf.C -o deaf -I /usr/include/mysql -L ./ -L /usr/lib/mysql -L /home/meteo/bvroot/usr/lib -l DEAF -l boost_regex -l xml++-2.6 -l mysqlpp -l stdc++ -l dl 

   Resources:
	http://www.isotton.com/howtos/C++-dlopen-mini-HOWTO/C++-dlopen-mini-HOWTO.html
*/




//-------------------------------- Bean Cooker -----------------------------------------------------------
// This bean cooker stuff should eventually be moved to a separate file.
typedef bool beancooker_func(DeafBean* &, destroy_bean_t*, DeafBean* &, create_bean_t*);
bool NullBeanCooker(DeafBean* &InputBean, destroy_bean_t* InputBeanDestroyer, 
		    DeafBean* &OutputBean, create_bean_t* OutputBeanCreator)
{
	// I don't use the two function pointers (InputBeanDestroyer and OutputBeanCreator)
	// because this NullBeanCooker is used when the two Beans are the same type.
	// So, for efficiency sake's, don't bother copying data from one place to another,
	// just make the OutputBean point to the InputBean.

	// Other cookers (Not implemented yet) will need to create the OutputBean
	// and destroy the InputBean.
	OutputBean = InputBean;
	return(true);
}
//-------------------------------------------------------------------------------------------------------


/*  SigHandle will be the function called for SIGINT signals.
    It outputs a message saying that the program is in the process of shutting down.
    Then, it will set a flag to true.  Execution will then resume where it left off before the SIGINT.
    At strategic points in the code, there are checks for the flag.  If it is true at that point, an exception
    will be thrown, triggering a safe shutdown of the program.
*/
void SigHandle(int SigNum);
void StuckPlugin(int SigNum);
jmp_buf JumperBuf;
bool SIGNAL_CAUGHT = false;             // flag for indicating that a signal was caught and it is time to clean up!



void PrintSyntax()
{
	cerr << "deaf [--config | -c CONFIGFILE] [--verbose | -v]\n"
	     << "       --input | -i INPUTFUNC     --output | -o OUTPUTFUNC\n";
	cerr << "     [--source | -s SOURCENAME]    [--dest | -d DESTNAME]\n";
	cerr << "     [--test | -t] [--flush | -f]\n";
	cerr << "     [-l[PLUGIN] | --list [PLUGIN]]\n";
	cerr << "     [--syntax | -x] [--help | -h]\n\n";
}

void PrintHelp()
{
	PrintSyntax();

	cerr << "This program is a plugin interface.  It is responsible for establishing any database connections,\n";
	cerr << "filesystem searches and stream connections for any registered deaf plugin.\n\n";
	cerr << "The input and the output functions are named in the following format:\n";
	cerr << "\tPluginName::FunctionName\n";
	cerr << "Available function names are --\n";
	cerr << "\tINPUT ACTION:\t\t\tOUTPUT ACTION:\n";
	cerr << "\tFrom_MySQL\t\t\tTo_MySQL\n";
	cerr << "\tFrom_File\t\t\tTo_File\n";
	cerr << "\tFrom_Stream\t\t\tTo_Stream\n\n\n";

	cerr << "-v  --verbose\n"
	     << "Increase verbosity.  Repeat as needed.\n\n";

	cerr << "-c  --config  CONFIGFILE\n";
	cerr << "By default, this program will use the configuration file installed on this system,\n";
	cerr << "however, you can override that config file with your own.\n\n";

	cerr << "-l  --list  [PLUGIN]\n"
	     << "List the available plugins, or specify a specific plugin for more details.\n\n";

	cerr << "-t  --test\n"
	     << "Execute deaf in the test mode.  The same steps are taken,\n"
	     << "but the data is never processed.\n\n";

	cerr << "-f  --flush\n"
	     << "Execute deaf in the force-flush mode.  If there is an error while\n"
	     << "reading a data source, deaf will still attempt to output the data\n"
	     << "bean to the data destination.\n\n\n";
}


void SuddenShutdown(deafInfo_t &In_Info, DeafPlugin* &In_Plugin_Ptr, DeafBean* &In_Bean_Ptr,
                    deafInfo_t &Out_Info, DeafPlugin* &Out_Plugin_Ptr, DeafBean* &Out_Bean_Ptr, const bool SamePlugin);

void PopulateSources(const DeafConf_t &DeafConf, 
		     const string &InPluginName, const string &OutPluginName, 
		     const string &InputFunc, const string &OutputFunc,
		     const string &Source, const string &Destination,
		     mysqlpp::Connection &InputConnect, mysqlpp::Connection &OutputConnect,
		     list<string> &InputSources, list<string> &OutputSources);

void TestAndLoadPlugins(const DeafConf_t &DeafConf,
                        const string &InPluginName, const string &OutPluginName, 
			bool SamePlugin, bool SameBean,
                        deafInfo_t &InPlugin_Info, deafInfo_t &OutPlugin_Info,
                        DeafPlugin* &InUserPlugin_Ptr, DeafPlugin* &OutUserPlugin_Ptr);

void DoInput(const string &InputFunc, DeafPlugin* &InUserPlugin_Ptr, DeafBean* &InPluginBean_Ptr,
             const string &SourceLoc, const string &A_Source, mysqlpp::Connection &InputConnect);

void DoOutput(const string &OutputFunc, DeafPlugin* &OutUserPlugin_Ptr, DeafBean* &OutPluginBean_Ptr,
              const string &DestinationLoc, const string &A_Dest, mysqlpp::Connection &OutputConnect);




int main(int argc, char* argv[])
{
	string ConfigFilename = "";

	string InPluginName = "";
	string OutPluginName = "";
	string InputFunc = "";
	string OutputFunc = "";
	int IsVerbose = 0;
	bool ForceFlush = false;
	bool DoTest = false;
	bool DoList = false;

	string PluginToList = "";

	string InputArg = "";
	string OutputArg = "";
	string Source = "";
	string Destination = "";

	bool SamePlugin = false;
	bool SameBean = false;

	int OptionIndex = 0;
        int OptionChar = 0;
        bool OptionError = false;
        opterr = 0;                     // don't print out error messages, let me handle that.

        static struct option TheLongOptions[] = {
                {"config", 1, NULL, 'c'},
		{"test", 0, NULL, 't'},
		{"list", 2, NULL, 'l'},
                {"input", 1, NULL, 'i'},
                {"output", 1, NULL, 'o'},
		{"source", 1, NULL, 's'},
		{"dest", 1, NULL, 'd'},
		{"verbose",0,NULL,'v'},
		{"flush",0,NULL,'f'},
                {"syntax", 0, NULL, 'x'},
                {"help", 0, NULL, 'h'},
                {0, 0, 0, 0}
        };

        while ((OptionChar = getopt_long(argc, argv, "c:tl::i:o:s:d:vfxh", TheLongOptions, &OptionIndex)) != -1)
        {
                switch (OptionChar)
                {
                case 'c':
                        ConfigFilename = optarg;
                        break;
		case 't':
			DoTest = true;
			break;
		case 'l':
			DoList = true;
			if (optarg != NULL)
			{
				PluginToList = optarg;
			}

			break;
                case 'i':
                        InputArg = optarg;
                        break;
                case 'o':
                        OutputArg = optarg;
                        break;
		case 's':
			Source = optarg;
			break;
		case 'd':
			Destination = optarg;
			break;
		case 'v':
			IsVerbose++;
			break;
		case 'f':
			ForceFlush = true;
			break;
                case 'x':
                        PrintSyntax();
                        return(1);
                        break;
                case 'h':
                        PrintHelp();
                        return(1);
                        break;
                case '?':
                        cerr << "ERROR: Unknown arguement: -" << (char) optopt << endl;
                        OptionError = true;
                        break;
                case ':':
                        cerr << "ERROR: Missing value for arguement: -" << (char) optopt << endl;
                        OptionError = true;
                        break;
                default:
                        cerr << "ERROR: Programming error... Unaccounted option: -" << (char) OptionChar << endl;
                        OptionError = true;
                        break;
                }
        }

        if (OptionError)
        {
                PrintSyntax();
                return(2);
        }

	if (!InputArg.empty())
	{
		if (InputArg.find("::") == string::npos || InputArg.find("::") == (InputArg.length() - 2))
		{
			cerr << "ERROR: Invalid syntax for input: " << InputArg << "\n"
			     << "     : Must follow format of _PLUGINNAME_::_ACTION_\n";
			PrintSyntax();
			return(3);
		}

		InPluginName = InputArg.substr(0, InputArg.find("::"));
		InputFunc = InputArg.substr(InputArg.find("::") + 2);
	}
	else if (!DoList)
	{
		cerr << "ERROR: Invalid syntax.  Missing input.\n";
		PrintSyntax();
		return(3);
	}


	if (!OutputArg.empty())
	{
		if (OutputArg.find("::") == string::npos || OutputArg.find("::") == (OutputArg.length() - 2))
                {
                        cerr << "ERROR: Invalid syntax for output: " << OutputArg << "\n"
                             << "     : Must follow format of _PLUGINNAME_::_ACTION_\n";
                        PrintSyntax();
                        return(3);
                }

		OutPluginName = OutputArg.substr(0, OutputArg.find("::"));
		OutputFunc = OutputArg.substr(OutputArg.find("::") + 2);
	}
	else if (!DoList)
        {
                cerr << "ERROR: Invalid syntax.  Missing output.\n";
                PrintSyntax();
                return(3);
        }


	DeafConf_t DeafConf;

	if (!ConfigFilename.empty())
	{
		/* Load deaf configurations.  This configuration file over-rides anything noted 
		   in any registration files.
	   	   The registration file's job is to fill in any missing information.
		   In order to over-ride other files, it must be loaded before any others.*/
		DeafConf.LoadConfiguration(ConfigFilename);

		if (!DeafConf.IsValid())
		{
			cerr << "ERROR: Invalid or missing deaf configuration file: " << ConfigFilename << endl;
			return(2);
		}
	}

	const vector <string> DeafDirs = GetDeafDirs();

	for (vector<string>::const_iterator ADir = DeafDirs.begin();
	     ADir != DeafDirs.end();
	     ADir++)
	{
		ConfigFilename = *ADir + "/deaf.config";

		// Load system configs, if it exists.
		if (access(ConfigFilename.c_str(), R_OK) == 0)
		{
			/* Load the system's deaf configurations.  This configuration file will supply 
			   any information that has yet been mentioned in the user-supplied configuration 
			   file. Nothing in this file will over-ride anything given by the user's config 
			   file. So, it must be loaded AFTER the user's config file.*/
			DeafConf.LoadConfiguration(ConfigFilename);

			if (!DeafConf.IsValid())
			{
				cerr << "ERROR: Invalid deaf system configuration file: " << ConfigFilename << "\n"
				     << "     : Please contact your system administrator to correct this file.\n";
				return(2);
			}
		}
	}

	// Now, all config files have been loaded, so we should know where all of the registration files are.


	if (DoList)
	{
		if (PluginToList.empty())
		{
			DeafConf.ListPlugins();
		}
		else
		{
			if (!DeafConf.ListPlugin(PluginToList, true))	// true for detailed
			{
				cerr << "ERROR: Could not list the plugin: " << PluginToList << endl;
				return(2);
			}
		}

		return(0);
	}



	if (!DeafConf.LoadRegistration(InPluginName))
	{
		cerr << "ERROR: Missing or invalid plugin registration file: "
		     << DeafConf.Give_RegistrationFilename(InPluginName) << endl;
		return(2);
	}

	if (!DeafConf.LoadRegistration(OutPluginName))
        {
	        cerr << "ERROR: Missing or invalid output plugin registration file: "
                     << DeafConf.Give_RegistrationFilename(OutPluginName) << endl;
                return(2);
        }


	if (IsVerbose > 1) cerr << "Registration file loaded...\n";

	// This check may not be necessary soon...
	if (InPluginName == OutPluginName)
	{
		SamePlugin = true;
		SameBean = true;
	}
	else
	{
		SamePlugin = false;

		if (DeafConf.Give_BeanName(InPluginName) == DeafConf.Give_BeanName(OutPluginName))
		{
			SameBean = true;
		}
		else
		{
			// Right now, it is an error.  Later, it will become a bean-cooker feature.
			cerr << "ERROR: This program can only support the use of a single bean at a time!\n";
			return(2);
		}
	}

	if (!DeafConf.Func_Avail(InPluginName, InputFunc))
	{
		cerr << "ERROR: input function, " << InputFunc << ", not available for plugin " << InPluginName << endl;
		return(2);
	}

	if (!DeafConf.Func_Avail(OutPluginName, OutputFunc))
	{
		cerr << "ERROR: output function, " << OutputFunc << ", not available for plugin " << OutPluginName << endl;
		return(2);
	}

	if (IsVerbose > 1) cerr << "Both functions are available!\n";

	mysqlpp::Connection InputConnect;
        mysqlpp::Connection OutputConnect;
        list <string> InputSources(0);
        list <string> OutputSources(0);

	try
	{
		// goes through and establishes all the information needed to do the end processing,
		// including file directory searches, and mysql connections.
		// It makes sure the databases exists, and the directories are available, and the streams are good.
		PopulateSources(DeafConf, InPluginName, OutPluginName, InputFunc, OutputFunc, Source, Destination,
				InputConnect, OutputConnect, InputSources, OutputSources);
	}
	catch (const mysqlpp::Exception &Err)
	{
		cerr << "ERROR: Mysqlpp exception caught.  " << Err.what() << endl;
		InputConnect.close();
		OutputConnect.close();
		return(2);
	}
/*	catch (const boost::regex_error &Err)
	{
		// Assumes that the regex string in question has been printed out already.
		cerr << (string(Err.position(), ' ')) << '^' << endl;
                cerr << "ERROR: Trouble compiling reg_ex.  Error Code: " << Err.code() << endl;
		InputConnect.close();
		OutputConnect.close();
		return(2);
	}
*/
	catch (const exception &Err)
	{
		cerr << "ERROR: Exception caught.  " << Err.what() << endl;
		InputConnect.close();
		OutputConnect.close();
		return(2);
	}
	catch (const string &ErrStr)
	{
		cerr << "ERROR: Exception caught.  " << ErrStr << endl;
		InputConnect.close();
		OutputConnect.close();
		return(2);
	}
	catch (...)
	{
		cerr << "ERROR: Unknown exception caught..." << endl;
		InputConnect.close();
		OutputConnect.close();
		return(3);
	}

	if (IsVerbose > 0)
	{
		size_t SourceMaxSize(0), DestMaxSize(0);
		for (list<string>::const_iterator An_In_Item(InputSources.begin()), An_Out_Item(OutputSources.begin());
                     An_In_Item != InputSources.end();
                     An_In_Item++, An_Out_Item++)
                {
			if (An_In_Item->length() > SourceMaxSize)
			{
				SourceMaxSize = An_In_Item->length();
			}

			if (An_Out_Item->length() > DestMaxSize)
			{
				DestMaxSize = An_Out_Item->length();
			}
		}

		fprintf(stderr, "\n%.*s\t %.*s\n", SourceMaxSize, "Source:", DestMaxSize, "Destination:");

		for (list<string>::const_iterator An_In_Item(InputSources.begin()), An_Out_Item(OutputSources.begin());
		     An_In_Item != InputSources.end();
		     An_In_Item++, An_Out_Item++)
		{
			fprintf(stderr, "%.*s\t %.*s\n", SourceMaxSize, An_In_Item->c_str(), DestMaxSize, An_Out_Item->c_str());
		}
		cerr << endl;
	}



	int ReturnVal = 0;
	deafInfo_t InPlugin_Info;
	deafInfo_t OutPlugin_Info;
	DeafPlugin* InUserPlugin_Ptr( NULL );
	DeafPlugin* OutUserPlugin_Ptr( NULL );
	DeafBean* InPluginBean_Ptr( NULL );
	DeafBean* OutPluginBean_Ptr( NULL );

	// Eventually, a BeanCooker function can somehow be selected
	// and that function will be loaded. and assigned here.
	// Right now, NullBeanCooker is the default, as it just assigns
	// the output bean pointer the address of the input bean.
	beancooker_func* TheBeanCooker = NullBeanCooker;
	

	try
	{
		
		signal(SIGINT, SigHandle);		// At this point, the SigHandle function will be called when SIGINT is sent.

		TestAndLoadPlugins(DeafConf, InPluginName, OutPluginName, 
				   SamePlugin, SameBean,
				   InPlugin_Info, OutPlugin_Info,
				   InUserPlugin_Ptr, OutUserPlugin_Ptr);

		if (setjmp(JumperBuf) != 0) { throw(0);}        // Execution will return to here when a double signal is issued, then the
                                                                // exception will be thrown, effectively shutting everything down.
                                                                // Note that any files open inside the plugin cannot be closed, sorry!

		if (IsVerbose > 1) cerr << "Plugins loaded successfully!\n";
		

		// The size of the two lists should be the same.  If not... oops!
		for (list<string>::const_iterator A_Source( InputSources.begin() ), A_Dest( OutputSources.begin() );
		     A_Source != InputSources.end() && !DoTest;
		     A_Source++, A_Dest++)
		{
			if (IsVerbose > 1) cout << "Source: " << *A_Source << endl;

			InPluginBean_Ptr = InPlugin_Info.BeanFactory();

			if (IsVerbose > 2) cerr << "Grew the bean...\n";

			try
			{
				DoInput(InputFunc, InUserPlugin_Ptr, InPluginBean_Ptr, Source, *A_Source, InputConnect);
			}
			catch (const string &ErrStr)
                        {
				if (ErrStr.find("Unknown Input function: ") != string::npos)
				{
					InPlugin_Info.BeanDestructor(InPluginBean_Ptr);
                                        throw(ErrStr);
				}
				

				cerr << "[ERROR] Exception caught: " << ErrStr << endl
                                     << "        Could not complete processing of the source.\n";

				if (!ForceFlush)
                                {
				        cerr << "        Throwing out the bean and moving onto the next source.\n\n";
                                        InPlugin_Info.BeanDestructor(InPluginBean_Ptr);
                                        continue;
                                }

                                cerr << "        Will attempt to flush the contents of the bean to the output.\n\n";
                        }
			catch (const exception &Err)
                        {
				cerr << "[ERROR] Exception caught: " << Err.what() << endl
                                     << "        Could not complete processing of the source.\n";

                                if (!ForceFlush)
                                {
					cerr << "        Throwing out the bean and moving onto the next source.\n\n";
                                        InPlugin_Info.BeanDestructor(InPluginBean_Ptr);
                                        continue;
                                }

                                cerr << "        Will attempt to flush the contents of the bean to the output.\n\n";
                        }
			catch (...)
			{
				cerr << "[ERROR] Unknown exception caught. " << endl
                                     << "        Could not complete processing of the source.\n";

				if (!ForceFlush)
				{
					cerr << "        Throwing out the bean and moving onto the next source.\n\n";
					InPlugin_Info.BeanDestructor(InPluginBean_Ptr);
					continue;
				}

				cerr << "        Will attempt to flush the contents of the bean to the output.\n\n";
			}

			// I have a catch statement ready to receive an int and recognize 0 as a non-error exception.
			if (SIGNAL_CAUGHT) { throw(0); }
			

			if (!TheBeanCooker(InPluginBean_Ptr, InPlugin_Info.Give_BeanDestructor(), 
					   OutPluginBean_Ptr, OutPlugin_Info.Give_BeanFactory()))
			{
				throw((string) "Could not cook the beans!");
			}
			// The data pointed to by both pointers will definately be destroyed at the end of the loop.


			// I have a catch statement ready to receive an int and recognize 0 as a non-error exception.
                        if (SIGNAL_CAUGHT) { throw(0); }

			if (IsVerbose > 1) cout << "Destination: " << *A_Dest << endl;

			try
			{
				DoOutput(OutputFunc, OutUserPlugin_Ptr, OutPluginBean_Ptr, Destination, *A_Dest, OutputConnect);
			}
			catch (const string &ErrStr)
			{
				cerr << "[ERROR]: Exception caught: " << ErrStr << endl
				     << "         Could not finish outputing data to destination.\n";
				
				if (ErrStr.find("Unknown Output function: ") != string::npos)
				{
					if (InPluginBean_Ptr == OutPluginBean_Ptr)
		                        {
                		                OutPlugin_Info.BeanDestructor(OutPluginBean_Ptr);
                                		InPluginBean_Ptr = NULL;
                        		}
                        		else
                        		{
		                                OutPlugin_Info.BeanDestructor(OutPluginBean_Ptr);
                		                InPlugin_Info.BeanDestructor(InPluginBean_Ptr);
                        		}

					throw(ErrStr);
				}
				
				cerr << "         Moving onto next source.\n\n";
			}
			catch (const exception &Err)
			{
				cerr << "[ERROR]: Exception caught: " << Err.what() << "\n"
                                     << "         Could not finish outputing data to destination.\n"
                                     << "         Moving onto next source.\n\n";
			}
			catch (...)
			{
				cerr << "[ERROR]: Unknown exception caught\n"
				     << "         Could not finish outputing data to destination.\n"
				     << "         Moving onto next source.\n\n";
			}

			// All bean data should be destroyed by this point.
			if (InPluginBean_Ptr == OutPluginBean_Ptr)
			{
				OutPlugin_Info.BeanDestructor(OutPluginBean_Ptr);
				InPluginBean_Ptr = NULL;
			}
			else
			{
				OutPlugin_Info.BeanDestructor(OutPluginBean_Ptr);
				InPlugin_Info.BeanDestructor(InPluginBean_Ptr);
			}

			// I have a catch statement ready to receive an int and recognize 0 as a non-error exception.
                        if (SIGNAL_CAUGHT) { throw(0); }

		}// loop over sources and destinations.


//		cout << "Destroying and unloading..." << endl;
		if (InUserPlugin_Ptr == OutUserPlugin_Ptr)
		{
			OutPlugin_Info.PluginDestructor(OutUserPlugin_Ptr);
			InUserPlugin_Ptr = NULL;
		}
		else
		{
			OutPlugin_Info.PluginDestructor(OutUserPlugin_Ptr);
			InPlugin_Info.PluginDestructor(InUserPlugin_Ptr);
		}

		InPlugin_Info.UnloadPlugin();

		if (!SamePlugin)
		{
			OutPlugin_Info.UnloadPlugin();
		}
	}
	catch (const mysqlpp::Exception &Err)
	{
		cerr << "ERROR: mysqlpp exception caught: " << Err.what() << endl;
		ReturnVal = 2;
		SuddenShutdown(InPlugin_Info, InUserPlugin_Ptr, InPluginBean_Ptr,
			       OutPlugin_Info, OutUserPlugin_Ptr, OutPluginBean_Ptr, SamePlugin);
	}
/*	catch (const boost::regex_error &Err)
	{
		cerr << "ERROR: boost::regex exception caught.  Error Code: " << Err.code() << endl;
		ReturnVal = 2;
		SuddenShutdown(InPlugin_Info, InUserPlugin_Ptr, InPluginBean_Ptr,
                               OutPlugin_Info, OutUserPlugin_Ptr, OutPluginBean_Ptr, SamePlugin);
	}
*/
	catch (const exception &Err)
	{
		cerr << "ERROR: Exception caught: " << Err.what() << endl;
		ReturnVal = 2;
		SuddenShutdown(InPlugin_Info, InUserPlugin_Ptr, InPluginBean_Ptr,
                               OutPlugin_Info, OutUserPlugin_Ptr, OutPluginBean_Ptr, SamePlugin);
	}
	catch (const string &ErrStr)
	{
		cerr << "ERROR: Exception caught: " << ErrStr << endl;
		ReturnVal = 2;
		SuddenShutdown(InPlugin_Info, InUserPlugin_Ptr, InPluginBean_Ptr,
                               OutPlugin_Info, OutUserPlugin_Ptr, OutPluginBean_Ptr, SamePlugin);
	}
	catch (int ErrNum)
	{
		if (ErrNum == 0)
		{
			cerr << "Shutting down safely...\n";
		}
		else
		{
			cerr << "ERROR: exception caught.  Error Number: " << ErrNum << endl;
		}

		ReturnVal = ErrNum;
		SuddenShutdown(InPlugin_Info, InUserPlugin_Ptr, InPluginBean_Ptr,
			       OutPlugin_Info, OutUserPlugin_Ptr, OutPluginBean_Ptr, SamePlugin);
	}
	catch (...)
	{
		cerr << "ERROR: Unknown exception caught..." << endl;
		ReturnVal = 3;
		SuddenShutdown(InPlugin_Info, InUserPlugin_Ptr, InPluginBean_Ptr,
                               OutPlugin_Info, OutUserPlugin_Ptr, OutPluginBean_Ptr, SamePlugin);
	}


	if (IsVerbose > 1) cerr << "Closing Input connection..." << endl;
	InputConnect.close();
	if (IsVerbose > 1) cerr << "Closing output connection..." << endl;
	OutputConnect.close();

	return(ReturnVal);
}


// ------------------ Additional functions ------------------------------------------------------------------------

//*******************************************************
//-------------------  SuddenShutdown()  ----------------
//*******************************************************
void SuddenShutdown(deafInfo_t &In_Info, DeafPlugin* &In_Plugin_Ptr, DeafBean* &In_Bean_Ptr,
		    deafInfo_t &Out_Info, DeafPlugin* &Out_Plugin_Ptr, DeafBean* &Out_Bean_Ptr, const bool SamePlugin)
{
//	cerr << "Destroying Beans...\n";
	if (In_Bean_Ptr == Out_Bean_Ptr)
	{
//		cerr << "Same beans...\n";
		In_Info.BeanDestructor(In_Bean_Ptr);
		Out_Bean_Ptr = NULL;
	}
	else
	{
//		cerr << "Different beans...\n";
		In_Info.BeanDestructor(In_Bean_Ptr);
		Out_Info.BeanDestructor(Out_Bean_Ptr);
	}

//	cerr << "Destroying Plugins...\n";

	if (In_Plugin_Ptr == Out_Plugin_Ptr)
	{
//		cerr << "Same plugins...\n";
		In_Info.PluginDestructor(In_Plugin_Ptr);
		Out_Plugin_Ptr = NULL;
	}
	else
	{
//		cerr << "Different plugins...\n";
		In_Info.PluginDestructor(In_Plugin_Ptr);
		Out_Info.PluginDestructor(Out_Plugin_Ptr);
	}

//	cerr << "Unloading plugins...\n";

	In_Info.UnloadPlugin();

//	cerr << "Done input...\n";

	if (!SamePlugin)
	{
		Out_Info.UnloadPlugin();
	}

//	cerr << "Done!\n";
}



//*****************************************
//-------------- SigHandle() --------------
//*****************************************
void SigHandle(int SigNum)
/* This will disable the signal associated with SigNum, print out a message, set SIGNAL_CAUGHT = true, and then send execution back to the code.
   at the bottom of the loops going through the list, there are if-statements that checks SIGNAL_CAUGHT, and throws an error number if it is true.
   The idea behind this is that the SIGINT could happen at any time.
   Therefore, the code must be allowed to resume to finish whatever it was doing, and then allow the program to pick 
   a safe time to shutdown the system without any fears.

   It would be nice to be able to know when even signals like SIGKILL are sent, so that I may possibly get a chance to just finish
   whatever is being done at the moment.
*/
{
        signal(SigNum, StuckPlugin);
        cerr << "\n--- Signal caught ----\n";
        cerr << "Proceeding to shut down deaf safely.  Please re-use that signal if you think the plugin is stuck...\n";
        SIGNAL_CAUGHT = true;
}

void StuckPlugin(int SigNum)
{
	signal(SigNum, SIG_IGN);
	cerr << "\n--- Signal caught ---\nForcibly shutting down the plugins...\n";
	SIGNAL_CAUGHT = true;
	longjmp(JumperBuf, 1);	// jumps to the last call of setjmp()
}


//***************************************************
//---------------  PopulateSources()  ---------------
//***************************************************
void PopulateSources(const DeafConf_t &DeafConf,
                     const string &InPluginName, const string &OutPluginName,
                     const string &InputFunc, const string &OutputFunc,
		     const string &Source, const string &Destination,
                     mysqlpp::Connection &InputConnect, mysqlpp::Connection &OutputConnect,
                     list<string> &InputSources, list<string> &OutputSources)
{
	// Start with the input information...
	if (InputFunc == "From_MySQL")
       	{
		// Host and user names come from the registration file, unless overridden by the configuration file
		string InHost = DeafConf.Give_InputHost( InPluginName );
		string InUser = DeafConf.Give_InputUser( InPluginName );
		unsigned int InPort = DeafConf.Give_InputHostPort( InPluginName );

		try
		{
			if (OutputFunc == "To_MySQL" && InHost == DeafConf.Give_OutputHost( OutPluginName ) &&
			    InUser == DeafConf.Give_OutputUser( OutPluginName ))
			{
				// Because both the input and the output actions are MySQL related, and the
				// username and the hostnames are the same, connect to both at the same time.
				// Assume that the same port number is used for both of them.
                        	if (!EstablishConnection(InputConnect, OutputConnect, InHost, InUser, InPort))
				{
					throw((string) "Could not establish input/output mysql connections.");
				}
       	                }
			else
			{
				// There are differences between the two connections or the output action
				// is not MySQL related, so only connect to the input server at this time.
				if (!EstablishConnection(InputConnect, InHost, InUser, InPort))
				{
					throw((string) "Could not establish input mysql connection.");
				}
			}
			
			const size_t PeriodPos = Source.find('.');

			if (PeriodPos == string::npos)
			{
				throw("Invalid source name for MySQL connection: " + Source
				      + "\nMust follow format of DatabaseName.TableName");
			}

			if (!InputConnect.select_db(Source.substr(0, PeriodPos)))
			{
				throw("Could not connect to the database: " + Source.substr(0, PeriodPos));
			}
		}
		catch (...)
		{
			cerr << "ERROR: Problem establishing mysql connection.  Host/User/Port: " 
			     << InHost + '/' + InUser + '/' << InPort << endl;
			throw;
		}

		// So, the Input source for the MySQL will be the database.table pair
		InputSources.push_back(Source);
	}
	else if (InputFunc == "From_File")
	{
		if (access(Source.c_str(), R_OK) != 0)
		{
			perror(Source.c_str());
			throw("Problem accessing the input directory " + Source);
		}

		boost::regex InputRegex;

		try
		{
			// regex string comes from the plugin registration, unless overridden by the configuration file
			InputRegex = DeafConf.Give_FileInputStr( InPluginName );
		}
		catch (...)
		{
			cerr << "ERROR: Exception caught!" << endl;
			cerr << "     : input regex string was: " << DeafConf.Give_FileInputStr( InPluginName ) << endl;
			throw;
		}

		
		try
		{			
			InputSources = FindFileMatches(Source, "", InputRegex);
		}
		catch (int ErrNum)
		{
                	perror(Source.c_str());
			throw("Problem while searching directories in " + Source); 
		}
		catch (...)
		{
			throw;
		}
	}
	else if (InputFunc == "From_Stream")
	{
		// So, whatever the user gives as the Source will be used as the InputSource
		// The value doesn't really matter.  Although, I guess one could pass a
		// named stream..., have to investigate this later.
		InputSources.push_back(Source);

		if (!cin.good())
		{
			throw((string) "Problem with the input stream");
		}
	}
	else
	{
		throw("Programming error.  Unaccounted InputFunc: " + InputFunc);
	}


	// Now for all of the output information...
	if (OutputFunc == "To_MySQL")
	{
		try
		{
			if (!OutputConnect.connected())
			{
				// The connection hasn't already been established earlier.
				const string OutHost = DeafConf.Give_OutputHost( OutPluginName );
	                        const string OutUser = DeafConf.Give_OutputUser( OutPluginName );
				const unsigned int OutPort = DeafConf.Give_OutputHostPort( OutPluginName );

				if (!EstablishConnection(OutputConnect, OutHost, OutUser, OutPort))
				{
					throw("Could not establish mysql output connection  Host/User: " + OutHost + '/' + OutUser);
				}
			}

			const size_t PeriodPos = Destination.find('.');

                        if (PeriodPos == string::npos)
                        {
                                throw("Invalid destination name for MySQL connection: " + Source
                                      + "\nMust follow format of DatabaseName.TableName");
                        }

				
			if (!OutputConnect.select_db(Destination.substr(0, PeriodPos)))
			{
				throw("Could not establish connection to output database: " + Destination.substr(0, PeriodPos));
			}
		}
		catch (...)
		{
                        cerr << "ERROR: Problem establishing output mysql connection.\n";
                       	throw;
                }
			

		// We want the same number of outputsources as there are inputsources
		OutputSources.assign(InputSources.size(), Destination);
	}
	else if (OutputFunc == "To_File")
	{
		if (access(Destination.c_str(), W_OK) != 0)
		{
			perror(Destination.c_str());
			throw("Problem while accessing the destination directory " + Destination);
		}

		if (InputFunc == "From_File")
		{
			const string OutputFormat = DeafConf.Give_FileOutputStr( OutPluginName );

			try
			{
				boost::regex InputRegex;
                	        InputRegex = DeafConf.Give_FileInputStr( InPluginName );

				OutputSources.resize(InputSources.size());

	                        for (list<string>::iterator InFilename( InputSources.begin() ), OutFilename( OutputSources.begin() );
        	                     InFilename != InputSources.end();
       	        	             InFilename++, OutFilename++)
               	        	{
                       	        	*OutFilename = regex_replace(*InFilename, InputRegex, OutputFormat);
	                       	}
			}
       	        	catch (...)
	               	{
        	        	cerr << "ERROR: Exception caught!" << endl;
                	        cerr << "     : input file string was: " << DeafConf.Give_FileInputStr( InPluginName ) << endl;
				cerr << "     : output file string was: " << OutputFormat << endl;
	                        throw;
       		        }
		}
		else
		{
			OutputSources.assign(InputSources.size(), DeafConf.Give_OutputFilename( OutPluginName ));
		}
	}
	else if (OutputFunc == "To_Stream")
	{
		// We want the same number of OutputSources as there are InputSources, 
		// even if we don't use it.  Although, this should be investigated further...
		OutputSources.assign(InputSources.size(), Destination);

		if (!cout.good())
		{
			throw((string) "Problem with output stream");
		}
	}
	else
	{
		throw("Programming error.  Unaccounted OutputFunc: " + OutputFunc);
	}
}





//**********************************************************
//----------- TestAndLoadPlugins() -------------------------
//**********************************************************
void TestAndLoadPlugins(const DeafConf_t &DeafConf,
			const string &InPluginName, const string &OutPluginName, 
			bool SamePlugin, bool SameBean,
			deafInfo_t &InPlugin_Info, deafInfo_t &OutPlugin_Info,
			DeafPlugin* &InUserPlugin_Ptr, DeafPlugin* &OutUserPlugin_Ptr)
{
	if (!InPlugin_Info.LoadPlugin( DeafConf.Give_PluginFilename( InPluginName ), 
				       InPluginName, 
				       DeafConf.Give_BeanName( InPluginName )))
	{
		throw("Could not load plugin: " + InPluginName + " filename: " + DeafConf.Give_PluginFilename( InPluginName ));
	}

//	cout << "Calling the factory function: " << endl;
	if ((InUserPlugin_Ptr = InPlugin_Info.PluginFactory()) == NULL)
	{
		throw("Null pointer obtained while trying to use input plugin: " + InPluginName);
	}

//	cout << "Done factory...Now for the bean..." << endl;

        if (!InPlugin_Info.TestBean())
        {
                throw("Bean test failed for input bean: " + InPlugin_Info.Give_BeanName() + " from plugin " + InPluginName);
        }

//	cout << "Successful bean test!\n";
		
	if (SamePlugin)
	{
		OutPlugin_Info = InPlugin_Info;
		OutUserPlugin_Ptr = InUserPlugin_Ptr;
	}
	else
	{
		if (!OutPlugin_Info.LoadPlugin( DeafConf.Give_PluginFilename( OutPluginName ), 
					        OutPluginName, 
					        DeafConf.Give_BeanName( OutPluginName )))
		{
			throw("Could not load output plugin: " + OutPluginName + " filename: " + DeafConf.Give_PluginFilename( OutPluginName ));
		}

		
		if ((OutUserPlugin_Ptr = OutPlugin_Info.PluginFactory()) == NULL)
		{
			throw("Null pointer obtained while trying to use output plugin: " + OutPluginName);
		}
			
		if (!SameBean)
		{	
               		if (!OutPlugin_Info.TestBean())
               		{
                       		throw("Bean test failed for output bean: " + OutPlugin_Info.Give_BeanName() + " from plugin " + OutPluginName);
               		}
		}
	}
}


void DoInput(const string &InputFunc, DeafPlugin* &InUserPlugin_Ptr, DeafBean* &InPluginBean_Ptr,
             const string &SourceLoc, const string &A_Source, mysqlpp::Connection &InputConnect)
{
	if (InputFunc == "From_MySQL")
        {
        	// get database name and table name from the A_Source iterator...
		// The names must have been verified already as following the
		// correct format.
                const string DatabaseName = A_Source.substr(0, A_Source.find('.'));
                const string TableName = A_Source.substr(A_Source.find('.') + 1);
                mysqlpp::Query TheQuery = InputConnect.query();

                if (!InUserPlugin_Ptr->From_MySQL(TheQuery, DatabaseName, TableName, *InPluginBean_Ptr))
                {
	                throw("Could not extract data from mysql database.table: " + A_Source);
                }
	}
        else if (InputFunc == "From_File")
        {
        	if (!InUserPlugin_Ptr->From_File(SourceLoc, A_Source, *InPluginBean_Ptr))
                {
                	throw("Could not extract data from file: " + SourceLoc + '/' + A_Source);
                }
        }
        else if (InputFunc == "From_Stream")
        {
        	if (!InUserPlugin_Ptr->From_Stream(cin, *InPluginBean_Ptr))
                {
                	throw((string) "Could not extract data from the stream.");
                }
        }
        else
        {
        	throw("Unknown Input function: " + InputFunc);
        }
}




void DoOutput(const string &OutputFunc, DeafPlugin* &OutUserPlugin_Ptr, DeafBean* &OutPluginBean_Ptr, 
	      const string &DestinationLoc, const string &A_Dest, mysqlpp::Connection &OutputConnect)
{
	if (OutputFunc == "To_MySQL")
        {
		// The destination name must have already been verified as having
		// the correct format.
	        const string DatabaseName = A_Dest.substr(0, A_Dest.find('.'));
                const string TableName = A_Dest.substr(A_Dest.find('.') + 1);
                mysqlpp::Query TheQuery = OutputConnect.query();

                if (!OutUserPlugin_Ptr->To_MySQL(*OutPluginBean_Ptr, TheQuery, DatabaseName, TableName))
                {
                	throw("Could not add data to mysql database.table: " + A_Dest);
                }
	}
        else if (OutputFunc == "To_File")
        {
        	if (A_Dest.rfind('/') != string::npos)
                {
                	if (!MakeDirectory(DestinationLoc + '/' + A_Dest.substr(0, A_Dest.rfind('/'))))
                        {
                        	cerr << "WARNING: Could not create directory "
                                     << DestinationLoc + '/' + A_Dest.substr(0, A_Dest.rfind('/')) << '\n'
                                     << "Chances are, the plug-in will have trouble creating files...\n";
                        }
                }

                if (!OutUserPlugin_Ptr->To_File(*OutPluginBean_Ptr, DestinationLoc, A_Dest))
                {
                	throw("Could not write to file: " + DestinationLoc + '/' + A_Dest);
                }
	}
        else if (OutputFunc == "To_Stream")
        {
                if (!OutUserPlugin_Ptr->To_Stream(*OutPluginBean_Ptr, cout))
                {
                	throw((string) "Could not properly output data to the stream.");
                }
        }
        else
        {
        	throw("Unknown Output function: " + OutputFunc);
        }
}

