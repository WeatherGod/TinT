#ifndef _MOSBEAN_C
#define _MOSBEAN_C

using namespace std;

#include <vector>
#include <DeafBean.h>


#include "MOSBean.h"
#include <BMOS.h>


MOSBean::MOSBean()
	:	myReports(0)
{
}

MOSBean::~MOSBean()
{
	myReports.clear();
}

void MOSBean::AddReport(const BMOS &AReport)
{
	myReports.push_back(AReport);
}

size_t MOSBean::MOSCount() const
{
	return(myReports.size());
}

vector<BMOS>::const_iterator MOSBean::Give_Begin() const
{
	return(myReports.begin());
}

const vector<BMOS>::const_iterator MOSBean::Give_End() const
{
	return(myReports.end());
}


extern "C" MOSBean* MOSBeanFactory()
{
	return(new MOSBean);
}

extern "C" void MOSBeanDestructor( MOSBean* &p )
{
	if (p != 0)
	{
		delete p;
		p = 0;
	}
}


#endif
