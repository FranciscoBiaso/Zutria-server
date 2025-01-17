
//////////////////////////////////////////////////////////////////////
// OpenTibia - an opensource roleplaying game
//////////////////////////////////////////////////////////////////////
// Account class
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

#ifndef __OTSERV_ACCOUNT_H__
#define __OTSERV_ACCOUNT_H__

#include "definitions.h"

#include <list>
#include <string>

class Account
{
public:
  Account();
  ~Account();

  static uint16_t getPremiumDaysLeft(int32_t _premEnd);

  uint32_t accnumber;
  int32_t warnings;
  std::string password;

  // < current time is none, (time_t)(-1) is infinite.
  time_t premEnd;
  std::list<std::string> charList;
};

#endif  // #ifndef __ACCOUNT_H__
