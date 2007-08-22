#ifndef _MOSBEAN_H
#define _MOSBEAN_H


#include <vector>

#include <DeafBean.h>
#include "BMOS.h"

class MOSBean : public DeafBean
//Extends the DeafBean base class
{
	public:
		MOSBean();
		virtual ~MOSBean();

		void AddReport(const BMOS &AReport);

		size_t MOSCount() const;

		vector<BMOS>::const_iterator Give_Begin() const;
		const vector<BMOS>::const_iterator Give_End() const;

//		vector<string>::const_iterator Give_StrBegin() const;
//		const vector<string>::const_iterator Give_StrEnd() const;

		
	private:
		vector <BMOS> myReports;
//		vector <string> myOrigReports;

};

#endif
