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

#ifndef __OTSERV_CONST_H__
#define __OTSERV_CONST_H__

#include "definitions.h"
#include "Color.h"
#include <utility>  
#include <random>

#define NETWORKMESSAGE_MAXSIZE 16768

//Ranges for ID Creatures
#define PLAYER_ID_RANGE 0x10000000
#define MONSTER_ID_RANGE 0x40000000
#define NPC_ID_RANGE 0x80000000

static const int cMaxViewW = 12;
static const int cMaxViewH = 7;

enum MagicEffectClasses {
	NM_ME_DRAW_BLOOD_MIN   = 0x00,
	NM_ME_MIN_DEF          = 1,
	NM_ME_PUFF			   = 2,
	NM_ME_FIREBALL		   = 0x03,
	NM_ME_EXPLOSION_AREA   = 0x04,
	NM_ME_FIREBALL_GREATEXPLOSION = 0x05,
	NM_ME_FIRESHIELD = 0x06,
	NM_ME_CONJURE_RUNE = 0x07,
	NM_ME_YELLOW_RINGS     = 0x07,
	NM_ME_POISON_RINGS     = 0x08,
	NM_ME_HIT_AREA         = 0x09,
	NM_ME_ENERGY_AREA      = 0x0A, //10
	NM_ME_ENERGY_DAMAGE    = 0x0B, //11
	NM_ME_MAGIC_ENERGY     = 0x0C, //12
	NM_ME_MAGIC_BLOOD      = 0x0D, //13
	NM_ME_MAGIC_POISON     = 0x0E, //14
	NM_ME_MORT_AREA        = 0x0F, //15
	NM_ME_POISON           = 0x10, //16
	NM_ME_HITBY_FIRE       = 0x11, //17
	NM_ME_SOUND_GREEN      = 0x12, //18
	NM_ME_SOUND_RED        = 0x13, //19
	NM_ME_POISON_AREA      = 0x14, //20
	NM_ME_SOUND_YELLOW     = 0x15, //21
	NM_ME_SOUND_PURPLE     = 0x16, //22
	NM_ME_SOUND_BLUE       = 0x17, //23
	NM_ME_SOUND_WHITE      = 0x18, //24
	
	
	NM_ME_DRAW_BLOOD_MEDIUM = 32,
	NM_ME_MAX_PUFF = 33,
	NM_ME_MEDIUM_DEF = 34,
	NM_ME_MAX_DEF = 35,
	NM_ME_DRAW_BLOOD_PLUS_PERFORATION = 37,
	NM_ME_DRAW_BLOOD_MAX = 38,
	NM_ME_DRAW_BLOOD_MEDIUM_PLUS_PERFORATION = 39,
	NM_ME_DRAW_BLOOD_MAX_PLUS_PERFORATION = 40,


	NM_ME_DRAW_BLOOD_PLUS_SLASH = 42,
	NM_ME_DRAW_BLOOD_MEDIUM_PLUS_SLASH = 43,
	NM_ME_DRAW_BLOOD_MAX_PLUS_SLASH = 44,


	NM_ME_DRAW_BLOOD_MIN_PLUS_PERFORATION_MED = 47,
	NM_ME_DRAW_BLOOD_MED_PLUS_PERFORATION_MED = 48,
	NM_ME_DRAW_BLOOD_MAX_PLUS_PERFORATION_MED = 49,


	NM_ME_DRAW_BLOOD_MIN_PLUS_PERFORATION_MAX = 50,
	NM_ME_DRAW_BLOOD_MED_PLUS_PERFORATION_MAX = 51,
	NM_ME_DRAW_BLOOD_MAX_PLUS_PERFORATION_MAX = 52,

	NM_ME_DRAW_BLOOD_PLUS_SLASH_MED = 55,
	NM_ME_DRAW_BLOOD_MEDIUM_PLUS_SLASH_MED = 56,
	NM_ME_DRAW_BLOOD_MAX_PLUS_SLASH_MED = 57,


