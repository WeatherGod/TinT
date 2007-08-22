#ifndef _PLUGINCONF_C
#define _PLUGINCONF_C
using namespace std;

#include <iostream>
#include <vector>
#include <string>
#include <libxml++/libxml++.h>	// for xml functions and types

#include <algorithm>	// for lower_bound() and binary_search()

#include <PluginConf.h>

PluginConf::PluginConf()
	:	myPluginName(""),
		myPluginFilename(""),
		myPluginRegistration(""),
		myBeanName(""),
		myAvailFuncs(0),
		myInputHost(""),
		myInputUser(""),
		myInputHostPort(0),
		myOutputHost(""),
		myOutputUser(""),
		myOutputHostPort(0),
		myOutputFilename(""),
		myFileInputStr(""),
		myFileOutputStr(""),
		myDescriptStr(""),
		myAuthorName(""),
		myContactStr(""),
		myNotesStr(""),
		myIsValid(false)
{
}

PluginConf::PluginConf(const PluginConf &OtherConf)
	:	myPluginName(OtherConf.myPluginName),
		myPluginFilename(OtherConf.myPluginFilename),
		myPluginRegistration(OtherConf.myPluginRegistration),
		myBeanName(OtherConf.myBeanName),
		myAvailFuncs(OtherConf.myAvailFuncs),
		myInputHost(OtherConf.myInputHost),
		myInputUser(OtherConf.myInputUser),
		myInputHostPort(OtherConf.myInputHostPort),
		myOutputHost(OtherConf.myOutputHost),
		myOutputUser(OtherConf.myOutputUser),
		myOutputHostPort(OtherConf.myOutputHostPort),
		myOutputFilename(OtherConf.myOutputFilename),
		myFileInputStr(OtherConf.myFileInputStr),
		myFileOutputStr(OtherConf.myFileOutputStr),
		myDescriptStr(OtherConf.myDescriptStr),
		myAuthorName(OtherConf.myAuthorName),
		myContactStr(OtherConf.myContactStr),
		myNotesStr(OtherConf.myNotesStr),
		myIsValid(OtherConf.myIsValid)
{
}

PluginConf::PluginConf(const string &ConfFilename)
	:	myPluginName(""),
                myPluginFilename(""),
                myPluginRegistration(""),
                myBeanName(""),
                myAvailFuncs(0),
                myInputHost(""),
                myInputUser(""),
		myInputHostPort(0),
                myOutputHost(""),
                myOutputUser(""),
		myOutputHostPort(0),
		myOutputFilename(""),
                myFileInputStr(""),
                myFileOutputStr(""),
		myDescriptStr(""),
		myAuthorName(""),
		myContactStr(""),
		myNotesStr(""),
                myIsValid(false)
{
	myIsValid = LoadRegistration(ConfFilename);
}



PluginConf::~PluginConf()
{
	myAvailFuncs.clear();
	myIsValid = false;
}

bool PluginConf::IsValid() const
{
	return(myIsValid);
}

void PluginConf::ListPlugin() const
{
	cout << "Name................ " << myPluginName << "\n"
	     << "Bean................ " << myBeanName << "\n"
	     << "Registration File... " << myPluginRegistration << "\n"
	     << "Library File........ " << myPluginFilename << "\n\n";

	cout << "Category    Input                      Output\n"
	     << "--------    -----                      ------\n";
	cout << "MySQL\n"
	     << "      Host: ";
	printf("%-25s  %-20s\n", myInputHost.c_str(), myOutputHost.c_str());

	cout << "      User: ";
	printf("%-25s  %-20s\n", myInputUser.c_str(), myOutputUser.c_str());

	cout << "      Port: ";
	printf("%-25u  %-20u\n", myInputHostPort, myOutputHostPort);

	cout << "File\n"
	     << "     Regex: ";
	printf("%-25s  %-20s\n", myFileInputStr.c_str(), myFileOutputStr.c_str());

	cout << "   Default:                            " << myOutputFilename << "\n";

	cout << "\nAvailable Functions:\n";

	for (vector<string>::const_iterator AFunc = myAvailFuncs.begin(); AFunc != myAvailFuncs.end(); AFunc++)
	{
		cout << "       " << *AFunc << "\n";
	}

	cout << '\n';

	if (!myDescriptStr.empty())
	{
		cout << "Description: " << myDescriptStr << '\n';
	}

	if (!myAuthorName.empty())
	{
		cout << "Author: " << myAuthorName << '\n';
	}

	if (!myContactStr.empty())
	{
		cout << "Contact: " << myContactStr << '\n';
	}

	if (!myNotesStr.empty())
	{
		cout << "Notes: " << myNotesStr << '\n';
	}
}






