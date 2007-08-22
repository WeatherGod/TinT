#ifndef _DEAFCONF_C
#define _DEAFCONF_C
using namespace std;

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <unistd.h>		// for access()

#include <DeafConf.h>
#include <PluginConf.h>

#include <deafUtil.h>		// for GetDeafDirs()

#include <libxml++/libxml++.h>


DeafConf_t::DeafConf_t()
	:	myPluginConfs(),
		myIsValid(false)
{
}


DeafConf_t::DeafConf_t(const DeafConf_t &OtherConfig)
	:	myPluginConfs( OtherConfig.myPluginConfs ),
		myIsValid( OtherConfig.myIsValid )
{
}

DeafConf_t::DeafConf_t(const string &ConfFilename)
	:	myPluginConfs(),
		myIsValid(false)
{
	myIsValid = LoadConfiguration(ConfFilename);
}



DeafConf_t::~DeafConf_t()
{
	myPluginConfs.erase(myPluginConfs.begin(), myPluginConfs.end());
	myIsValid = false;
}

bool DeafConf_t::LoadConfiguration(const string &ConfFilename)
{
	try
	{
		xmlpp::DomParser TheParser;
//		TheParser.set_validate();
//		TheParser.set_substitute_entities();
		TheParser.parse_file(ConfFilename.c_str());

        	if (!TheParser)
        	{
                	throw("Could not parse configuration file: " + ConfFilename);
        	}

//		cerr << "Over here\n";
    
		const string TagName = TheParser.get_document()->get_root_node()->get_name();

        	if (TagName != "DeafConf")
		{
			throw("This does not look like a DeafConf file.  Starting tag: " + TagName);
		}

//		cerr << "About to load config..." << endl;
		
		myIsValid = LoadConfiguration( TheParser.get_document()->get_root_node() );
	}
	catch (const exception &Err)
	{
		cerr << "ERROR: Problem loading configuration file: " << ConfFilename << endl;
		cerr << "     : " << Err.what() << endl;
		myIsValid = false;
	}
	catch (const string &ErrStr)
	{
		cerr << "ERROR: Problem loading configuration file: " << ConfFilename << endl;
		cerr << "     : " << ErrStr << endl;
		myIsValid = false;
	}
	catch (...)
	{
		cerr << "ERROR: Problem loading configuration file: " << ConfFilename << endl;
		cerr << "     : Unknown exception was caught..." << endl;
		myIsValid = false;
	}

	//cerr << "finished loading..." << endl;

	return(myIsValid);
}


bool DeafConf_t::LoadConfiguration(const xmlpp::Node* ConfigNode)
{
//	cerr << "Getting children..." << endl;
	const xmlpp::Node::NodeList TheNodes = ConfigNode->get_children();

	for (xmlpp::Node::NodeList::const_iterator Cur_Node = TheNodes.begin(); Cur_Node != TheNodes.end(); Cur_Node++)
	{
		if ((*Cur_Node)->get_name() == "Plugin")
		{
//			cerr << "About to load registration..." << endl;
			if (!LoadRegistration(*Cur_Node))	// note, *Cur_Node returns a pointer to the Node object
			{
				throw((string) "Couldn't load plugin information.");
			}
		}
	}

	return(true);
}



bool DeafConf_t::IsValid() const
{
	return(myIsValid);
}

bool DeafConf_t::LoadRegistration(const string &PluginName)
// If the plugin doesn't exist already, then return false.
// if it does exist, then load up the registration file
// that is mentioned in the PluginConf, if it exists.
{
	map <string, PluginConf>::iterator ThePlugin = myPluginConfs.find(PluginName);
	if (ThePlugin == myPluginConfs.end())
	{
		cerr << "ERROR: Unknown plugin: " << PluginName << endl;
		return(false);
	}

	const string RegFilename = ThePlugin->second.Give_RegistrationFilename();

	if (RegFilename.empty())
	{
		cerr << "ERROR: No registration filename for plug-in: " << PluginName << endl;
		return(false);
	}

	string FilePath = "";

	if (RegFilename[0] != '/')	// if it is relative path
	{
		const vector <string> DeafDirs = GetDeafDirs();

		vector<string>::const_iterator ADir = DeafDirs.begin();

		while (ADir != DeafDirs.end() && access((*ADir + '/' + RegFilename).c_str(), R_OK) != 0)
		{
			ADir++;
		}

		if (ADir != DeafDirs.end())
		{
			FilePath = *ADir;
		}
		else
		{
			FilePath = ".";
		}
	}

	PluginConf TempConf(FilePath + '/' + RegFilename);

	if (TempConf.IsValid())
	{
		ThePlugin->second += TempConf;
		return(true);
	}
	else
	{
		cerr << "ERROR: Unable to load this registration file: " << FilePath + '/' + RegFilename << endl;
		return(false);
	}
}

