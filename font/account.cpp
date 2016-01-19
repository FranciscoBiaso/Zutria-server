//////////////////////////////////////////////////////////////////////
// OpenTibia - an opensource roleplaying game
//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//////////////////////////////////////////////////////////////////////

#include "otpch.h"

#include "account.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <cmath>

Account::Account()
{
	accnumber = 0;
	premEnd = 0;
}

Account::~Account()
{
	charList.clear();
}

uint16_t Account::getPremiumDaysLeft(int32_t _premEnd)
{

	struct tm y2k = { 0 };
	double seconds;
	y2k.tm_hour = 0;   y2k.tm_min = 0; y2k.tm_sec = 0;
	y2k.tm_year = 115; y2k.tm_mon = 10-1; y2k.tm_mday = 1;
	time_t timer = time(NULL);  /* get current time; same as: timer = time(NULL)  */
	
	seconds = difftime(timer, mktime(&y2k));

	return seconds / 86400;

	if (_premEnd < time(NULL)){
		return 0;
	}


	return (uint16_t)std::ceil(((double)(_premEnd - time(NULL))) / 86400.);
}