bool PluginConf::LoadRegistration(const string &ConfFilename)
{
	try
	{
		xmlpp::DomParser TheParser;
//		TheParser.set_validate();
	        TheParser.parse_file(ConfFilename.c_str());

        	if (!TheParser)
	        {
        	        throw("Problem reading plugin registration file: " + ConfFilename);
		}

		const string TagName = TheParser.get_document()->get_root_node()->get_name();

		if (TagName != "Plugin")
		{
			throw("This does not look like a Plugin configuration node.  Starting tag: " + TagName);
		}

		myIsValid = LoadRegistration( TheParser.get_document()->get_root_node() );
	}
	catch (const exception &Err)
	{
		cerr << "ERROR: Problem loading plugin configurations: " << ConfFilename << endl;
		cerr << "     : " << Err.what() << endl;
		myIsValid = false;
	}
	catch (const string &ErrStr)
	{
		cerr << "ERROR: Problem loading plugin configurations: " << ConfFilename << endl;
		cerr << "     : " << ErrStr << endl;
		myIsValid = false;
	}
	catch (...)
	{
		cerr << "ERROR: Problem loading plugin configurations: " << ConfFilename << endl;
		cerr << "     : Unknown exception was caught..." << endl;
		myIsValid = false;
	}

	return(myIsValid);
}

bool PluginConf::LoadRegistration(const xmlpp::Node* PluginNode)
{
//	cerr << "Doing dynamic casting...";
	const xmlpp::Element* ElementNode = dynamic_cast<const xmlpp::Element*>(PluginNode);
//	cerr << "Success!\n";
	if (ElementNode == NULL)
	{
		cerr << "ERROR: Problem with Plugin node of the configuration...\n";
		return(false);
	}

	const xmlpp::Element::AttributeList TheAttribs = ElementNode->get_attributes();
	for (xmlpp::Element::AttributeList::const_iterator An_Attrib = TheAttribs.begin(); An_Attrib != TheAttribs.end(); An_Attrib++)
	{
		const string AttribName = (*An_Attrib)->get_name();
		if (AttribName == "Name")
		{
			Take_PluginName( (*An_Attrib)->get_value() );
		}
		else if (AttribName == "Registration")
		{
			Take_RegistrationFilename( (*An_Attrib)->get_value() );
		}
		else if (AttribName == "File")
		{
			Take_PluginFilename( (*An_Attrib)->get_value() );
		}
		else if (AttribName == "Bean")
		{
			Take_BeanName( (*An_Attrib)->get_value() );
		}
		else
                {
                        cerr << "WARNING: Unknown attribute name for Plugin tag: " << AttribName << "  value: " << (*An_Attrib)->get_value() << endl;
                }

	}

//	cerr << "Getting node list...\n";

	const xmlpp::Node::NodeList TheNodes = PluginNode->get_children();

	for (xmlpp::Node::NodeList::const_iterator A_Node = TheNodes.begin(); A_Node != TheNodes.end(); A_Node++)
	{		
		if ((*A_Node)->get_name() == "MySQL")
		{
			Take_MySQLNode(*A_Node);
		}
		else if ((*A_Node)->get_name() == "File")
		{
			Take_FileNode(*A_Node);
		}
		else if ((*A_Node)->get_name() == "Function")
		{
			Take_FuncNode(*A_Node);
		}
		else if ((*A_Node)->get_name() == "Author")
		{
			Take_AuthorNode(*A_Node);
		}
		else if ((*A_Node)->get_name() == "Description")
		{
			Take_DescriptionNode(*A_Node);
		}
		else if ((*A_Node)->get_name() == "Contact")
		{
			Take_ContactNode(*A_Node);
		}
		else if ((*A_Node)->get_name() == "Notes")
		{
			Take_NotesNode(*A_Node);
		}
		else if ((*A_Node)->get_name() != "text")
		{
			cerr << "WARNING: Unknown tag in registration for plug-in " << myPluginName << ": " << (*A_Node)->get_name() << endl;
		}
	}


	return(true);
}