bool DeafConf_t::LoadRegistration(const xmlpp::Node* PluginNode)
// If the plugin already exists in this class, load up any new info.
// If it doesn't exist already, then create a new PluginConf, load it,
// and then insert it into this class.
{
//	cerr << "Creating temp conf..." << endl;
	PluginConf TempConf;

//	cerr << "About to load registration for TempConf...\n";
	if (!TempConf.LoadRegistration(PluginNode))
	{
		cerr << "ERROR: Problem reading configuration for some plugin..." << endl;
		return(false);
	}

	//cerr << "Success!\n";
	const string PluginName = TempConf.Give_PluginName();

	if (PluginName.empty())
	{
		cerr << "ERROR: No name given for plugin in configuration file." << endl;
		return(false);
	}

	map <string, PluginConf>::iterator A_Plugin;
        if ((A_Plugin = myPluginConfs.find(PluginName)) == myPluginConfs.end())
        {
        	// Assume that it gets inserted properly
                myPluginConfs.insert( make_pair(PluginName, TempConf) );
	}
        else
        {
		A_Plugin->second += TempConf;
	}

	return(true);
}

//----------------------------------------------------------------------------------------------------------------------


vector <string> DeafConf_t::Give_PluginNames() const
{
	vector <string> PluginNames(myPluginConfs.size(), "");

	vector<string>::iterator AName = PluginNames.begin();
	for (map<string, PluginConf>::const_iterator A_Plugin = myPluginConfs.begin(); A_Plugin != myPluginConfs.end(); A_Plugin++, AName++)
	{
		*AName = A_Plugin->first;
	}

	return(PluginNames);
}


bool DeafConf_t::Func_Avail(const string &PluginName, const string &FunctionName) const
{
	map<string, PluginConf>::const_iterator A_Plugin = myPluginConfs.find(PluginName);

	if (A_Plugin == myPluginConfs.end())
	{
		cerr << "ERROR: Unknown plugin: " << PluginName << endl;
		return(false);
	}

	return(A_Plugin->second.Func_Avail(FunctionName));
}

string DeafConf_t::Give_RegistrationFilename(const string &PluginName) const
{
	map<string, PluginConf>::const_iterator A_Plugin = myPluginConfs.find(PluginName);

	if (A_Plugin == myPluginConfs.end())
	{
		throw("DeafConf_t::Give_RegistrationFilename(): Unknown plugin: " + PluginName);
	}

	return(A_Plugin->second.Give_RegistrationFilename());
}


string DeafConf_t::Give_PluginFilename(const string &PluginName) const
{
	map<string, PluginConf>::const_iterator A_Plugin = myPluginConfs.find(PluginName);

        if (A_Plugin == myPluginConfs.end())
        {
                throw("DeafConf_t::Give_PluginFilename(): Unknown plugin: " + PluginName);
        }

	return(A_Plugin->second.Give_PluginFilename());
}

string DeafConf_t::Give_BeanName(const string &PluginName) const
{
	map<string, PluginConf>::const_iterator A_Plugin = myPluginConfs.find(PluginName);

        if (A_Plugin == myPluginConfs.end())
        {
                throw("DeafConf_t::Give_BeanName(): Unknown plugin: " +PluginName);
        }

	return(A_Plugin->second.Give_BeanName());
}


string DeafConf_t::Give_InputHost(const string &PluginName) const
{
	map<string, PluginConf>::const_iterator A_Plugin = myPluginConfs.find(PluginName);

        if (A_Plugin == myPluginConfs.end())
        {
                throw("DeafConf_t::Give_InputHost(): Unknown plugin: " + PluginName);
        }

	return(A_Plugin->second.Give_InputHost());
}

string DeafConf_t::Give_InputUser(const string &PluginName) const
{
	map<string, PluginConf>::const_iterator A_Plugin = myPluginConfs.find(PluginName);

        if (A_Plugin == myPluginConfs.end())
        {
                throw("DeafConf_t::Give_InputUser(): Unknown plugin: " + PluginName);
        }

	return(A_Plugin->second.Give_InputUser());
}

unsigned int DeafConf_t::Give_InputHostPort(const string &PluginName) const
{
	map<string, PluginConf>::const_iterator A_Plugin = myPluginConfs.find(PluginName);

        if (A_Plugin == myPluginConfs.end())
        {
                throw("DeafConf_t::Give_InputHostPort(): Unknown plugin: " + PluginName);
        }

        return(A_Plugin->second.Give_InputHostPort());
}


string DeafConf_t::Give_OutputHost(const string &PluginName) const
{
	map<string, PluginConf>::const_iterator A_Plugin = myPluginConfs.find(PluginName);

        if (A_Plugin == myPluginConfs.end())
        {
                throw("DeafConf_t::Give_OutputHost(): Unknown plugin: " + PluginName);
        }

	return(A_Plugin->second.Give_OutputHost());
}

