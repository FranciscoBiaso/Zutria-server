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

#ifndef _CONFIG_MANAGER_H
#define _CONFIG_MANAGER_H

#include "definitions.h"

#include <string>

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}


class ConfigManager {
public:
	ConfigManager();
	~ConfigManager();

	enum string_config_t {
		DUMMY_STR = 0,
		CONFIG_FILE,
		DATA_DIRECTORY,
		MAP_FILE,
		MAP_STORE_FILE,
		HOUSE_STORE_FILE,
		HOUSE_RENT_PERIOD,
		MAP_KIND,
		LOGIN_MSG,
		SERVER_NAME,
		WORLD_NAME,
		OWNER_NAME,
		OWNER_EMAIL,
		URL,
		LOCATION,
		IP,
		MOTD,
		PASSWORD_TYPE_STR,
		WORLD_TYPE,
		SQL_HOST,
		SQL_USER,
		SQL_PASS,
		SQL_DB,
		SQL_TYPE,
		MAP_STORAGE_TYPE,
		PREMIUM_ONLY_BEDS,
		LAST_STRING_CONFIG/* this must be the last one */
	};

	enum integer_config_t {
		LOGIN_TRIES = 0,
		RETRY_TIMEOUT,
		LOGIN_TIMEOUT,
		PORT,
		MOTD_NUM,
		MAX_PLAYERS,
		EXHAUSTED,
		EXHAUSTED_ADD,
		FIGHTEXHAUSTED,
		HEALEXHAUSTED,
		PZ_LOCKED,
		FIELD_OWNERSHIP_DURATION,
		MIN_ACTIONTIME,
		MIN_ACTIONEXTIME,		
		DEFAULT_DESPAWNRANGE,
		DEFAULT_DESPAWNRADIUS,
		ALLOW_CLONES,
		RATE_EXPERIENCE,
		RATE_SKILL,
		RATE_LOOT,
		RATE_MAGIC,
		RATE_SPAWN,
		MAX_MESSAGEBUFFER,
		SAVE_CLIENT_DEBUG_ASSERTIONS,
		CHECK_ACCOUNTS,
		PASSWORD_TYPE,
		SQL_PORT,
		STATUSQUERY_TIMEOUT,
		FRAG_TIME,
		IDLE_TIME_KICK,
		IDLE_TIME_WARNING,
		NOTATIONS_TO_BAN,
		WARNINGS_TO_FINALBAN,
		WARNINGS_TO_DELETION,
		BAN_LENGTH,
		FINALBAN_LENGTH,
		IPBANISHMENT_LENGTH,
		LEVEL_TO_ROOK,
		ROOK_TEMPLE_ID,
		WHITE_SKULL_TIME,
		KILLS_TO_RED,
		KILLS_TO_BAN,
		STORE_DEATHS,
		HOUSE_PRICE,
		REMOVE_AMMUNITION,
		REMOVE_RUNE_CHARGES,
		REMOVE_WEAPON_CHARGES,
		USE_ACCBALANCE,
		TEMPLE_TP_ID,
		UH_TRAP,
		KICK_ON_LOGIN,
		TEAM_MODE,
		DAMAGE_PERCENT,
		LAST_INTEGER_CONFIG /* this must be the last one */
	};

	lua_State * getLuaState(){ return L; }
	bool loadFile(const std::string& _filename);
	bool reload();

	void getConfigValue(const std::string& key, lua_State* _L);
	const std::string& getString(uint32_t _what) const;
	int getNumber(uint32_t _what) const;
	bool getBoolean(uint32_t _what) const { return getNumber(_what) != 0; }
	bool setNumber(uint32_t _what, int _value);
	bool setString(uint32_t _what, const std::string& _value);

private:
	static void moveValue(lua_State* fromL, lua_State* toL);
	std::string getGlobalString(lua_State* _L, const std::string& _identifier, const std::string& _default="");
	int getGlobalNumber(lua_State* _L, const std::string& _identifier, int _default=0);
	bool getGlobalBoolean(lua_State* _L, const std::string& _identifier, bool _default=false);

	lua_State* L;
	bool m_isLoaded;
	std::string m_confString[LAST_STRING_CONFIG];
	int m_confInteger[LAST_INTEGER_CONFIG];
};


#endif /* _CONFIG_MANAGER_H */
