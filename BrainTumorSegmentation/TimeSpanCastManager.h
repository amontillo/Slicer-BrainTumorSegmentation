#pragma once
#include <string>
#include <boost/lexical_cast.hpp>

using namespace std;
using boost::lexical_cast;


namespace TimeSpanCastManager
{
	// note in C++ static const ints can be defined in class, no separate cpp file req'd
	static const int HOURS_IN_DAY = 24;    // it's good form to make
	static const int MINUTES_IN_HOUR = 60; // symbolic constants all caps
	static const int SECONDS_IN_MINUTE = 60;


	static string timespan_cast(double dTotalSeconds)
		{
			double dTotalSeconds_=dTotalSeconds;

			// compute nSeconds
			int nSeconds = ((long)(dTotalSeconds)) % SECONDS_IN_MINUTE ;

			// throw away nSeconds used in previous statement and convert to nMinutes_
			long longMinutes = (long)(dTotalSeconds / (double)(SECONDS_IN_MINUTE)) ;

			// compute  nMinutes_
			int nMinutes_ = longMinutes % MINUTES_IN_HOUR ;
        
			// throw away nMinutes_ used in previous statement and convert to nHours_
			long longHours = longMinutes / MINUTES_IN_HOUR ;

			// compute nHours_
			int nHours_ = longHours % HOURS_IN_DAY ;

			// throw away nHours_ used in previous statement and convert to nDays_
			int nDays_ = longHours / HOURS_IN_DAY ;

			double dSeconds_ = dTotalSeconds - ((nMinutes_*SECONDS_IN_MINUTE)+(nHours_*MINUTES_IN_HOUR*SECONDS_IN_MINUTE)+(nDays_*HOURS_IN_DAY*MINUTES_IN_HOUR*SECONDS_IN_MINUTE) );

			string strString =  lexical_cast<string>(nDays_) + "." + lexical_cast<string>(nHours_) + ":" + lexical_cast<string>(nMinutes_) + ":" + lexical_cast<string>(dSeconds_);  

			return strString;
		}

};

