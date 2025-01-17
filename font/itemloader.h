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

#ifndef __OTSERV_ITEMLOADER_H__
#define __OTSERV_ITEMLOADER_H__

#include "definitions.h"
#include "fileloader.h"

typedef uint8_t attribute_t;
typedef uint16_t datasize_t;
typedef uint32_t flags_t;
enum itemgroup_t{

	ITEM_GROUP_NONE = 0,
	ITEM_GROUP_GROUND,
	ITEM_GROUP_CONTAINER,
	ITEM_GROUP_WEAPON,
	ITEM_GROUP_AMMUNITION,
	ITEM_GROUP_ARMOR,
	ITEM_GROUP_CHANGES,
	ITEM_GROUP_TELEPORT,
	ITEM_GROUP_MAGICFIELD,
	ITEM_GROUP_WRITEABLE,
	ITEM_GROUP_KEY,
	ITEM_GROUP_SPLASH,
	ITEM_GROUP_FLUID,
	ITEM_GROUP_DOOR,
	ITEM_GROUP_DEPRECATED,
	ITEM_GROUP_LAST
};

/////////OTB specific//////////////
enum clientVersion_t{
	CLIENT_VERSION_740 = 1,
	CLIENT_VERSION_750 = 1,
	CLIENT_VERSION_755 = 2,
	CLIENT_VERSION_760 = 3,
	CLIENT_VERSION_770 = 3,
	CLIENT_VERSION_780 = 4,
	CLIENT_VERSION_790 = 5,
	CLIENT_VERSION_792 = 6,
	CLIENT_VERSION_800 = 7
};

enum rootattrib_t{
	ROOT_ATTR_VERSION = 0x01
};



enum itemattrib_t : unsigned char {
	ITEM_ATTR_SERVERID  = 0x10,
	ITEM_ATTR_CLIENTID = 0x11,
	ITEM_ATTR_NAME = 0x12,
	ITEM_ATTR_SPEED = 0x14,
	ITEM_ATTR_HASH = 0x20,
	ITEM_ATTR_MINMAPCOLOR = 0x21,
	ITEM_ATTR_MaxReadWriteChars = 0x22,
	ITEM_ATTR_MaxReadChars = 0x23,
	ITEM_ATTR_LIGHT = 0x2A,
	ITEM_ATTR_StackOrder = 0x2B,
	ITEM_ATTR_TradeAs = 0x2C,
	ITEM_ATTR_ELEVATION = 0x2D,
	ITEM_ATTR_LAST
};

enum itemflags_t {
	FLAG_BLOCK_SOLID = 1,
	FLAG_BLOCK_PROJECTILE = 2,
	FLAG_BLOCK_PATHFIND = 4,
	FLAG_HAS_HEIGHT = 8,
	FLAG_USEABLE = 16,
	FLAG_PICKUPABLE = 32,
	FLAG_MOVEABLE = 64,
	FLAG_STACKABLE = 128,
	FLAG_FLOORCHANGEDOWN = 256,
	FLAG_FLOORCHANGENORTH = 512,
	FLAG_FLOORCHANGEEAST = 1024,
	FLAG_FLOORCHANGESOUTH = 2048,
	FLAG_FLOORCHANGEWEST = 4096,
	FLAG_ALWAYSONTOP = 8192,
	FLAG_READABLE = 16384,
	FLAG_ROTABLE = 32768,
	FLAG_HANGABLE = 65536,
	FLAG_VERTICAL = 131072,
	FLAG_HORIZONTAL = 262144,
	FLAG_CANNOTDECAY = 524288,
	FLAG_ALLOWDISTREAD = 1048576,
	___Unused = 1 << 21,
	___ClientCharges = 1 << 22,
	___IgnoreLook = 1 << 23,
	___IsAnimation = 1 << 24,
	___FullGround = 1 << 25,
	___ForceUse = 1 << 26
};
/*
enum slotsOTB_t{
	OTB_SLOT_DEFAULT,
	OTB_SLOT_HEAD,
	OTB_SLOT_BODY,
	OTB_SLOT_LEGS,
	OTB_SLOT_BACKPACK,
	OTB_SLOT_WEAPON,
	OTB_SLOT_2HAND,
	OTB_SLOT_FEET,
	OTB_SLOT_AMULET,
	OTB_SLOT_RING,
	OTB_SLOT_HAND,
};

enum ShootTypeOtb_t {
	OTB_SHOOT_NONE          = 0,
	OTB_SHOOT_BOLT          = 1,
	OTB_SHOOT_ARROW         = 2,
	OTB_SHOOT_FIRE          = 3,
	OTB_SHOOT_ENERGY        = 4,
	OTB_SHOOT_POISONARROW   = 5,
	OTB_SHOOT_BURSTARROW    = 6,
	OTB_SHOOT_THROWINGSTAR  = 7,
	OTB_SHOOT_THROWINGKNIFE = 8,
	OTB_SHOOT_SMALLSTONE    = 9,
	OTB_SHOOT_SUDDENDEATH   = 10,
	OTB_SHOOT_LARGEROCK     = 11,
	OTB_SHOOT_SNOWBALL      = 12,
	OTB_SHOOT_POWERBOLT     = 13,
	OTB_SHOOT_SPEAR         = 14,
	OTB_SHOOT_POISONFIELD   = 15,
	OTB_SHOOT_INFERNALBOLT  = 16
};
*/
//1-byte aligned structs
#pragma pack(1)

struct VERSIONINFO{
	uint32_t dwMajorVersion;
	uint32_t dwMinorVersion;
	uint32_t dwBuildNumber;
	uint8_t CSDVersion[128];
};
/*
struct decayBlock2{
	uint16_t decayTo;
	uint16_t decayTime;
};

struct weaponBlock2{
	uint8_t weaponType;
	uint8_t amuType;
	uint8_t shootType;
	uint8_t attack;
	uint8_t defence;
};

struct amuBlock2{
	uint8_t amuType;
	uint8_t shootType;
	uint8_t attack;
};

struct armorBlock2{
	uint16_t armor;
	double weight;
	uint16_t slot_position;
};

struct writeableBlock2{
	uint16_t readOnlyId;
};
*/
struct lightBlock2{
	uint16_t lightLevel;
	uint16_t lightColor;
};
/*
struct writeableBlock3{
	uint16_t readOnlyId;
	uint16_t maxTextLen;
};
*/
#pragma pack()
/////////OTB specific//////////////
#endif
