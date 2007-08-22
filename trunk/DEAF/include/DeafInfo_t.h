#ifndef _DEAFINFO_T_H
#define _DEAFINFO_T_H

#include <string>

#include <DeafPlugin.h>
#include <DeafBean.h>

typedef DeafPlugin* create_plugin_t();
typedef void destroy_plugin_t(DeafPlugin*&);

typedef DeafBean* create_bean_t();
typedef void destroy_bean_t(DeafBean*&);


class deafInfo_t
{
        public:
                deafInfo_t();
		deafInfo_t(const string &PluginFilename, const string &PluginName, const string &BeanName);
                deafInfo_t(const deafInfo_t &PluginInfo);
		~deafInfo_t();

		DeafPlugin* PluginFactory() const;
		void PluginDestructor(DeafPlugin* &APlugin) const;
		DeafBean* BeanFactory() const;
		void BeanDestructor(DeafBean* &ABean) const;

		string Give_PluginName() const;
		string Give_PluginFilename() const;
		string Give_BeanName() const;

		// Temporary, until I figure something better.
		// It really isn't a good idea to be passing
		// around these pointers outside the class.
		create_bean_t* Give_BeanFactory();
		destroy_bean_t* Give_BeanDestructor();

		bool Is_Open() const;
		bool LoadPlugin(const string &PluginFilename, const string &PluginName, const string &BeanName);
		bool UnloadPlugin();

		bool TestBean() const;
		bool TestPlugin() const;

	private:
		void* myLib_Handle;
		create_plugin_t* myPluginFactory;
                destroy_plugin_t* myPluginDestructor;
                create_bean_t* myBeanFactory;
                destroy_bean_t* myBeanDestructor;
		string myPluginFilename;
		string myPluginName;
		string myBeanName;

		bool LoadPlugin();
};

#endif