string DeafConf_t::Give_OutputUser(const string &PluginName) const
{
	map<string, PluginConf>::const_iterator A_Plugin = myPluginConfs.find(PluginName);

        if (A_Plugin == myPluginConfs.end())
        {
		throw("DeafConf_t::Give_OutputUser(): Unknown plugin: " + PluginName);
        }

	return(A_Plugin->second.Give_OutputUser());
}

unsigned int DeafConf_t::Give_OutputHostPort(const string &PluginName) const
{
	map<string, PluginConf>::const_iterator A_Plugin = myPluginConfs.find(PluginName);

        if (A_Plugin == myPluginConfs.end())
        {
                throw("DeafConf_t::Give_OutputHostPort(): Unknown plugin: " + PluginName);
        }

        return(A_Plugin->second.Give_OutputHostPort());
}

string DeafConf_t::Give_OutputFilename(const string &PluginName) const
{
	map<string, PluginConf>::const_iterator A_Plugin = myPluginConfs.find(PluginName);

	if (A_Plugin == myPluginConfs.end())
	{
		throw("DeafConf_t::Give_OutputFilename(): Unknown plugin: " + PluginName);
	}

	const string TempHold = A_Plugin->second.Give_OutputFilename();

	if (TempHold.empty())
	{
		return("default.out");
	}
	else
	{
		return(TempHold);
	}
}


string DeafConf_t::Give_FileInputStr(const string &PluginName) const
{
	map<string, PluginConf>::const_iterator A_Plugin = myPluginConfs.find(PluginName);

        if (A_Plugin == myPluginConfs.end())
        {
                cerr << "ERROR: Unknown plugin: " << PluginName << endl;
                return("");
        }

	return(A_Plugin->second.Give_FileInputStr());
}

string DeafConf_t::Give_FileOutputStr(const string &PluginName) const
{
	map<string, PluginConf>::const_iterator A_Plugin = myPluginConfs.find(PluginName);

        if (A_Plugin == myPluginConfs.end())
        {
                cerr << "ERROR: Unknown plugin: " << PluginName << endl;
                return("");
        }

	return(A_Plugin->second.Give_FileOutputStr());
}



bool DeafConf_t::ListPlugin(const string &PluginName, const bool &DoDetailed)
{
	map<string, PluginConf>::iterator A_Plugin = myPluginConfs.find(PluginName);

        if (A_Plugin == myPluginConfs.end())
        {
                cerr << "ERROR: Unknown plugin: " << PluginName << endl;
                return(false);
        }

	if (!LoadRegistration(PluginName))
	{
		cerr << "ERROR: Unable to read registration file, " << A_Plugin->second.Give_RegistrationFilename()
		     << ", for plugin " << PluginName << endl;
		return(false);
	}

	if (DoDetailed)
	{
		A_Plugin->second.ListPlugin();
	}
	else
	{
		printf("%s %s %s\n", "Plugin", "Bean", "RegistrationFile");
		printf("%s %s %s\n", PluginName.c_str(),
				     A_Plugin->second.Give_BeanName().c_str(),
				     A_Plugin->second.Give_RegistrationFilename().c_str());
	}

	cout << endl;

	return(true);
}

void DeafConf_t::ListPlugins()
{
	size_t MaxPluginNameLength = 0;
	size_t MaxBeanNameLength = 0;
	size_t MaxRegistLength = 0;

	for (map<string, PluginConf>::iterator A_Plugin = myPluginConfs.begin();
             A_Plugin != myPluginConfs.end();
             A_Plugin++)
        {
		if (!LoadRegistration(A_Plugin->first))
	        {
        	        cerr << "ERROR: Unable to read registration file, " << A_Plugin->second.Give_RegistrationFilename()
                	     << ", for plugin " << A_Plugin->first << endl;
	        }


		if (A_Plugin->first.length() > MaxPluginNameLength)
		{
			MaxPluginNameLength = A_Plugin->first.length();
		}

		if (A_Plugin->second.Give_BeanName().length() > MaxBeanNameLength)
                {
                        MaxBeanNameLength = A_Plugin->second.Give_BeanName().length();
                }

		if (A_Plugin->second.Give_RegistrationFilename().length() > MaxRegistLength)
                {
                        MaxRegistLength = A_Plugin->second.Give_RegistrationFilename().length();
                }
	}

	printf("%*s  %*s  %*s\n", -MaxPluginNameLength, "Plugin", 
				-MaxBeanNameLength, "Bean", 
				-MaxRegistLength, "RegistrationFile");

	for (map<string, PluginConf>::const_iterator A_Plugin = myPluginConfs.begin();
	     A_Plugin != myPluginConfs.end();
	     A_Plugin++)
	{
		printf("%*s  %*s  %*s\n", -MaxPluginNameLength, A_Plugin->first.c_str(),
                                        -MaxBeanNameLength, A_Plugin->second.Give_BeanName().c_str(),
                                        -MaxRegistLength, A_Plugin->second.Give_RegistrationFilename().c_str());
	}

	cout << endl;
}


#endif