	NM_ME_DRAW_BLOOD_PLUS_SLASH_MAX = 58,
	NM_ME_DRAW_BLOOD_MEDIUM_PLUS_SLASH_MAX = 59,
	NM_ME_DRAW_BLOOD_MAX_PLUS_SLASH_MAX = 60,


	NM_ME_TRAUMA = 61,


	NM_ME_DRAW_BLEEDING_MIN = 62,

	NM_ME_DRAW_BLOOD_MEGA = 63,

	NM_ME_MIN_DEF_PLUS_PERFORATION_MIN = 65,
	NM_ME_MIN_DEF_PLUS_PERFORATION_MEDIUM = 66,
	NM_ME_MIN_DEF_PLUS_PERFORATION_MAX = 67,

	NM_ME_MEDIUM_DEF_PLUS_PERFORATION_MIN = 68,
	NM_ME_MEDIUM_DEF_PLUS_PERFORATION_MEDIUM = 69,
	NM_ME_MEDIUM_DEF_PLUS_PERFORATION_MAX= 70,

	NM_ME_MAX_DEF_PLUS_PERFORATION_MIN = 71,
	NM_ME_MAX_DEF_PLUS_PERFORATION_MEDIUM = 72,
	NM_ME_MAX_DEF_PLUS_PERFORATION_MAX = 73,
	
	//for internal use, dont send to client

	NM_ME_NONE             = 0xFF,
	NM_ME_UNK              = 0xFFFF
};

enum ShootType_t {
	NM_SHOOT_SPEAR			= 0,
	NM_SHOOT_BOLT           = 1,
	NM_SHOOT_ARROW          = 2,
	NM_SHOOT_FIRE			= 3,
	NM_SHOOT_ENERGY         = 4,
	NM_SHOOT_POISONARROW    = 5,
	NM_SHOOT_BURSTARROW     = 6,
	NM_SHOOT_THROWINGSTAR   = 7,
	NM_SHOOT_THROWINGKNIFE  = 8,
	NM_SHOOT_SMALLSTONE     = 9,
	NM_SHOOT_SUDDENDEATH    = 10,
	NM_SHOOT_LARGEROCK      = 11,
	NM_SHOOT_SNOWBALL       = 12,
	NM_SHOOT_POWERBOLT      = 13,
	NM_SHOOT_POISONFIELD    = 14,
	//for internal use, dont send to client
	NM_SHOOT_NONE			= 255,
	NM_SHOOT_UNK			= 0xFFFF
};

enum MessageGUITarget{
	MSG_TARGET_CONSOLE = 1,
	MSG_TARGET_TOP_CENTER_MAP = 2,
	MSG_TARGET_BOTTOM_CENTER_MAP = 4
};

enum MessageClasses {
	MSG_PLAYER_TALK = 1,
	MSG_PLAYER_WHISPER = 2,
	MSG_PLAYER_YELL = 3,
	MSG_PLAYER_PRIVATE_FROM = 4,
	MSG_PLAYER_PRIVATE_TO = 5,
	MSG_BROADCAST = 6,
	MSG_MONSTER_TALK = 7,
	MSG_MONSTER_YELL = 8,
	MSG_PLAYER_LEVELUP = 9,
	MSG_INFORMATION = 10,
	MSG_MUTED = 11,
	MSG_LOOK = 12,
};

enum MessageColors{
	MSG_COLOR_RED = 1,
	MSG_COLOR_DARK_RED,
	MSG_COLOR_LIGHT_RED,
	MSG_COLOR_GREEN,
	MSG_COLOR_DARK_GREEN,
	MSG_COLOR_LIGHT_GREEN,
	MSG_COLOR_BLUE,
	MSG_COLOR_DARK_BLUE,
	MSG_COLOR_LIGHT_BLUE,
	MSG_COLOR_ORGANE,
	MSG_COLOR_DARK_ORANGE,
	MSG_COLOR_LIGHT_ORGANE,
	MSG_COLOR_YELLOW,
	MSG_COLOR_DARK_YELLOW,
	MSG_COLOR_LIGHT_YELLOW,
	MSG_COLOR_BLACK,
	MSG_COLOR_WHITE
};

