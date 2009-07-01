#ifndef _WACUTIL_C
#define _WACUTIL_C
using namespace std;

#include <iostream>
#include <cstdlib>	// for getenv()
#include <unistd.h>	// for gethostname()

#include <string>

#include "WACUtil.h"

#ifndef _WAC_DATABASE_NAME_
#define _WAC_DATABASE_NAME_ "WX_ARCHIVE_CATALOG"
#endif

#ifndef _WAC_ADMIN_SYS_
#define _WAC_ADMIN_SYS_ "ls2.meteo.psu.edu"
#endif

#ifndef _WAC_ADMIN_NAME_
#define _WAC_ADMIN_NAME_ "wxarchadmin"
#endif

#ifndef _WAC_USER_NAME_
#define _WAC_USER_NAME_ "wxarchuser"
#endif

// seems like some systems do not have this defined, for some reason...
#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 65
#endif

string GetWACDir()
{
        string TempHold = getenv("WAC_HOME");
        if (TempHold.empty())
        {
                cerr << "WARNING!  WAC_HOME environment variable is not set.  Using '.'" << endl;
                return(".");
        }
        else
        {
                return(TempHold);
        }
}

string GetDatabaseName()
{
	return(_WAC_DATABASE_NAME_);
}


string GetAdminServerName()
{
	return(_WAC_ADMIN_SYS_);
}

bool OnAdminSystem()
// This is probably not the best way to do this.
// It would be nice to resolve the name so that I can get a more definitive match.
{
        char SysName[HOST_NAME_MAX + 1];
	memset(SysName, '\0', HOST_NAME_MAX + 1);
	
	if (gethostname(SysName, HOST_NAME_MAX) == -1)
	{
		cerr << "ERROR: Unable to determine this system's name." << endl;
		return(false);
	}

        if (strcmp(SysName, GetAdminServerName().c_str()) == 0)
	{
		return(true);
	}
	else
	{
		return(false);
	}
}

string GetAdminUserName()
{
	return(_WAC_ADMIN_NAME_);
}

string GetWACUserName()
{
	return(_WAC_USER_NAME_);
}

#endif
