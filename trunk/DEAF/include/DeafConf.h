#ifndef _DEAFCONF_H
#define _DEAFCONF_H

#include <string>
#include <map>

#include <libxml++/libxml++.h>

#include <PluginConf.h>

class DeafConf_t
{
	public:
		DeafConf_t();
		DeafConf_t(const DeafConf_t &OtherConfig);
		DeafConf_t(const string &ConfFilename);
		~DeafConf_t();

		bool LoadConfiguration(const string &ConfFilename);
		bool LoadConfiguration(const xmlpp::Node* ConfigNode);

		bool IsValid() const;
		bool LoadRegistration(const string &PluginName);

		bool Func_Avail(const string &PluginName, const string &FunctionName) const;
		vector <string> Give_PluginNames() const;
		string Give_RegistrationFilename(const string &PluginName) const;
		string Give_PluginFilename(const string &PluginName) const;
		string Give_BeanName(const string &PluginName) const;

		string Give_InputHost(const string &PluginName) const;
		string Give_InputUser(const string &PluginName) const;
		unsigned int Give_InputHostPort(const string &PluginName) const;
		string Give_OutputHost(const string &PluginName) const;
		string Give_OutputUser(const string &PluginName) const;
		unsigned int Give_OutputHostPort(const string &PluginName) const;

		string Give_OutputFilename(const string &PluginName) const;
		string Give_FileInputStr(const string &PluginName) const;
		string Give_FileOutputStr(const string &PluginName) const;

		void ListPlugins();
		bool ListPlugin(const string &PluginName, const bool &DoDetailed);

	private:
		map <string, PluginConf> myPluginConfs;
		bool myIsValid;

		bool LoadRegistration(const xmlpp::Node* PluginNode);
};

#endif