enum FluidClasses {
	FLUID_EMPTY   = 0x00,     //note: class = fluid_number mod 8
	FLUID_GREEN  = 0x01,
	FLUID_BLUE = 0x02,
	FLUID_YELLOW = 0x03,
	FLUID_RED   = 0x04,
	FLUID_WHITE = 0x05,
	FLUID_BROWN = 0x06,
	FLUID_PURPLE  = 0x07
};

enum e_fluids {	
	FLUID_WATER	= FLUID_BLUE,
	FLUID_BLOOD	= FLUID_RED,
	FLUID_BEER = FLUID_BROWN,
	FLUID_SLIME = FLUID_GREEN,
	FLUID_LEMONADE = FLUID_YELLOW,
	FLUID_MILK = FLUID_WHITE,
	FLUID_MANAFLUID = FLUID_PURPLE,
	FLUID_LIFEFLUID= FLUID_RED+8,
	FLUID_OIL = FLUID_BROWN+8,
	FLUID_WINE = FLUID_PURPLE+8
};

const uint8_t reverseFluidMap[] = {
	FLUID_EMPTY,
	FLUID_WATER,
	FLUID_MANAFLUID,
	FLUID_BEER,
	FLUID_EMPTY,
	FLUID_BLOOD,
	FLUID_SLIME,
	FLUID_EMPTY,
	FLUID_LEMONADE,
	FLUID_MILK
};

const uint8_t fluidMap[] = {
	FLUID_EMPTY,
	FLUID_BLUE,
	FLUID_RED,
	FLUID_BROWN,
	FLUID_EMPTY,
	FLUID_BLOOD,
	FLUID_GREEN,
	FLUID_EMPTY,
	FLUID_YELLOW,
	FLUID_WHITE
};

enum SquareColor_t {
	SQ_COLOR_NONE = 256,
	SQ_COLOR_BLACK = 0
};

enum TextColor_t {
	TEXTCOLOR_BLUE        = 5,
	TEXTCOLOR_LIGHTBLUE   = 35,
	TEXTCOLOR_LIGHTGREEN  = 30,
	TEXTCOLOR_LIGHTGREY   = 129,
	TEXTCOLOR_LIGHT_RED   = 144,
	TEXTCOLOR_RED         = 180,
	TEXTCOLOR_ORANGE      = 198,
	TEXTCOLOR_YELLOW       = 212,
	TEXTCOLOR_LIGHT_YELLOW   = 211,
	TEXTCOLOR_WHITE_EXP   = 215,
	TEXTCOLOR_DARK_BLUE   = 3,
	TEXTCOLOR_GRAY        = 172,
	TEXTCOLOR_PURPLE        = 173,
	TEXTCOLOR_NONE        = 255,
	TEXTCOLOR_UNK         = 256,
};

enum Icons_t{
	ICON_POISON = 1,
	ICON_BURN = 2,
	ICON_ENERGY =  4,
	ICON_DRUNK = 8,
	ICON_MANASHIELD = 16,
	ICON_PARALYZE = 32,
	ICON_HASTE = 64,
	ICON_SWORDS = 128
};

enum WeaponType_t {
	WEAPON_TYPE_NONE = 0,
	WEAPON_TYPE_MELEE,
	WEAPON_TYPE_DISTANCE,
	WEAPON_TYPE_SHIELD,
	WEAPON_TYPE_AMMO
};

enum Ammo_t {
	AMMO_NONE = 0,
	AMMO_BOLT = 1,
	AMMO_ARROW = 2,
	AMMO_SPEAR = 3,
	AMMO_THROWINGSTAR = 4,
	AMMO_THROWINGKNIFE = 5,
	AMMO_STONE = 6,
	AMMO_SNOWBALL = 7
};

enum AmmoAction_t{
    AMMOACTION_NONE,
    AMMOACTION_REMOVECOUNT,
    AMMOACTION_REMOVECHARGE,
    AMMOACTION_MOVE,
    AMMOACTION_MOVEBACK
};