void PluginConf::Take_MySQLNode(const xmlpp::Node* TheNode)
{
	const xmlpp::Element* ElementNode = dynamic_cast<const xmlpp::Element*>(TheNode);
//      cerr << "Success!\n";
        if (ElementNode == NULL)
        {
                throw((string) "Problem with MySQL node of the configuration...");
        }

        const xmlpp::Element::AttributeList TheAttribs = ElementNode->get_attributes();
        for (xmlpp::Element::AttributeList::const_iterator An_Attrib = TheAttribs.begin(); An_Attrib != TheAttribs.end(); An_Attrib++)
	{
		const string AttribName = (*An_Attrib)->get_name();

		if (AttribName == "InputHost")
		{
			Take_InputHost( (*An_Attrib)->get_value() );
		}
		else if (AttribName == "InputUser")
		{
			Take_InputUser( (*An_Attrib)->get_value() );
		}
		else if (AttribName == "InputHostPort")
		{
			Take_InputHostPort( strtoul((*An_Attrib)->get_value().c_str(), NULL, 10) );
		}
		else if (AttribName == "OutputHost")
		{
			Take_OutputHost( (*An_Attrib)->get_value() );
		}
		else if (AttribName == "OutputUser")
		{
			Take_OutputUser( (*An_Attrib)->get_value() );
		}
		else if (AttribName == "OutputHostPort")
		{
			Take_OutputHostPort( strtoul((*An_Attrib)->get_value().c_str(), NULL, 10) );
		}
		else
		{
			cerr << "WARNING: Unknown attribute name in MySQL tag: " << AttribName << "  value: " << (*An_Attrib)->get_value() << endl;
		}
	}
}

void PluginConf::Take_FileNode(const xmlpp::Node* TheNode)
{
	const xmlpp::Element* ElementNode = dynamic_cast<const xmlpp::Element*>(TheNode);
//      cerr << "Success!\n";
        if (ElementNode == NULL)
        {
                throw((string) "Problem with File node of the configuration...");
        }

        const xmlpp::Element::AttributeList TheAttribs = ElementNode->get_attributes();
        for (xmlpp::Element::AttributeList::const_iterator An_Attrib = TheAttribs.begin(); An_Attrib != TheAttribs.end(); An_Attrib++)
        {
                const string AttribName = (*An_Attrib)->get_name();

                if (AttribName == "Input")
		{
			Take_FileInputStr( (*An_Attrib)->get_value() );
		}
		else if (AttribName == "Output")
		{
			Take_FileOutputStr( (*An_Attrib)->get_value() );
		}
		else if (AttribName == "Filename")
		{
			Take_OutputFilename( (*An_Attrib)->get_value() );
		}
		else
                {
                        cerr << "WARNING: Unknown attribute name: " << AttribName << "  value: " << (*An_Attrib)->get_value() << endl;
                }
	}
}

void PluginConf::Take_FuncNode(const xmlpp::Node* TheNode)
{
	const xmlpp::Element* ElementNode = dynamic_cast<const xmlpp::Element*>(TheNode);
//      cerr << "Success!\n";
        if (ElementNode == NULL)
        {
                throw((string) "Problem with Func node of the configuration...");
        }

        const xmlpp::Element::AttributeList TheAttribs = ElementNode->get_attributes();
        for (xmlpp::Element::AttributeList::const_iterator An_Attrib = TheAttribs.begin(); An_Attrib != TheAttribs.end(); An_Attrib++)
        {
                const string AttribName = (*An_Attrib)->get_name();

                if (AttribName == "Name")
		{
			Take_AvailFunc( (*An_Attrib)->get_value() );
		}
		else
                {
                        cerr << "WARNING: Unknown attribute name in Function tag: " << AttribName << "  value: " << (*An_Attrib)->get_value() << endl;
                }
	}
}

void PluginConf::Take_DescriptionNode(const xmlpp::Node* TheNode)
{
	const xmlpp::Node::NodeList ChildNodes = TheNode->get_children();

	for (xmlpp::Node::NodeList::const_iterator A_Node = ChildNodes.begin(); A_Node != ChildNodes.end(); A_Node++)
        {
		const xmlpp::TextNode* NodeText = dynamic_cast<const xmlpp::TextNode*>(*A_Node);

		if (NodeText == NULL)
		{
			continue;
		}

		if (!NodeText->is_white_space())
		{
			Take_Description(NodeText->get_content());
		}
	}
}

