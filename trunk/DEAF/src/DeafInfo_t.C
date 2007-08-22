#ifndef _DEAFINFO_T_C
#define _DEAFINFO_T_C
using namespace std;

#include <iostream>
#include <vector>
#include <dlfcn.h>              // for dlopen(), dlerror(), dlsym()
#include <string>
#include <unistd.h>		// for access()

#include <DeafPlugin.h>
#include <DeafBean.h>
#include <DeafInfo_t.h>

#include <deafUtil.h>		// for GetDeafDirs()

deafInfo_t::deafInfo_t()
	:       myLib_Handle(NULL),
                myPluginFactory(NULL),
                myPluginDestructor(NULL),
                myBeanFactory(NULL),
                myBeanDestructor(NULL),
		myPluginFilename(""),
		myPluginName(""),
		myBeanName("")
{
}



deafInfo_t::deafInfo_t(const deafInfo_t &PluginInfo)
        :       myLib_Handle(PluginInfo.myLib_Handle),
                myPluginFactory(PluginInfo.myPluginFactory),
                myPluginDestructor(PluginInfo.myPluginDestructor),
                myBeanFactory(PluginInfo.myBeanFactory),
                myBeanDestructor(PluginInfo.myBeanDestructor),
		myPluginFilename(PluginInfo.myPluginFilename),
		myPluginName(PluginInfo.myPluginName),
		myBeanName(PluginInfo.myBeanName)
{
}


deafInfo_t::deafInfo_t(const string &PluginFilename, const string &PluginName, const string &BeanName)
	:	myLib_Handle(NULL),
                myPluginFactory(NULL),
                myPluginDestructor(NULL),
                myBeanFactory(NULL),
                myBeanDestructor(NULL),
		myPluginFilename(PluginFilename),
		myPluginName(PluginName),
		myBeanName(BeanName)
{
	LoadPlugin();
}

deafInfo_t::~deafInfo_t()
{
	// Need to research this further...
	// Gotta be careful about destroying, because the copy constructor
	// makes the pointers point to the same stuff.  That can cause weird problems.
}

string deafInfo_t::Give_PluginFilename() const
{
	return(myPluginFilename);
}

string deafInfo_t::Give_PluginName() const
{
	return(myPluginName);
}

string deafInfo_t::Give_BeanName() const
{
	return(myBeanName);
}


// Temporary, until I figure out something better...
create_bean_t* deafInfo_t::Give_BeanFactory()
{
	return(myBeanFactory);
}

// Temporary, until I figure out something better...
destroy_bean_t* deafInfo_t::Give_BeanDestructor()
{
	return(myBeanDestructor);
}


bool deafInfo_t::Is_Open() const
{
        return(!(myLib_Handle == NULL));
}

DeafPlugin* deafInfo_t::PluginFactory() const
{
	return(myPluginFactory());
}

void deafInfo_t::PluginDestructor(DeafPlugin* &APlugin) const
{
	myPluginDestructor(APlugin);
}

DeafBean* deafInfo_t::BeanFactory() const
{
	return(myBeanFactory());
}

void deafInfo_t::BeanDestructor(DeafBean* &ABean) const
{
	myBeanDestructor(ABean);
}

bool deafInfo_t::LoadPlugin(const string &PluginFilename, const string &PluginName, const string &BeanName)
{
	if (PluginFilename.empty())
	{
		cerr << "ERROR: No plug-in file specified for plug-in: " << PluginName << "\n";
		return(false);
	}

	if (Is_Open())
	{
		if (!UnloadPlugin())
		{
			cerr << "ERROR: Could not load plugin " << PluginName << " because old plugin " 
			     << myPluginName << " could not be unloaded!\n";
			return(false);
		}
	}

	if (PluginFilename[0] != '/')		// if it is relative path
	{
		const vector <string> DeafDirs = GetDeafDirs();

                vector<string>::const_iterator ADir = DeafDirs.begin();

                while (ADir != DeafDirs.end() && access((*ADir + '/' + PluginFilename).c_str(), R_OK) != 0)
                {
                        ADir++;
                }

                if (ADir != DeafDirs.end())
                {
                        myPluginFilename = *ADir + '/' + PluginFilename;
                }
		else
		{
			cerr << "ERROR: Could not find the Plugin file to load: " << PluginFilename
			     << " for plugin: " << PluginName << endl;
			return(false);
		}
        }
	else
	{
		myPluginFilename = PluginFilename;
	}

	myPluginName = PluginName;
	myBeanName = BeanName;

	return(LoadPlugin());
}

bool deafInfo_t::LoadPlugin()
{
        dlerror();      // resetting error messages...

        myLib_Handle = dlopen(myPluginFilename.c_str(), RTLD_NOW);    // or RTLD_LAZY ??

        if (!Is_Open())
        {
                cerr << "ERROR: during dlopen(): " << dlerror() << endl;
                return(false);
        }

	//dlerror();

        myPluginFactory = (create_plugin_t*) dlsym(myLib_Handle, (myPluginName + "Factory").c_str());

        char* ErrorMsg = dlerror();
        if (ErrorMsg)
        {
                cerr << "ERROR: during plugin factory loading: " << ErrorMsg << endl;
                dlclose(myLib_Handle);
                return(false);
        }

        dlerror();      // resetting error messages...

        myPluginDestructor = (destroy_plugin_t*) dlsym(myLib_Handle, (myPluginName + "Destructor").c_str());
        ErrorMsg = dlerror();
        if (ErrorMsg)
        {
                cerr << "ERROR: during plugin destructor loading: " << ErrorMsg << endl;
                dlclose(myLib_Handle);
                return(false);
        }

        myBeanFactory = (create_bean_t*) dlsym(myLib_Handle, (myBeanName + "Factory").c_str());
        ErrorMsg = dlerror();
        if (ErrorMsg)
        {
                cerr << "ERROR: during bean destructor loading: " << ErrorMsg << endl;
                dlclose(myLib_Handle);
                return(false);
        }

        myBeanDestructor = (destroy_bean_t*) dlsym(myLib_Handle, (myBeanName + "Destructor").c_str());
        ErrorMsg = dlerror();
        if (ErrorMsg)
        {
                cerr << "ERROR: during bean destructor loading: " << ErrorMsg << endl;
                dlclose(myLib_Handle);
                return(false);
        }

	return(true);
}


bool deafInfo_t::UnloadPlugin()
{
        dlerror();      // resetting error messages...

	// No, this is not a memory leak, the dlclose() should handle getting rid of the functions from memory, right?
	myPluginFactory = NULL;
	myPluginDestructor = NULL;
	myBeanFactory = NULL;
	myBeanDestructor = NULL;

        if (dlclose(myLib_Handle) != 0)
        {
                cerr << "ERROR: Problem while closing the plugin library " << myPluginName << ": " << dlerror() << endl;
                return(false);
        }
        else
        {
		myLib_Handle = NULL;
                return(true);
        }
}

bool deafInfo_t::TestBean() const
{
        DeafBean* PluginBean_Ptr = BeanFactory();

        if (PluginBean_Ptr == NULL)
        {
                return(false);
        }

        BeanDestructor(PluginBean_Ptr);
        return(true);
}

bool deafInfo_t::TestPlugin() const
{
	DeafPlugin* PluginPtr = PluginFactory();

	if (PluginPtr == NULL)
	{
		return(false);
	}

	PluginDestructor(PluginPtr);
	return(true);
}


#endif