enum WieldInfo_t{
    WIELDINFO_LEVEL = 1,
    WIELDINFO_MAGLV = 2,
    WIELDINFO_VOCREQ = 4,
    WIELDINFO_PREMIUM = 8
};

enum Skulls_t{
	SKULL_NONE = 0,
	SKULL_YELLOW = 1,
	SKULL_GREEN = 2,
	SKULL_WHITE = 3,
	SKULL_RED = 4
};

enum PartyShields_t{
    SHIELD_NONE = 0,
    SHIELD_WHITEYELLOW = 1,
    SHIELD_WHITEBLUE = 2,
    SHIELD_BLUE = 3,
    SHIELD_YELLOW = 4
};

enum item_t {
    ITEM_FIREFIELD_PVP    = 1492,
    ITEM_FIREFIELD_NOPVP  = 1500,
    
    ITEM_POISONFIELD_PVP    = 1496,
    ITEM_POISONFIELD_NOPVP  = 1503,
    
    ITEM_ENERGYFIELD_PVP    = 1495,
    ITEM_ENERGYFIELD_NOPVP  = 1504, 
     
	ITEM_COINS_GOLD       = 2148,
	ITEM_COINS_PLATINUM   = 2152,
	ITEM_COINS_CRYSTAL    = 2160,

	ITEM_DEPOT            = 2594,
	ITEM_LOCKER1          = 2589,

	ITEM_MALE_CORPSE      = 515,
	ITEM_FEMALE_CORPSE    = 3065,

	ITEM_MEAT             = 2666,
	ITEM_HAM              = 2671,
	ITEM_GRAPE            = 2681,
	ITEM_APPLE            = 2674,
	ITEM_BREAD            = 2689,
	ITEM_ROLL             = 2690,
	ITEM_CHEESE           = 2696,

	ITEM_FULLSPLASH       = 2233,
	ITEM_SMALLSPLASH      = 2234,

	ITEM_PARCEL           = 2595,
	ITEM_PARCEL_STAMPED   = 2596,
	ITEM_LETTER           = 2597,
	ITEM_LETTER_STAMPED   = 2598,
	ITEM_LABEL            = 2599,

	ITEM_DOCUMENT_RO      = 1968 //read-only
};