void PluginConf::Take_AuthorNode(const xmlpp::Node* TheNode)
{
	const xmlpp::Node::NodeList ChildNodes = TheNode->get_children();

        for (xmlpp::Node::NodeList::const_iterator A_Node = ChildNodes.begin(); A_Node != ChildNodes.end(); A_Node++)
        {
                const xmlpp::TextNode* NodeText = dynamic_cast<const xmlpp::TextNode*>(*A_Node);

                if (NodeText == NULL)
                {
                        continue;
                }

		if (!NodeText->is_white_space())
        	{
	        	Take_Author(NodeText->get_content());
		}
	}
}

void PluginConf::Take_ContactNode(const xmlpp::Node* TheNode)
{
        const xmlpp::Node::NodeList ChildNodes = TheNode->get_children();

        for (xmlpp::Node::NodeList::const_iterator A_Node = ChildNodes.begin(); A_Node != ChildNodes.end(); A_Node++)
        {
                const xmlpp::TextNode* NodeText = dynamic_cast<const xmlpp::TextNode*>(*A_Node);

                if (NodeText == NULL)
                {
                        continue;
                }


		if (!NodeText->is_white_space())
        	{
	        	Take_Contact(NodeText->get_content());
		}
	}
}

void PluginConf::Take_NotesNode(const xmlpp::Node* TheNode)
{
        const xmlpp::Node::NodeList ChildNodes = TheNode->get_children();

        for (xmlpp::Node::NodeList::const_iterator A_Node = ChildNodes.begin(); A_Node != ChildNodes.end(); A_Node++)
        {
                const xmlpp::TextNode* NodeText = dynamic_cast<const xmlpp::TextNode*>(*A_Node);

                if (NodeText == NULL)
                {
                        continue;
                }

		if (!NodeText->is_white_space())
        	{
	        	Take_Notes(NodeText->get_content());
		}
	}
}





PluginConf& operator += (PluginConf &OrigConf, const PluginConf &AdditionConf)
{
	if (!AdditionConf.IsValid())
	{
		return(OrigConf);
	}

	OrigConf.Take_PluginName(AdditionConf.myPluginName);
	OrigConf.Take_RegistrationFilename(AdditionConf.myPluginRegistration);
	OrigConf.Take_PluginFilename(AdditionConf.myPluginFilename);
	OrigConf.Take_BeanName(AdditionConf.myBeanName);
	OrigConf.Take_InputHost(AdditionConf.myInputHost);
	OrigConf.Take_InputUser(AdditionConf.myInputUser);
	OrigConf.Take_InputHostPort(AdditionConf.myInputHostPort);
	OrigConf.Take_OutputHost(AdditionConf.myOutputHost);
	OrigConf.Take_OutputUser(AdditionConf.myOutputUser);
	OrigConf.Take_OutputHostPort(AdditionConf.myOutputHostPort);
	OrigConf.Take_OutputFilename(AdditionConf.myOutputFilename);
	OrigConf.Take_FileInputStr(AdditionConf.myFileInputStr);
	OrigConf.Take_FileOutputStr(AdditionConf.myFileOutputStr);
	OrigConf.Take_Description(AdditionConf.myDescriptStr);
	OrigConf.Take_Author(AdditionConf.myAuthorName);
	OrigConf.Take_Contact(AdditionConf.myContactStr);
	OrigConf.Take_Notes(AdditionConf.myNotesStr);

	for (vector<string>::const_iterator AFuncName = AdditionConf.myAvailFuncs.begin();
	     AFuncName != AdditionConf.myAvailFuncs.end();
	     AFuncName++)
	{
		OrigConf.Take_AvailFunc(*AFuncName);
	}

	OrigConf.myIsValid = true;

	return(OrigConf);
}

//-----------------------------------------------------------------------------------------------------------------
bool PluginConf::Func_Avail(const string &FunctionName) const
{
	return(binary_search(myAvailFuncs.begin(), myAvailFuncs.end(), FunctionName));
}

string PluginConf::Give_PluginName() const
{
	return(myPluginName);
}

string PluginConf::Give_RegistrationFilename() const
{
	return(myPluginRegistration);
}

string PluginConf::Give_PluginFilename() const
{
	return(myPluginFilename);
}

string PluginConf::Give_BeanName() const
{
	return(myBeanName);
}

string PluginConf::Give_InputHost() const
{
	return(myInputHost);
}

string PluginConf::Give_InputUser() const
{
	return(myInputUser);
}

unsigned int PluginConf::Give_InputHostPort() const
{
	return(myInputHostPort);
}

string PluginConf::Give_OutputHost() const
{
	return(myOutputHost);
}

