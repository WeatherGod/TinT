#ifndef _PLUGINCONF_H
#define _PLUGINCONF_H

#include <string>
#include <vector>
#include <libxml++/libxml++.h>

class PluginConf
{
	public:
		PluginConf();
		PluginConf(const PluginConf &OtherConfig);
		PluginConf(const string &ConfFilename);
		~PluginConf();

		bool IsValid() const;

		bool LoadRegistration(const string &ConfFilename);
		bool LoadRegistration(const xmlpp::Node* PluginNode);

		void ListPlugin() const;

		bool Func_Avail(const string &FunctionName) const;

		string Give_PluginName() const;
		string Give_RegistrationFilename() const;
		string Give_PluginFilename() const;
		string Give_BeanName() const;

		string Give_InputHost() const;
		string Give_InputUser() const;
		unsigned int Give_InputHostPort() const;

		string Give_OutputHost() const;
		string Give_OutputUser() const;
		unsigned int Give_OutputHostPort() const;


		string Give_OutputFilename() const;
		string Give_FileInputStr() const;
		string Give_FileOutputStr() const;

		string Give_Description() const;
		string Give_Author() const;
		string Give_Contact() const;
		string Give_Notes() const;

		vector <string> Give_AvailFuncs() const;



		void Take_PluginName(const string &PluginName);
		void Take_RegistrationFilename(const string &RegistFilename);
		void Take_PluginFilename(const string &PluginFilename);
		void Take_BeanName(const string &BeanName);

		void Take_InputHost(const string &InputHost);
		void Take_InputUser(const string &InputUser);
		void Take_InputHostPort(const unsigned int InputHostPort);

		void Take_OutputHost(const string &OutputHost);
		void Take_OutputUser(const string &OutputUser);
		void Take_OutputHostPort(const unsigned int OutputHostPort);


		void Take_FileInputStr(const string &RegExStr);
		void Take_FileOutputStr(const string &RegExStr);
		void Take_OutputFilename(const string &Filename);

		void Take_Description(const string &DescriptionStr);
		void Take_Author(const string &AuthorName);
		void Take_Contact(const string &ContactStr);
		void Take_Notes(const string &NotesStr);

		void Take_AvailFunc(const string &FunctionName);
		
	private:
		string myPluginName;
		string myPluginFilename;
		string myPluginRegistration;
		string myBeanName;
		vector <string> myAvailFuncs;
		string myInputHost;
		string myInputUser;
		unsigned int myInputHostPort;
		string myOutputHost;
		string myOutputUser;
		unsigned int myOutputHostPort;
		string myOutputFilename;
		string myFileInputStr;
		string myFileOutputStr;

		string myDescriptStr;
		string myAuthorName;
		string myContactStr;
		string myNotesStr;

		bool myIsValid;

		void Take_MySQLNode(const xmlpp::Node* A_Node);
		void Take_FileNode(const xmlpp::Node* A_Node);
		void Take_FuncNode(const xmlpp::Node* A_Node);
		void Take_AuthorNode(const xmlpp::Node* A_Node);
		void Take_DescriptionNode(const xmlpp::Node* A_Node);
		void Take_ContactNode(const xmlpp::Node* A_Node);
		void Take_NotesNode(const xmlpp::Node* A_Node);

	friend PluginConf& operator += (PluginConf &OrigConf, const PluginConf &AdditionConf);
};

#endif