enum PlayerFlags{
	//Add the flag's numbers to get the groupFlags number you need
	PlayerFlag_CannotUseCombat = 0,			//2^0 = 1
	PlayerFlag_CannotAttackPlayer,			//2^1 = 2
	PlayerFlag_CannotAttackMonster,			//2^2 = 4
	PlayerFlag_CannotBeAttacked,			//2^3 = 8
	PlayerFlag_CanConvinceAll,				//2^4 = 16
	PlayerFlag_CanSummonAll,				//2^5 = 32
	PlayerFlag_CanIllusionAll,				//2^6 = 64
	PlayerFlag_CanSenseInvisibility,		//2^7 = 128
	PlayerFlag_IgnoredByMonsters,			//2^8 = 256
	PlayerFlag_NotGainInFight,				//2^9 = 512
	PlayerFlag_HasInfiniteMana,				//2^10 = 1024
	PlayerFlag_HasInfiniteSoul,				//2^11 = 2048
	PlayerFlag_HasNoExhaustion,				//2^12 = 4096
	PlayerFlag_CannotUseSpells,				//2^13 = 8192
	PlayerFlag_CannotPickupItem,			//2^14 = 16384
	PlayerFlag_CanAlwaysLogin,				//2^15 = 32768
	PlayerFlag_CanBroadcast,				//2^16 = 65536
	PlayerFlag_CanEditHouses,				//2^17 = 131072
	PlayerFlag_CannotBeBanned,				//2^18 = 262144
	PlayerFlag_CannotBePushed,				//2^19 = 524288
	PlayerFlag_HasInfiniteCapacity,			//2^20 = 1048576
	PlayerFlag_CanPushAllCreatures,			//2^21 = 2097152
	PlayerFlag_CanTalkRedPrivate,			//2^22 = 4194304
	PlayerFlag_CanTalkRedChannel,			//2^23 = 8388608
	PlayerFlag_TalkOrangeHelpChannel,		//2^24 = 16777216
	PlayerFlag_NotGainExperience,			//2^25 = 33554432
	PlayerFlag_NotGainMana,					//2^26 = 67108864
	PlayerFlag_NotGainHealth,				//2^27 = 134217728
	PlayerFlag_NotGainSkill,				//2^28 = 268435456
	PlayerFlag_SetMaxSpeed,					//2^29 = 536870912
	PlayerFlag_SpecialVIP,					//2^30 = 1073741824
	PlayerFlag_NotGenerateLoot,				//2^31 = 2147483648
	PlayerFlag_CanTalkRedChannelAnonymous,  //2^32 = 4294967296
	PlayerFlag_IgnoreProtectionZone,        //2^33 = 8589934592
	PlayerFlag_IgnoreSpellCheck,            //2^34 = 17179869184
	PlayerFlag_IgnoreWeaponCheck,           //2^35 = 34359738368
	PlayerFlag_CannotBeMuted,               //2^36 = 68719476736
	PlayerFlag_IsAlwaysPremium,             //2^37 = 137438953472
	PlayerFlag_CanAnswerRuleViolations,     //2^38 = 274877906944
	PlayerFlag_CanReloadContent,            //2^39 = 549755813888
	PlayerFlag_ShowGroupInsteadOfVocation,  //2^40 = 1099511627776
	PlayerFlag_CantChangeOutfit,			  //2^41 = 2199023255552
	//PlayerFlag_CannotMoveItems,             //2^42 = 4398046511104
	//PlayerFlag_CannotMoveCreatures,         //2^43 = 8796093022208
	//PlayerFlag_CanReportBugs,               //2^44 = 17592186044416
	PlayerFlag_CanSeeSpecialDescription,    //2^45 = 35184372088832
	//PlayerFlag_CannotBeSeen,                //2^46 = 70368744177664
	//PlayerFlag_HideHealth,                  //2^47 = 140737488355328
	//PlayerFlag_CanPassThroughAllCreatures,  //2^48 = 281474976710656
	//add new flags here
	PlayerFlag_LastFlag
};

// player attributes gain per level
// health points, physical attack, physical defense, capacity
// mana points, magical attack, magical defense, magic points,
// player speed, attack speed, cooldown, avoidance
namespace _player_{
	const int max_creatures_to_block = 8;

	const int attributes[][12] =
	{
		{ 2, 1, 1, 2, 7, 5, 4, 2, 2, 1, 2, 1 }, // fire mage
		{ 2, 1, 1, 2, 7, 5, 3, 2, 3, 1, 2, 1 }, // eletrician mage
		{ 3, 4, 3, 3, 3, 2, 1, 1, 3, 2, 1, 4 }, // archer
		{ 5, 5, 4, 6, 2, 1, 1, 1, 1, 1, 1, 2 }, // knight
		{ 2, 1, 1, 2, 7, 4, 5, 2, 2, 1, 2, 1 }, // druid
		{ 2, 1, 1, 2, 6, 4, 5, 2, 2, 1, 3, 1 }, // sumoner
	};
	
	//10,20,30,40,50,60,70,80,90,100%
	const int attackSpeedDivision[] = { 100, 200, 300, 400, 500, 600, 700, 800, 900 ,1000};
	
	//player spells

#define NUMBER_OF_SPELLS 1

	struct spell
	{	
		int magicPoints; //magic points for lvl 1,2,3, ...
		std::string dependentSpell;
		uint8_t maxLevel;

		spell()
		{

		}
		spell(int maxlevel, int magicPoints, std::string dependentSpell)
		{
			// 3 levels
			this->magicPoints = magicPoints;
			this->dependentSpell = dependentSpell; //-1 no dependency
			this->maxLevel = maxlevel;
		}
		
	};

	typedef std::pair<std::string, spell> spellPair;
	
