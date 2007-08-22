#ifndef _METARBEAN_C
#define _METARBEAN_C

using namespace std;

#include <vector>
#include <algorithm>			// for lower_bound()
#include <DeafBean.h>


#include "MetarBean.h"
#include "BMetar.h"


MetarBean::MetarBean()
	:	myReports(0)
//		myOrigReports(0)
{
}

MetarBean::~MetarBean()
{
	myReports.clear();
//	myOrigReports.clear();
}

void MetarBean::AddReport(const BMetar &AReport)
{
	myReports.push_back(AReport);
}

size_t MetarBean::MetarCount() const
{
	return(myReports.size());
}

vector<BMetar>::const_iterator MetarBean::Give_Begin() const
{
	return(myReports.begin());
}

const vector<BMetar>::const_iterator MetarBean::Give_End() const
{
	return(myReports.end());
}

/*
vector<string>::const_iterator MetarBean::Give_StrBegin() const
{
        return(myOrigReports.begin());
}

const vector<string>::const_iterator MetarBean::Give_StrEnd() const
{
        return(myOrigReports.end());
}
*/

extern "C" MetarBean* MetarBeanFactory()
{
	return(new MetarBean);
}

extern "C" void MetarBeanDestructor( MetarBean* &p )
{
	if (p != 0)
	{
		delete p;
		p = 0;
	}
}


#endif
