#ifndef _METARBEAN_H
#define _METARBEAN_H


#include <vector>

#include <DeafBean.h>
#include "BMetar.h"

class MetarBean : public DeafBean
//Extends the DeafBean base class
{
	public:
		MetarBean();
		virtual ~MetarBean();

		void AddReport(const BMetar &AReport);

		size_t MetarCount() const;

		vector<BMetar>::const_iterator Give_Begin() const;
		const vector<BMetar>::const_iterator Give_End() const;

//		vector<string>::const_iterator Give_StrBegin() const;
//		const vector<string>::const_iterator Give_StrEnd() const;

		
	private:
		vector <BMetar> myReports;
//		vector <string> myOrigReports;

};

#endif