	const spellPair g_map_table[] = {
		spellPair("teleporte inflamável", spell( 2, 14 , "no dependency")),
		spellPair("língua de fogo", spell( 2, 20 , "Bola de fogo")),
		spellPair("bola de fogo", spell(3, 12, "Bola de fogo")),
		spellPair("onda sísmica", spell(2, 23, "Bola de fogo")),
		spellPair("chuva de meteoros", spell(2, 23, "Bola de fogo")),
		spellPair("fogo vivo", spell(2, 23, "Bola de fogo")),
		spellPair("descarga elétrica", spell(2, 23, "Bola de fogo")),
		spellPair("choque fulminante", spell(2, 23, "Bola de fogo")),
		spellPair("onda elétrica", spell(2, 23, "Bola de fogo")),
		spellPair("raio", spell(2, 23, "Bola de fogo")),
		spellPair("ciclone elétrico", spell(2, 23, "Bola de fogo")),
		spellPair("potência energética", spell(2, 23, "Bola de fogo")),
		spellPair("x", spell(2, 23, "Bola de fogo")),
		spellPair("x", spell(2, 23, "Bola de fogo")),
		spellPair("x", spell(2, 23, "Bola de fogo")),//15
		spellPair("x", spell(2, 23, "Bola de fogo")),
		spellPair("x", spell(2, 23, "Bola de fogo")),
		spellPair("x", spell(2, 23, "Bola de fogo")),
		spellPair("x", spell(2, 23, "Bola de fogo")),
		spellPair("x", spell(2, 23, "Bola de fogo")),//20
		spellPair("barreira de galhos", spell(2, 23, "Bola de fogo")),

	};

	extern std::map<std::string, struct spell> g_spellsTree;	
};

struct _weaponDamage_ {
	int32_t damageByTrauma;
	int32_t traumaFactor;
	int32_t damageByPerforation;
	int32_t perforationFactor;
	double perforationFactorPercentage;
	int32_t	damageBySlash;
	int32_t slashFactor;
	int32_t totalDamage;
	int damageType;
	bool critic;
	int32_t criticDmg;
};


#define inifity 2e15

//only one definition
static std::default_random_engine engine;
static auto dice_00_10 = std::bind(std::uniform_real_distribution<double> (0, 1), engine);
static auto dice_01_75 = std::bind(std::uniform_real_distribution<double>(0.1, 0.75), engine);
static auto dice_02_45 = std::bind(std::uniform_real_distribution<double>(0.20, 0.45), engine);
static auto dice_05_10 = std::bind(std::uniform_real_distribution<double> (0.05,0.10), engine);
static auto dice_04_09 = std::bind(std::uniform_real_distribution<double>(0.04,0.09), engine);
static auto dice_04_085 = std::bind(std::uniform_real_distribution<double>(0.04, 0.085), engine);
static auto dice_04_075 = std::bind(std::uniform_real_distribution<double>(0.04, 0.075), engine);
static auto dice_04_065 = std::bind(std::uniform_real_distribution<double>(0.04, 0.065), engine);
static auto dice_25_10 = std::bind(std::uniform_real_distribution<double>(0.25, 1), engine);
static auto dice_35_10 = std::bind(std::uniform_real_distribution<double>(0.35, 1), engine);
static auto dice_35_055 = std::bind(std::uniform_real_distribution<double>(0.35, 0.55), engine);
static auto dice_07_10 = std::bind(std::uniform_real_distribution<double> (0.7, 1.0), engine);

//Reserved player storage key ranges
//[10000000 - 20000000]
#define PSTRG_RESERVED_RANGE_START  10000000
#define PSTRG_RESERVED_RANGE_SIZE   10000000
//[1000 - 1500]
#define PSTRG_OUTFITS_RANGE_START   (PSTRG_RESERVED_RANGE_START + 1000)
#define PSTRG_OUTFITS_RANGE_SIZE    500

#define IS_IN_KEYRANGE(key, range) (key >= PSTRG_##range##_START && ((key - PSTRG_##range##_START) < PSTRG_##range##_SIZE))

#endif