string PluginConf::Give_OutputUser() const
{
	return(myOutputUser);
}

unsigned int PluginConf::Give_OutputHostPort() const
{
	return(myOutputHostPort);
}

string PluginConf::Give_OutputFilename() const
{
	return(myOutputFilename);
}

string PluginConf::Give_FileInputStr() const
{
	return(myFileInputStr);
}

string PluginConf::Give_FileOutputStr() const
{
	return(myFileOutputStr);
}

string PluginConf::Give_Description() const
{
	return(myDescriptStr);
}

string PluginConf::Give_Author() const
{
	return(myAuthorName);
}

string PluginConf::Give_Contact() const
{
	return(myContactStr);
}

string PluginConf::Give_Notes() const
{
	return(myNotesStr);
}

vector <string> PluginConf::Give_AvailFuncs() const
{
	return(myAvailFuncs);
}

//----------------------------------------------------------------------------
/*	Member Value Modifiers
   These functions check to see if the member value is 'undefined'.
   If the member value is undefined, then it is ok to assign a value.
   This allows for an 'over-riding' behavior to occur by reading the
   highest priority file first, with each succeeding file providing
   additional information that is not provided by the previous files.
*/
void PluginConf::Take_PluginName(const string &PluginName)
{
	if (myPluginName.empty())
	{
		myPluginName = PluginName;
	}
}

void PluginConf::Take_RegistrationFilename(const string &RegistrationFilename)
{
        if (myPluginRegistration.empty())
        {
                myPluginRegistration = RegistrationFilename;
        }
}

void PluginConf::Take_PluginFilename(const string &PluginFilename)
{
        if (myPluginFilename.empty())
        {
                myPluginFilename = PluginFilename;
        }
}

void PluginConf::Take_BeanName(const string &BeanName)
{
        if (myBeanName.empty())
        {
                myBeanName = BeanName;
        }
}

void PluginConf::Take_InputHost(const string &InputHost)
{
        if (myInputHost.empty())
        {
                myInputHost = InputHost;
        }
}

void PluginConf::Take_InputUser(const string &InputUser)
{
        if (myInputUser.empty())
        {
                myInputUser = InputUser;
        }
}

void PluginConf::Take_InputHostPort(const unsigned int PortNumber)
{
	if (myInputHostPort == 0)
	{
		myInputHostPort = PortNumber;
	}
}

void PluginConf::Take_OutputHost(const string &OutputHost)
{
        if (myOutputHost.empty())
        {
                myOutputHost = OutputHost;
        }
}

void PluginConf::Take_OutputUser(const string &OutputUser)
{
        if (myOutputUser.empty())
        {
                myOutputUser = OutputUser;
        }
}

void PluginConf::Take_OutputHostPort(const unsigned int PortNumber)
{
	if (myOutputHostPort == 0)
	{
		myOutputHostPort = PortNumber;
	}
}

void PluginConf::Take_OutputFilename(const string &OutputFilename)
{
	if (myOutputFilename.empty())
	{
		myOutputFilename = OutputFilename;
	}
}

void PluginConf::Take_FileInputStr(const string &FileInputStr)
{
        if (myFileInputStr.empty())
        {
                myFileInputStr = FileInputStr;
        }
}

void PluginConf::Take_FileOutputStr(const string &FileOutputStr)
{
        if (myFileOutputStr.empty())
        {
                myFileOutputStr = FileOutputStr;
        }
}

void PluginConf::Take_AvailFunc(const string &FunctionName)
{
        if (!binary_search(myAvailFuncs.begin(), myAvailFuncs.end(), FunctionName))
        {
		myAvailFuncs.insert(lower_bound(myAvailFuncs.begin(), myAvailFuncs.end(), FunctionName), FunctionName);
        }
}

void PluginConf::Take_Description(const string &DescriptionStr)
{
        if (myDescriptStr.empty())
        {
                myDescriptStr = DescriptionStr;
        }
}

void PluginConf::Take_Author(const string &AuthorName)
{
        if (myAuthorName.empty())
        {
                myAuthorName = AuthorName;
        }
}

void PluginConf::Take_Contact(const string &ContactStr)
{
        if (myContactStr.empty())
        {
                myContactStr = ContactStr;
        }
}

void PluginConf::Take_Notes(const string &NotesStr)
{
        if (myNotesStr.empty())
        {
                myNotesStr = NotesStr;
        }
}



#endif
