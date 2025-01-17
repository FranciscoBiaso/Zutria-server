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

#include "creature.h"
#include "game.h"
#include "player.h"
#include "npc.h"
#include "monster.h"
#include "container.h"
#include "condition.h"
#include "combat.h"
#include "configmanager.h"
#include "ioplayer.h"

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>


extern Weapons* g_weapons;

#if defined __EXCEPTION_TRACER__
#include "exception.h"
#endif

boost::recursive_mutex AutoID::autoIDLock;
uint32_t AutoID::count = 1000;
AutoID::list_type AutoID::list;

extern Game g_game;
extern ConfigManager g_config;
extern CreatureEvents* g_creatureEvents;

Creature::Creature() :
	isInternalRemoved(false), m_attackTicks(0),
	m_attackSpeed(EVENT_CREATURE_INTERVAL * 2),
	m_attacking(false)
{
	id = 0;
    _tile = NULL;
	direction  = NORTH;
	master = NULL;
	lootDrop = true;
	skillLoss = true;

	health     = 1000;
	healthMax  = 1000;
	mana = 0;
	manaMax = 0;

	lastStep = 0;
	lastStepCost = 1;
	extraStepDuration = 0;
	baseSpeed = 220;
	varSpeed = 0;

	masterRadius = -1;
	masterPos.x = 0;
	masterPos.y = 0;
	masterPos.z = 0;

	followCreature = NULL;
	hasFollowPath = false;
	eventWalk = 0;
	forceUpdateFollowPath = false;
	isMapLoaded = false;
	isUpdatingPath = false;
	memset(localMapCache, false, sizeof(localMapCache));

	attackedCreature = NULL;
	lastHitCreature = 0;

	blockCount = 0;
	blockTicks = 0;
	walkUpdateTicks = 0;
	checkCreatureVectorIndex = 0;
	scriptEventsBitField = 0;
	onIdleStatus();
	isDying = false;
	range = 1;
	//defenderCreature = nullptr;

	for (int32_t i = 0; i < SLOT_LAST; i++) {
		inventory[i] = NULL;
		inventoryAbilities[i] = false;
	}

	for (int i = 0; i < NUMBER_OF_SKILLS; i++)
		m_skills[i] = 0;
}

Creature::~Creature()
{
	std::list<Creature*>::iterator cit;
	for(cit = summons.begin(); cit != summons.end(); ++cit) {
		(*cit)->setAttackedCreature(NULL);
		(*cit)->setMaster(NULL);
		(*cit)->releaseThing2();
	}

	summons.clear();

	for(ConditionList::iterator it = conditions.begin(); it != conditions.end(); ++it){
		(*it)->endCondition(this, CONDITIONEND_CLEANUP);
		delete *it;
	}

	conditions.clear();

	attackedCreature = NULL;

	//std::cout << "Creature destructor " << this->getID() << std::endl;
}

void Creature::onAttacking(uint32_t interval)
{
	if (attackedCreature)
	{
		//onAttacked();
		attackedCreature->onAttacked();

		//target not in range or blocked by objects
		if (isInRange(attackedCreature) && g_game.isSightClear(getPosition(), attackedCreature->getPosition(), true))
		{
			m_attackTicks += interval;

			//delay time to attack
			if (m_attackTicks >= getAttackSpeed())
			{
				m_attacking = true;
				doAttacking();
				resetAttackTicks();
				m_attacking = false;
			}
		}
	}
}

uint16_t Creature::getAttackSpeed() const 
{
	return m_attackSpeed;
}


void Creature::resetAttackTicks()
{
	m_attackTicks = 0;
}

Item * Creature::getInventoryItem(slots_t slot) const
{
	return inventory[slot];
}


Item* Creature::getWeapon() const
{
	if (getInventoryItem(SLOT_RIGHT))
		return getInventoryItem(SLOT_RIGHT)->isWeapon() ? getInventoryItem(SLOT_RIGHT) : getInventoryItem(SLOT_LEFT);
	else
		return  getInventoryItem(SLOT_LEFT);
}

Item* Creature::getShield() const
{
	if (getInventoryItem(SLOT_RIGHT))
		return getInventoryItem(SLOT_RIGHT)->isEquipament() ? getInventoryItem(SLOT_RIGHT) : getInventoryItem(SLOT_LEFT);
	else
		return  getInventoryItem(SLOT_LEFT);
}

bool Creature::whereDmgTook(Creature *attacker, int32_t& blockType, double & defenseBodyFactor, int32_t & defenseItemFactor)
{
	bool critic = false;
	double probabilityOccurr = 0;	

	Item * weapon = getWeapon();
	Item * shield = getShield();
	Item * elm = getInventoryItem(SLOT_HEAD);
	Item * legs = getInventoryItem(SLOT_LEGS);
	Item * armor = getInventoryItem(SLOT_ARMOR);

	double attackerAgilityRandomNumber = dice_00_10() * attacker->getSkillValue(ATTR_AGILITY);
	double defenderAgilityRandomNumber = dice_00_10() * getSkillValue(ATTR_AGILITY);

	if (defenderAgilityRandomNumber > attackerAgilityRandomNumber)
	{
		//get the influence of the amount of agility of this player
		double defenderAgilityInfluence = 1 - std::pow(0.5, defenderAgilityRandomNumber / 20.0);

		//(defenderAgilityRandomNumber - attackerAgilityRandomNumber)/defendingPlayerAgility = %of agility in use in this moment                 
		probabilityOccurr = (defenderAgilityRandomNumber - attackerAgilityRandomNumber) / defenderAgilityRandomNumber * defenderAgilityInfluence * 0.35;


		if (dice_00_10() <= probabilityOccurr)
		{
			blockType = BLOCK_DODGE;
			defenseBodyFactor = inifity;
			defenseItemFactor = inifity;
		}

	}

	double attackerSkillRandomNumber = dice_00_10() * attacker->getSkillValue(attacker->getAttackSkillType(attacker->getWeaponType()));
	double defenderSkillRandomNumber = dice_00_10() * getSkillValue(ATTR_DEFENSE);


	if (defenderSkillRandomNumber > attackerSkillRandomNumber)
	{
		//get the influence of the amount of defense of this player
		double defenderDefenseInfluence = 1 - std::pow(0.5, defenderSkillRandomNumber / 20.0);
		//(defenderAgilityRandomNumber - attackerAgilityRandomNumber)/defendingPlayerAgility = %of agility in use in this moment                 
		probabilityOccurr = (defenderSkillRandomNumber - attackerSkillRandomNumber) / defenderSkillRandomNumber * defenderDefenseInfluence;

		//trying to block with shield		
		if (dice_00_10() <= probabilityOccurr && shield)
		{
			blockType = BLOCK_SHIELD;
			defenseItemFactor = shield->getDefense();
			defenseBodyFactor = dice_05_10();

		}

		//trying to block with weapon
		if (dice_00_10() <= probabilityOccurr && weapon)
		{
			blockType = BLOCK_WEAPON;
			defenseItemFactor = weapon->getDefense();
			defenseBodyFactor = dice_04_09();

		}
	}

	//didn't took at shield/weapon/air(dodge)
	if (blockType == BLOCK_NONE)
	{
		double randomNumber = dice_00_10();
		//55% chance armor
		if (randomNumber <= 0.55)
		{
			blockType = BLOCK_ARMOR;
			if (armor)
				defenseItemFactor = armor->getDefense();
			defenseBodyFactor = dice_04_085();

		}
		//30% legs
		else if (randomNumber <= 0.85)
		{
			blockType = BLOCK_LEGS;
			if (legs)
				defenseItemFactor = legs->getDefense();
			defenseBodyFactor = dice_04_075();

		}
		//15% helmet
		else
		{
			blockType = BLOCK_ELM;
			if (elm)
				defenseItemFactor = elm->getDefense();
			defenseBodyFactor = dice_00_10();

			// aprox 3% chance to critic occur
			//increased with attacker concentration
			if (defenseBodyFactor <= 1.0 - std::pow(0.5, attacker->getSkillValue(ATTR_CONCENTRATION) / 25.0))
				critic = true;
		}
	}

	//we already has defenseitemfactor and defensebodyfactor
	//let's reset blocktype it will be attributed in future
	if (!(blockType & BLOCK_DODGE))
		blockType = BLOCK_NONE;

	if (critic)
		return true;
	else
		return false;
}


bool Creature::canSee(const Position& myPos, const Position& pos, uint32_t viewRangeX, uint32_t viewRangeY)
{
	if(myPos.z <= 7){
		//we are on ground level or above (7 -> 0)
		//view is from 7 -> 0
		if(pos.z > 7){
			return false;
		}
	}
	else if(myPos.z >= 8){
		//we are underground (8 -> 15)
		//view is +/- 2 from the floor we stand on
		if(std::abs(myPos.z - pos.z) > 2){
			return false;
		}
	}

	int offsetz = myPos.z - pos.z;

	if ((pos.x >= myPos.x - viewRangeX + offsetz) && (pos.x <= myPos.x + viewRangeX + offsetz) &&
		(pos.y >= myPos.y - viewRangeY + offsetz) && (pos.y <= myPos.y + viewRangeY + offsetz))
		return true;

	return false;
}

bool Creature::canSee(const Position& pos) const
{
	return canSee(getPosition(), pos, Map::maxViewportX, Map::maxViewportY);
}

bool Creature::canSeeCreature(const Creature* creature) const
{
	if(!canSeeInvisibility() && creature->isInvisible()){
		return false;
	}

	return true;
}

int64_t Creature::getTimeSinceLastMove() const
{
	if(lastStep){
		return OTSYS_TIME() - lastStep;
	}

	return 0x7FFFFFFFFFFFFFFFLL;
}

int32_t Creature::getWalkDelay(Direction dir) const
{
	if(lastStep != 0){
		int64_t ct = OTSYS_TIME();
		int64_t stepDuration = getStepDuration(dir);
		return stepDuration - (ct - lastStep);
	}

	return 0;
}

int32_t Creature::getWalkDelay() const
{
	//Used for auto-walking
	if(lastStep != 0){
		int64_t ct = OTSYS_TIME();
		int64_t stepDuration = getStepDuration();
		return stepDuration - (ct - lastStep);
	}

	return 0;
}

void Creature::onThink(uint32_t interval)
{
	if(!isMapLoaded && useCacheMap()){
		isMapLoaded = true;
		updateMapCache();
	}
	
	if(followCreature && getMaster() != followCreature && !canSeeCreature(followCreature)){
		onCreatureDisappear(followCreature, false);
	}

	if(attackedCreature && getMaster() != attackedCreature && !canSeeCreature(attackedCreature)){
		onCreatureDisappear(attackedCreature, false);
	}

	blockTicks += interval;

	if(blockTicks >= 1000){
		//blockCount = std::min((uint32_t)blockCount + 1, (uint32_t)2);
		//reset
		blockCount = 0;
		blockTicks = 0;
	}

	if(followCreature){
		walkUpdateTicks += interval;
		if(forceUpdateFollowPath || walkUpdateTicks >= 2000){
			walkUpdateTicks = 0;
			forceUpdateFollowPath = false;
			isUpdatingPath = true;
		}
	}

	if(isUpdatingPath){
		isUpdatingPath = false;
		getPathToFollowCreature();
	}
}


void Creature::onWalk()
{
	if(getWalkDelay() <= 0){
		Direction dir;
		if(getNextStep(dir)){
			if(g_game.internalMoveCreature(this, dir, FLAG_IGNOREFIELDDAMAGE) != RET_NOERROR){
				forceUpdateFollowPath = true;
			}
		}
	}

	if(listWalkDir.empty()){
		onWalkComplete();
	}

	if(eventWalk != 0){
		eventWalk = 0;
		addEventWalk();
	}
}

void Creature::onWalk(Direction& dir)
{
	//if(hasCondition(CONDITION_DRUNK)){
	//	uint32_t r = random_range(0, 16);

	//	if(r <= 4){
	//		switch(r){
	//			case 0: dir = NORTH; break;
	//			case 1: dir = WEST;  break;
	//			case 3: dir = SOUTH; break;
	//			case 4: dir = EAST;  break;

	//			default:
	//				break;
	//		}

	//		//g_game.internalCreatureSay(this, SPEAK_SAY, "Hicks!");
	//	}
	//}
}

bool Creature::getNextStep(Direction& dir)
{
	if(!listWalkDir.empty()){
		dir = listWalkDir.front();
		listWalkDir.pop_front();
		onWalk(dir);

		return true;
	}

	return false;
}


uint8_t Creature::getAttackSkillType(WeaponType_t weaponType) const
{
	switch (weaponType)
	{
		case WEAPON_TYPE_NONE:
		case WEAPON_TYPE_MELEE:
			return ATTR_MELEE;
			break;

		case WEAPON_TYPE_DISTANCE:
			return ATTR_DISTANCE;
			break;

		default:
			return ATTR_MELEE;
	}
}

bool Creature::startAutoWalk(std::list<Direction>& listDir)
{
	listWalkDir = listDir;
	addEventWalk();
	return true;
}

void Creature::addEventWalk()
{
	if(eventWalk == 0){
		//std::cout << "addEventWalk() - " << getName() << std::endl;

		int64_t ticks = getEventStepTicks();
		if(ticks > 0){
			eventWalk = Scheduler::getScheduler().addEvent(createSchedulerTask(
				ticks, boost::bind(&Game::checkCreatureWalk, &g_game, getID())));
		}
	}
}

void Creature::stopEventWalk()
{
	if(eventWalk != 0){
		Scheduler::getScheduler().stopEvent(eventWalk);
		eventWalk = 0;

		if(!listWalkDir.empty()){
			listWalkDir.clear();
			onWalkAborted();
		}
	}
}

void Creature::updateMapCache()
{
	Tile* tile;
	const Position& myPos = getPosition();
	Position pos(0, 0, myPos.z);

	for(int32_t y = -((mapWalkHeight - 1) / 2); y <= ((mapWalkHeight - 1) / 2); ++y)
	{
		for(int32_t x = -((mapWalkWidth - 1) / 2); x <= ((mapWalkWidth - 1) / 2); ++x)
		{
			pos.x = myPos.x + x;
			pos.y = myPos.y + y;
			tile = g_game.getTile(pos.x, pos.y, myPos.z);
			updateTileCache(tile, pos);
		}
	}
}

#ifdef __DEBUG__
void Creature::validateMapCache()
{
	const Position& myPos = getPosition();
	for(int32_t y = -((mapWalkHeight - 1) / 2); y <= ((mapWalkHeight - 1) / 2); ++y){
		for(int32_t x = -((mapWalkWidth - 1) / 2); x <= ((mapWalkWidth - 1) / 2); ++x){
			bool result = (getWalkCache(Position(myPos.x + x, myPos.y + y, myPos.z)) == 1);
		}
	}
}
#endif

void Creature::updateTileCache(const Tile* tile, int32_t dx, int32_t dy)
{
	if((std::abs(dx) <= (mapWalkWidth - 1) / 2) && (std::abs(dy) <= (mapWalkHeight - 1) / 2)){

		int32_t x = (mapWalkWidth - 1) / 2 + dx;
		int32_t y = (mapWalkHeight - 1) / 2 + dy;

		localMapCache[y][x] = (tile && tile->__queryAdd(0, this, 1,
			FLAG_PATHFINDING | FLAG_IGNOREFIELDDAMAGE) == RET_NOERROR);
	}
#ifdef __DEBUG__
	else{
		std::cout << "Creature::updateTileCache out of range." << std::endl;
	}
#endif
}

void Creature::updateTileCache(const Tile* tile, const Position& pos)
{
	const Position& myPos = getPosition();
	if(pos.z == myPos.z){
		int32_t dx = pos.x - myPos.x;
		int32_t dy = pos.y - myPos.y;

		updateTileCache(tile, dx, dy);
	}
}

int32_t Creature::getWalkCache(const Position& pos) const
{
	if(!useCacheMap()){
		return 2;
	}

	const Position& myPos = getPosition();
	if(myPos.z != pos.z){
		return 0;
	}

	if(pos == myPos){
		return 1;
	}

	int32_t dx = pos.x - myPos.x;
	int32_t dy = pos.y - myPos.y;

	if((std::abs(dx) <= (mapWalkWidth - 1) / 2) && (std::abs(dy) <= (mapWalkHeight - 1) / 2))
	{
		int32_t x = (mapWalkWidth - 1) / 2 + dx;
		int32_t y = (mapWalkHeight - 1) / 2 + dy;

#ifdef __DEBUG__
		//testing
		Tile* tile = g_game.getTile(pos.x, pos.y, pos.z);

		if(tile && (tile->__queryAdd(0, this, 1, FLAG_PATHFINDING | FLAG_IGNOREFIELDDAMAGE) == RET_NOERROR)){
			if(!localMapCache[y][x]){
				std::cout << "Wrong cache value" << std::endl;
			}
		}
		else{
			if(localMapCache[y][x]){
				std::cout << "Wrong cache value" << std::endl;
			}
		}
#endif

		if(localMapCache[y][x])
			return 1;
		else
			return 0;
	}

	//out of range
	return 2;
}

void Creature::onAddTileItem(const Tile* tile, const Position& pos, const Item* item)
{
	if(isMapLoaded)
	{
		if(pos.z == getPosition().z)
			updateTileCache(tile, pos);		
	}
}

void Creature::onUpdateTileItem(const Tile* tile, const Position& pos, uint32_t stackpos,
	const Item* oldItem, const ItemType& oldType, const Item* newItem, const ItemType& newType)
{
	if(isMapLoaded){
		if(oldType.blockSolid || oldType.blockPathFind || newType.blockPathFind || newType.blockSolid){
			if(pos.z == getPosition().z){
				updateTileCache(tile, pos);
			}
		}
	}
}

void Creature::onRemoveTileItem(const Tile* tile, const Position& pos, uint32_t stackpos,
	const ItemType& iType, const Item* item)
{
	if(isMapLoaded){
		if(iType.blockSolid || iType.blockPathFind || iType.isGroundTile()){
			if(pos.z == getPosition().z){
				updateTileCache(tile, pos);
			}
		}
	}
}

void Creature::onUpdateTile(const Tile* tile, const Position& pos)
{
	//
}

void Creature::onCreatureAppear(const Creature* creature, bool isLogin)
{
	if(creature == this){
		if(useCacheMap()){
			isMapLoaded = true;
			updateMapCache();
		}
	}
	else if(isMapLoaded){
		if(creature->getPosition().z == getPosition().z){
			updateTileCache(creature->getTile(), creature->getPosition());
		}
	}
}

void Creature::onCreatureDisappear(const Creature* creature, uint32_t stackpos, bool isLogout)
{
	onCreatureDisappear(creature, true);

	if(creature == this){
		if(getMaster() && !getMaster()->isRemoved()){
			getMaster()->removeSummon(this);
		}
	}
	else if(isMapLoaded){
		if(creature->getPosition().z == getPosition().z){
			updateTileCache(creature->getTile(), creature->getPosition());
		}
	}
}

void Creature::onCreatureDisappear(const Creature* creature, bool isLogout)
{
	if(attackedCreature == creature){
		setAttackedCreature(NULL);
		onAttackedCreatureDissapear(isLogout);
	}

	if(followCreature == creature){
		setFollowCreature(NULL);
		onFollowCreatureDissapear(isLogout);
	}
}

void Creature::onChangeZone(ZoneType_t zone)
{
	if(attackedCreature){
		if(zone == ZONE_PROTECTION){
			onCreatureDisappear(attackedCreature, false);
		}
	}
}

void Creature::onAttackedCreatureChangeZone(ZoneType_t zone)
{
	if(zone == ZONE_PROTECTION){
		onCreatureDisappear(attackedCreature, false);
	}
}

void Creature::onCreatureMove(const Creature* creature, const Tile* newTile, const Position& newPos,
	const Tile* oldTile, const Position& oldPos, uint32_t oldStackPos, bool teleport)
{
	if(creature == this)
	{
		lastStep = OTSYS_TIME();
		extraStepDuration = 0;
		lastStepCost = 1;

		if(teleport)
			stopEventWalk();
		else
		{
			//floor change extra cost
			if(oldPos.z != newPos.z)
				lastStepCost = 2;			
			//diagonal extra cost
			else if(std::abs(newPos.x - oldPos.x) >=1 && std::abs(newPos.y - oldPos.y) >= 1)				
				lastStepCost = 2;			
		}

		if(!summons.empty())
		{
			//check if any of our summons is out of range (+/- 2 floors or 30 tiles away)

			std::list<Creature*> despawnList;
			std::list<Creature*>::iterator cit;
			for(cit = summons.begin(); cit != summons.end(); ++cit){
				const Position pos = (*cit)->getPosition();

				if( (std::abs(pos.z - newPos.z) > 2) ||
					(std::max(std::abs((newPos.x) - pos.x), std::abs((newPos.y - 1) - pos.y)) > 30) ){
					despawnList.push_back((*cit));
				}
			}

			for(cit = despawnList.begin(); cit != despawnList.end(); ++cit){
				g_game.removeCreature((*cit), true);
			}
		}

		onChangeZone(getZone());

		//update map cache
		if(isMapLoaded)
		{
			if(teleport || oldPos.z != newPos.z)
				updateMapCache();			
			else
			{
				Tile* tile;
				const Position& myPos = getPosition();
				Position pos;

				if(oldPos.y > newPos.y)
				{ // north
					//shift y south
					for(int32_t y = mapWalkHeight - 1 - 1; y >= 0; --y){
						memcpy(localMapCache[y + 1], localMapCache[y], sizeof(localMapCache[y]));
					}

					//update 0
					for(int32_t x = -((mapWalkWidth - 1) / 2); x <= ((mapWalkWidth - 1) / 2); ++x){
						tile = g_game.getTile(myPos.x + x, myPos.y - ((mapWalkHeight - 1) / 2), myPos.z);
						updateTileCache(tile, x, -((mapWalkHeight - 1) / 2));
					}
				}
				else if(oldPos.y < newPos.y){ // south
					//shift y north
					for(int32_t y = 0; y <= mapWalkHeight - 1 - 1; ++y){
						memcpy(localMapCache[y], localMapCache[y + 1], sizeof(localMapCache[y]));
					}

					//update mapWalkHeight - 1
					for(int32_t x = -((mapWalkWidth - 1) / 2); x <= ((mapWalkWidth - 1) / 2); ++x){
						tile = g_game.getTile(myPos.x + x, myPos.y + ((mapWalkHeight - 1) / 2), myPos.z);
						updateTileCache(tile, x, (mapWalkHeight - 1) / 2);
					}
				}

				if(oldPos.x < newPos.x){ // east
					//shift y west
					int32_t starty = 0;
					int32_t endy = mapWalkHeight - 1;
					int32_t dy = (oldPos.y - newPos.y);
					if(dy < 0){
						endy = endy + dy;
					}
					else if(dy > 0){
						starty = starty + dy;
					}

					for(int32_t y = starty; y <= endy; ++y){
						for(int32_t x = 0; x <= mapWalkWidth - 1 - 1; ++x){
							localMapCache[y][x] = localMapCache[y][x + 1];
						}
					}

					//update mapWalkWidth - 1
					for(int32_t y = -((mapWalkHeight - 1) / 2); y <= ((mapWalkHeight - 1) / 2); ++y){
						tile = g_game.getTile(myPos.x + ((mapWalkWidth - 1) / 2), myPos.y + y, myPos.z);
						updateTileCache(tile, (mapWalkWidth - 1) / 2, y);
					}
				}
				else if(oldPos.x > newPos.x){ // west
					//shift y east
					int32_t starty = 0;
					int32_t endy = mapWalkHeight - 1;
					int32_t dy = (oldPos.y - newPos.y);
					if(dy < 0){
						endy = endy + dy;
					}
					else if(dy > 0){
						starty = starty + dy;
					}

					for(int32_t y = starty; y <= endy; ++y){
						for(int32_t x = mapWalkWidth - 1 - 1; x >= 0; --x){
							localMapCache[y][x + 1] = localMapCache[y][x];
						}
					}

					//update 0
					for(int32_t y = -((mapWalkHeight - 1) / 2); y <= ((mapWalkHeight - 1) / 2); ++y){
						tile = g_game.getTile(myPos.x - ((mapWalkWidth - 1) / 2), myPos.y + y, myPos.z);
						updateTileCache(tile, -((mapWalkWidth - 1) / 2), y);
					}
				}

				updateTileCache(oldTile, oldPos);
	#ifdef __DEBUG__
				validateMapCache();
	#endif
			}
		}
	}
	else{
		if(isMapLoaded){
			const Position& myPos = getPosition();
			if(newPos.z == myPos.z){
				updateTileCache(newTile, newPos);
			}

			if(oldPos.z == myPos.z){
				updateTileCache(oldTile, oldPos);
			}
		}
	}

	if(creature == followCreature || (creature == this && followCreature)){
		if(hasFollowPath){
			isUpdatingPath = false;
			Dispatcher::getDispatcher().addTask(createTask(
				boost::bind(&Game::updateCreatureWalk, &g_game, getID())));
		}

		if(newPos.z != oldPos.z || !canSee(followCreature->getPosition())){
			onCreatureDisappear(followCreature, false);
		}
	}

	if(creature == attackedCreature || (creature == this && attackedCreature)){
		if(newPos.z != oldPos.z || !canSee(attackedCreature->getPosition())){
			onCreatureDisappear(attackedCreature, false);
		}
		else{
			//if(hasExtraSwing()){
			//	//our target is moving lets see if we can get in hit
			//	Dispatcher::getDispatcher().addTask(createTask(
			//		boost::bind(&Game::checkCreatureAttack, &g_game, getID())));
			//}

			onAttackedCreatureChangeZone(attackedCreature->getZone());
		}
	}
}

void Creature::onCreatureChangeVisible(const Creature* creature, bool visible)
{
	//
}

void Creature::onDie()
{
	Creature* lastHitCreature = NULL;
	Creature* mostDamageCreature = NULL;
	Creature* mostDamageCreatureMaster = NULL;
	Creature* lastHitCreatureMaster = NULL;

	if(getKillers(&lastHitCreature, &mostDamageCreature)){
		if(lastHitCreature){
			lastHitCreature->onKilledCreature(this, true);
			lastHitCreatureMaster = lastHitCreature->getMaster();
		}

		if(mostDamageCreature){
			mostDamageCreatureMaster = mostDamageCreature->getMaster();
			bool isNotLastHitMaster = (mostDamageCreature != lastHitCreatureMaster);
			bool isNotMostDamageMaster = (lastHitCreature != mostDamageCreatureMaster);
			bool isNotSameMaster = lastHitCreatureMaster == NULL || (mostDamageCreatureMaster != lastHitCreatureMaster);

			if(mostDamageCreature != lastHitCreature && isNotLastHitMaster &&
				isNotMostDamageMaster && isNotSameMaster){
				mostDamageCreature->onKilledCreature(this, false);
			}
		}
	}

	for(CountMap::iterator it = damageMap.begin(); it != damageMap.end(); ++it){
		if(Creature* attacker = g_game.getCreatureByID((*it).first)){
			attacker->onAttackedCreatureKilled(this);
		}
	}

	if (g_config.getBoolean(ConfigManager::STORE_DEATHS)) {
		Player* player = getPlayer();
		if (player && lastHitCreature && mostDamageCreature) {
			IOPlayer::instance()->saveDeath(player->getName(), std::time(NULL),
				player->getLevel(), lastHitCreature->getName(), mostDamageCreature->getName());
		}
	}

	die();
	dropCorpse();

	if(getMaster()){
		getMaster()->removeSummon(this);
	}
}
	
void Creature::dropCorpse()
{
	Item* splash = NULL;


	switch(getRace())
	{
		case RACE_VENOM:
			splash = Item::CreateItem(ITEM_SPLASH_SIZE_1, FLUID_GREEN);
			break;

		case RACE_BLOOD:
			splash = Item::CreateItem(ITEM_SPLASH_SIZE_1, FLUID_BLOOD);
			break;

		case RACE_UNDEAD:
			break;

		case RACE_FIRE:
			break;

		default:
			break;
	}

	Tile* tile = getTile();
	if(splash){
		g_game.internalAddItem(tile, splash, INDEX_WHEREEVER, FLAG_NOLIMIT);
		g_game.startDecay(splash);
	}

	Item* corpse = getCorpse();
	if(corpse){
		g_game.internalAddItem(tile, corpse, INDEX_WHEREEVER, FLAG_NOLIMIT);
		g_game.startDecay(corpse);
	}

	//scripting event - onDie
	onDieEvent(corpse);
	
	//if(corpse)
		//dropLoot(corpse->getContainer());

	g_game.removeCreature(this, false);
}

bool Creature::getKillers(Creature** _lastHitCreature, Creature** _mostDamageCreature)
{
	*_lastHitCreature = g_game.getCreatureByID(lastHitCreature);

	int32_t mostDamage = 0;
	CountBlock_t cb;
	for(CountMap::iterator it = damageMap.begin(); it != damageMap.end(); ++it){
		cb = it->second;

		if((cb.total > mostDamage && (OTSYS_TIME() - cb.ticks <= g_game.getInFightTicks()))){
			if((*_mostDamageCreature = g_game.getCreatureByID((*it).first))){
				mostDamage = cb.total;
			}
		}
	}

	return (*_lastHitCreature || *_mostDamageCreature);
}

bool Creature::hasBeenAttacked(uint32_t attackerId)
{
	CountMap::iterator it = damageMap.find(attackerId);
	if(it != damageMap.end()){
		return (OTSYS_TIME() - it->second.ticks <= g_game.getInFightTicks());
	}

	return false;
}

Item* Creature::getCorpse()
{
	Item* corpse = Item::CreateItem(getLookCorpse());
	return corpse;
}

void Creature::changeHealth(int32_t healthChange)
{
	if(healthChange > 0){
		health += std::min(healthChange, getMaxHealth() - health);
	}
	else{
		health = std::max((int32_t)0, health + healthChange);
	}

	g_game.addCreatureHealth(this);
}

void Creature::changeMana(int32_t manaChange)
{
	if(manaChange > 0){
		mana += std::min(manaChange, getMaxMana() - mana);
	}
	else{
		mana = std::max((int32_t)0, mana + manaChange);
	}
}

void Creature::gainHealth(Creature* caster, int32_t healthGain)
{
	if(healthGain > 0){
		int32_t prevHealth = getHealth();
		changeHealth(healthGain);

		int32_t effectiveGain = getHealth() - prevHealth;
		if(caster){
			caster->onTargetCreatureGainHealth(this, effectiveGain);
		}
	}
	else{
		changeHealth(healthGain);
	}
}

void Creature::drainHealth(Creature* attacker, CombatType_t combatType, int32_t damage)
{
	changeHealth(-damage);

	if(attacker){
		attacker->onAttackedCreatureDrainHealth(this, damage);
	}
}

void Creature::drainMana(Creature* attacker, int32_t manaLoss)
{
	onAttacked();
	changeMana(-manaLoss);
}

int Creature::blockHit(Creature* attacker, CombatType_t combatType, int * blockType, void * data)
{	
	//if(isImmune(combatType))
	//{
	//	//the defender is immune, so blocked
	//	return true;
	//}

	////count of hits in a interval time
	//blockCount++;

	//struct _weaponDamage_ * wd = (struct _weaponDamage_ *)data;
	////reset
	//wd->damageType = 0;
	//wd->critic = false;

	//Item * weapon = getWeapon();
	//Item * elm = getInventoryItem(SLOT_HEAD);
	//Item * legs = getInventoryItem(SLOT_LEGS);
	//Item * armor = getInventoryItem(SLOT_ARMOR);
	//double itemDefenseFactor = 0;
	//double bodyDefenseFactor = 0;

	//double probabilityOccurr = 0.05;
	//double attackingCreaturesPercentage = 0;
	//double randomNumber = random_range(0, 100) / 100.0;

	//double attackingCreatureAttackSkill = attacker->getSkillValue(attacker->getAttackSkillType(attacker->getWeaponType()));
	//double defendingPlayerAgility = getSkillValue(ATTR_AGILITY);
	//double defendingPlayerDefenseSkill = getSkillValue(ATTR_DEFENSE);
	//double damageBlockled = (0.5 * getSkillValue(ATTR_FORCE) + 0.5 * defendingPlayerDefenseSkill);

	////a player can not block more than your max number block
	//if (blockCount < _player_::max_creatures_to_block)
	//{
	//	double reducePercentageByAmountOfCreatures = 1.0 - (blockCount - 1.0) / _player_::max_creatures_to_block;

	//	//------------- 1�) agility phase -------------//
	//	//has attacking player a low agility then defending player?
	//	//so it's possible to try dodge
	//	double attackerAgilityRandomNumber = random_range(1, attacker->getSkillValue(ATTR_AGILITY) + 1);
	//	double defenderAgilityRandomNumber = random_range(1, defendingPlayerAgility + 1);
	//	if (attackerAgilityRandomNumber - defenderAgilityRandomNumber <= 0)
	//	{
	//		//get the influence of the amount of agility of this player
	//		double defenderAgilityInfluence = 1 - std::pow(0.5, defendingPlayerAgility / 20.0);
	//		//(defenderAgilityRandomNumber - attackerAgilityRandomNumber)/defendingPlayerAgility = %of agility in use in this moment                 
	//		probabilityOccurr = (defenderAgilityRandomNumber - attackerAgilityRandomNumber) / defenderAgilityRandomNumber * defenderAgilityInfluence;


	//		//+ creatures attacking - chance to dodge
	//		probabilityOccurr *= reducePercentageByAmountOfCreatures;
	//		randomNumber = random_range(0, 100) / 100.0;
	//		if (randomNumber <= probabilityOccurr)
	//			*blockType = BLOCK_DODGE;
	//	}
	//	//------------- agility phase -------------//

	//	//------------- 2�) skill defending phase -------------//	
	//	//has attacking player a low skill attack then defedeing player skill defense?

	//	double attackerSkillRandomNumber = random_range(1, attackingCreatureAttackSkill + 1);
	//	double defenderSkillRandomNumber = random_range(1, defendingPlayerDefenseSkill + 1);
	//	if (attackerSkillRandomNumber - defenderSkillRandomNumber <= 0)
	//	{
	//		//get the influence of the amount of defense of this player
	//		double defenderDefenseInfluence = 1 - std::pow(0.5, defendingPlayerDefenseSkill / 20.0);
	//		//(defenderAgilityRandomNumber - attackerAgilityRandomNumber)/defendingPlayerAgility = %of agility in use in this moment                 
	//		probabilityOccurr = (defenderSkillRandomNumber - attackerSkillRandomNumber) / defenderSkillRandomNumber * defenderDefenseInfluence;

	//		probabilityOccurr *= reducePercentageByAmountOfCreatures;
	//		//trying to block with shield
	//		randomNumber = random_range(0, 100) / 100.0;
	//		if (randomNumber <= probabilityOccurr && hasShield())
	//			*blockType = BLOCK_SHIELD;

	//		//trying to block with weapon
	//		randomNumber = random_range(0, 100) / 100.0;
	//		if (randomNumber <= probabilityOccurr && weapon)
	//			*blockType = BLOCK_WEAPON;
	//	}
	//}

	////where the damage took? It was not defended by the weapon or shield or not dodge
	//if (*blockType == BLOCK_NONE)
	//{
	//	randomNumber = random_range(0, 100) / 100.0;
	//	//55% chance armor
	//	if (randomNumber >= 0.0 && randomNumber <= 0.55)
	//		*blockType = BLOCK_ARMOR;
	//	//30% legs
	//	else if (randomNumber > 0.55 && randomNumber <= 0.85)
	//		*blockType = BLOCK_LEGS;
	//	//15% helmet
	//	else
	//		*blockType = BLOCK_ELM;
	//}
	////------------- 2�) skill defending phase -------------//	

	////let's see if we coul'd defense and subtract the dmg
	//switch (*blockType)
	//{
	//	//dodge
	//case BLOCK_DODGE:
	//{
	//	std::cout << "esquiva" << std::endl;
	//}break;

	//case BLOCK_WEAPON:
	//{
	//	itemDefenseFactor = weapon->getDefense();
	//	bodyDefenseFactor = 0.045;
	//	std::cout << "arma" << std::endl;

	//}break;

	//case BLOCK_SHIELD:
	//{
	//	itemDefenseFactor = getShieldDefense();
	//	bodyDefenseFactor = 0.07;
	//	std::cout << "escudo" << std::endl;
	//}break;

	//case BLOCK_ARMOR:
	//{
	//	std::cout << "armadura" << std::endl;
	//	bodyDefenseFactor = 0.040;
	//	if (armor)
	//	{
	//		itemDefenseFactor = armor->getDefense();
	//	}
	//}break;

	//case BLOCK_LEGS:
	//{
	//	std::cout << "cal�a" << std::endl;
	//	bodyDefenseFactor = 0.035;
	//	if (legs)
	//	{
	//		itemDefenseFactor = legs->getDefense();
	//	}
	//}break;

	//case BLOCK_ELM:
	//{
	//	wd->critic = true;
	//	std::cout << "elmo" << std::endl;
	//	bodyDefenseFactor = 0.030;
	//	if (elm)
	//	{
	//		itemDefenseFactor = elm->getDefense();
	//	}
	//}break;
	//}

	////***perfuration***//

	//bool perfurationOcurr = false;
	//if (!(*blockType & BLOCK_DODGE) && wd->perforationFactor > 0)//block type is not a dodge, so reset	
	//{
	//	//reset
	//	*blockType = BLOCK_NONE;

	//	//lets subract defense if perforation was occured	
	//	probabilityOccurr = 1 - std::pow(0.5, wd->perforationFactor / 55.0);
	//	randomNumber = random_range(0, 100) / 100.0;

	//	//perforation ocurred
	//	if (randomNumber <= probabilityOccurr)
	//	{
	//		perfurationOcurr = true;
	//		//perforation exist's but defense player has blocking with def > 0
	//		double ignoreDefenseFactor = 1;
	//		double weaponPerforationPercentage =
	//			random_range(0.3 * (1 - std::pow(0.5, wd->perforationFactor / 20.0)) * 100.0, (1 - std::pow(0.5, wd->perforationFactor / 20.0)) * 100.0) / 100.0;
	//		if (itemDefenseFactor != 0)
	//		{
	//			double tmpItemDefenseFactor = itemDefenseFactor;
	//			// let's break it def
	//			itemDefenseFactor = itemDefenseFactor - itemDefenseFactor * weaponPerforationPercentage;
	//			ignoreDefenseFactor = 1 - itemDefenseFactor / tmpItemDefenseFactor;
	//			//reducing armor percentPerforationEffective %	

	//			//print in text msg count perfured %
	//			wd->perforationFactor = ignoreDefenseFactor * 100.0;
	//		}
	//		//perforation exist's but defense player has blokcint with def == 0
	//		else
	//		{
	//			//the damage was incresed proportional to weapon dmg 
	//			double weaponPerforationPercentage = 1 - std::pow(0.5, wd->perforationFactor / 20.0);
	//			wd->totalDamage += wd->totalDamage * weaponPerforationPercentage;
	//			//100% of perforation, igonre defense factor keep 1

	//			//used in text msg to print how much + dmg perforation did
	//			wd->perforationFactor = random_range(0.3 * (1 - std::pow(0.5, wd->perforationFactor / 20.0)) * 100.0, (1 - std::pow(0.5, wd->perforationFactor / 20.0)) * 100.0);
	//		}

	//		if (ignoreDefenseFactor >= 0 && ignoreDefenseFactor <= 0.33)
	//		{
	//			*blockType |= BLOCK_PERFORATION_MIN;
	//			wd->damageType |= DAMAGE_PERFORATION_MIN;
	//		}
	//		else if (ignoreDefenseFactor > 0.33 && ignoreDefenseFactor <= 0.66)
	//		{
	//			*blockType |= BLOCK_PERFORATION_MEDIUM;
	//			wd->damageType |= DAMAGE_PERFORATION_MED;
	//		}
	//		else
	//		{
	//			*blockType |= BLOCK_PERFORATION_MAX;
	//			wd->damageType |= DAMAGE_PERFORATION_MAX;
	//		}
	//	}
	//}

	//if (!perfurationOcurr)
	//	wd->perforationFactor = 0;

	//damageBlockled *= bodyDefenseFactor * itemDefenseFactor;

	////if dodge
	//if (*blockType & BLOCK_DODGE)
	//	wd->totalDamage = 0;
	//else//subtract the dmg
	//	wd->totalDamage += damageBlockled;


	//if (wd->critic)
	//{
	//	wd->criticDmg = 0.45 * wd->totalDamage;
	//	wd->totalDamage = wd->totalDamage + wd->criticDmg;
	//}

	////the dmg was def
	//if (wd->totalDamage >= 0)
	//{
	//	double damageProportionality = 0;
	//	if (damageBlockled != 0)
	//		damageProportionality = 1 - wd->totalDamage / damageBlockled;
	//	//we blocked so let's 0 the dmg, we are not gain life, only blocking
	//	if (*blockType & BLOCK_DODGE)
	//		return true;
	//	else if (damageProportionality >= 0 && damageProportionality <= 0.40)
	//		*blockType |= BLOCK_DEFENSE_MAX;
	//	else if (damageProportionality > 0.40 && damageProportionality < 0.85)
	//		*blockType |= BLOCK_DEFENSE_MEDIUM;
	//	else if (damageProportionality >= 0.85)
	//		*blockType |= BLOCK_DEFENSE_MIN;
	//	return true;
	//}
	//else //block none - the dmg passed, we are losing life
	//{
	//	//0% until 100%
	//	double percentageOfLife = std::abs(wd->totalDamage) / (double)getMaxHealth();
	//	//*** slash can occur ***//
	//	bool slashOcurr = false;
	//	if (wd->slashFactor > 0)
	//	{
	//		probabilityOccurr = 1 - std::pow(0.5, wd->slashFactor / 55.0);
	//		randomNumber = random_range(0, 100) / 100.0;

	//		//slash was generated let's do bleeding
	//		if (randomNumber <= probabilityOccurr)
	//		{
	//			double slashValuePercentage = 1 + std::pow(1 - std::pow(0.5, wd->slashFactor / 20.0), 2) * 0.8;
	//			//if perforation ocurred
	//			if ((DAMAGE_PERFORATION_MAX | DAMAGE_PERFORATION_MED | DAMAGE_PERFORATION_MIN) &  wd->damageType)
	//			{
	//				probabilityOccurr = 0.5;
	//				randomNumber = random_range(0, 100) / 100.0;

	//				if (randomNumber <= probabilityOccurr)
	//				{
	//					slashOcurr = true;
	//				}
	//			}
	//			else
	//			{
	//				slashOcurr = true;
	//			}

	//			if (slashOcurr)
	//			{
	//				//reset
	//				wd->damageType = 0;
	//				if (percentageOfLife <= 0.07)
	//					wd->damageType |= DAMAGE_SLASH_MIN;
	//				else if (percentageOfLife <= 0.15)
	//					wd->damageType |= DAMAGE_SLASH_MED;
	//				else
	//					wd->damageType |= DAMAGE_SLASH_MAX;
	//				int32_t dmgBySlash = random_range(std::floor(slashValuePercentage * wd->totalDamage) * 0.3, std::floor(slashValuePercentage * wd->totalDamage));
	//				addCombatBleeding(dmgBySlash);

	//				wd->slashFactor = -dmgBySlash;
	//			}
	//		}
	//	}

	//	if (!slashOcurr)
	//		wd->slashFactor = 0;

	//	if (percentageOfLife <= 0.07)
	//		wd->damageType |= DAMAGE_MIN;
	//	else if (percentageOfLife <= 0.15)
	//		wd->damageType |= DAMAGE_MEDIUM;
	//	else
	//		wd->damageType |= DAMAGE_MAX;

	//	return false;
	//}


	//if(attacker)
	//{
	//	attacker->onAttackedCreature(this);
	//	attacker->onAttackedCreatureBlockHit(this, *blockType);
	//}

	//onAttacked();

	//return true;
	return true;
}

bool Creature::setAttackedCreature(Creature* creature)
{
	if(creature){
		const Position& creaturePos = creature->getPosition();
		if(creaturePos.z != getPosition().z || !canSee(creaturePos)){
			attackedCreature = NULL;
			return false;
		}
	}

	attackedCreature = creature;

	if(attackedCreature){
		onAttackedCreature(attackedCreature);
		attackedCreature->onAttacked();
	}

	std::list<Creature*>::iterator cit;
	for(cit = summons.begin(); cit != summons.end(); ++cit) {
		(*cit)->setAttackedCreature(creature);
	}

	return true;
}

void Creature::getPathSearchParams(const Creature* creature, FindPathParams& fpp) const
{
	fpp.fullPathSearch = !hasFollowPath;
	fpp.clearSight = true;
	fpp.maxSearchDist = 13;
	fpp.minTargetDist = 1;
	fpp.maxTargetDist = 1;
}

void Creature::getPathToFollowCreature()
{
	if(followCreature){
		FindPathParams fpp;
		getPathSearchParams(followCreature, fpp);

		if(g_game.getPathToEx(this, followCreature->getPosition(), listWalkDir, fpp)){
			hasFollowPath = true;
			startAutoWalk(listWalkDir);
		}
		else{
			hasFollowPath = false;
		}
	}

	onFollowCreatureComplete(followCreature);
}

bool Creature::setFollowCreature(Creature* creature, bool fullPathSearch /*= false*/)
{
	if(creature){
		if(followCreature == creature){
			return true;
		}

		const Position& creaturePos = creature->getPosition();
		if(creaturePos.z != getPosition().z || !canSee(creaturePos)){
			followCreature = NULL;
			return false;
		}

		if(!listWalkDir.empty()){
			listWalkDir.clear();
			onWalkAborted();
		}

		hasFollowPath = false;
		forceUpdateFollowPath = false;
		followCreature = creature;
		isUpdatingPath = true;
	}
	else{
		isUpdatingPath = false;
		followCreature = NULL;
	}

	g_game.updateCreatureWalk(getID());
	onFollowCreature(creature);
	return true;
}

double Creature::getDamageRatio(Creature* attacker) const
{
	int32_t totalDamage = 0;
	int32_t attackerDamage = 0;

	CountBlock_t cb;
	for(CountMap::const_iterator it = damageMap.begin(); it != damageMap.end(); ++it){
		cb = it->second;

		totalDamage += cb.total;

		if(it->first == attacker->getID()){
			attackerDamage += cb.total;
		}
	}

	return ((double)attackerDamage / totalDamage);
}

uint64_t Creature::getGainedExperience(Creature* attacker, bool useMultiplier /*= true*/) const
{
	uint64_t retValue = (int64_t)std::floor(getDamageRatio(attacker) * getLostExperience());

	Player* player = attacker->getPlayer();
	if (!player) {
		if (attacker->getMaster())
			player = attacker->getMaster()->getPlayer();
	}

	if (player) {
		if(useMultiplier){
			retValue = (uint64_t)std::floor(retValue * player->getRateValue(LEVEL_EXPERIENCE));
		}
	}

	retValue = (uint64_t)std::floor(retValue * (float)g_config.getNumber(ConfigManager::RATE_EXPERIENCE));
	return retValue;
}

void Creature::addDamagePoints(Creature* attacker, int32_t damagePoints)
{
	if(damagePoints > 0){
		uint32_t attackerId = (attacker ? attacker->getID() : 0);

		CountMap::iterator it = damageMap.find(attackerId);
		if(it == damageMap.end()){
			CountBlock_t cb;
			cb.ticks = OTSYS_TIME();
			cb.total = damagePoints;
			damageMap[attackerId] = cb;
		}
		else{
			it->second.total += damagePoints;
			it->second.ticks = OTSYS_TIME();
		}

		lastHitCreature = attackerId;
	}
}

void Creature::addHealPoints(Creature* caster, int32_t healthPoints)
{
	if(healthPoints > 0){
		uint32_t casterId = (caster ? caster->getID() : 0);

		CountMap::iterator it = healMap.find(casterId);
		if(it == healMap.end()){
			CountBlock_t cb;
			cb.ticks = OTSYS_TIME();
			cb.total = healthPoints;
			healMap[casterId] = cb;
		}
		else{
			it->second.total += healthPoints;
			it->second.ticks = OTSYS_TIME();
		}
	}
}

void Creature::onAddCondition(ConditionType_t type)
{
	if(type == CONDITION_PARALYZE && hasCondition(CONDITION_HASTE)){
		removeCondition(CONDITION_HASTE);
	}
	else if(type == CONDITION_HASTE && hasCondition(CONDITION_PARALYZE)){
		removeCondition(CONDITION_PARALYZE);
	}
}

void Creature::onAddCombatCondition(ConditionType_t type)
{
	//
}

void Creature::onEndCondition(ConditionType_t type)
{
	//
}

void Creature::onTickCondition(ConditionType_t type, bool& bRemove)
{
	if(const MagicField* field = getTile()->getFieldItem()){
		switch(type){
			case CONDITION_FIRE: bRemove = (field->getCombatType() != COMBAT_FIREDAMAGE); break;
			case CONDITION_ENERGY: bRemove = (field->getCombatType() != COMBAT_ENERGYDAMAGE); break;
			case CONDITION_POISON: bRemove = (field->getCombatType() != COMBAT_POISONDAMAGE); break;
			default:
				break;
		}
	}
}

void Creature::onCombatRemoveCondition(const Creature* attacker, Condition* condition)
{
	removeCondition(condition);
}

void Creature::onAttackedCreature(Creature* target)
{
	//
}

void Creature::onAttacked()
{
	//
}

void Creature::onIdleStatus()
{
	if(getHealth() > 0){
		healMap.clear();
		damageMap.clear();
	}
}

void Creature::onAttackedCreatureDrainHealth(Creature* target, int32_t points)
{
	target->addDamagePoints(this, points);
}

void Creature::onTargetCreatureGainHealth(Creature* target, int32_t points)
{
	target->addHealPoints(this, points);
}

void Creature::onAttackedCreatureKilled(Creature* target)
{
	if(target != this){
		uint64_t gainExp = target->getGainedExperience(this);
		onGainExperience(gainExp);
	}
}

void Creature::onKilledCreature(Creature* target, bool lastHit)
{
	if(getMaster()){
		getMaster()->onKilledCreature(target, lastHit);
	}

	//scripting event - onKill
	onKillEvent(target, lastHit);
}

void Creature::onGainExperience(uint64_t gainExp)
{
	if(gainExp > 0){
		if(getMaster()){
			gainExp = gainExp / 2;
			getMaster()->onGainExperience(gainExp);
		}

		std::stringstream strExp;
		strExp << gainExp;
		g_game.addAnimatedText(getPosition(), TEXTCOLOR_WHITE_EXP, strExp.str());
	}
}

void Creature::onAttackedCreatureBlockHit(Creature* target, int blockType)
{
	//
}

void Creature::onBlockHit(int blockType)
{
	//
}

void Creature::addSummon(Creature* creature)
{
	//std::cout << "addSummon: " << this << " summon=" << creature << std::endl;
	creature->setDropLoot(false);
	creature->setLossSkill(false);
	creature->setMaster(this);
	creature->useThing2();
	summons.push_back(creature);
}

void Creature::removeSummon(const Creature* creature)
{
	//std::cout << "removeSummon: " << this << " summon=" << creature << std::endl;
	std::list<Creature*>::iterator cit = std::find(summons.begin(), summons.end(), creature);
	if(cit != summons.end()){
		(*cit)->setDropLoot(false);
		(*cit)->setLossSkill(true);
		(*cit)->setMaster(NULL);
		(*cit)->releaseThing2();
		summons.erase(cit);
	}
}


void Creature::addCombatBleeding(double hpToBlend)
{
	if (hasCondition(CONDITION_BLEEDING))
		removeCondition(CONDITION_BLEEDING);
	Condition* condition = Condition::createCondition(CONDITIONID_COMBAT, CONDITION_BLEEDING, 2000, 0);
	condition->setParam(CONDITIONPARAM_OWNER, getID());
	condition->setParam(CONDITIONPARAM_TOTAL_DMG, hpToBlend);
	condition->setParam(CONDITIONPARAM_DELAYED, true);
	
	//intervals
	condition->setParam(CONDITIONPARAM_INTERVAL_MAX, std::ceil(10.0 * (1 - std::pow(0.5,-hpToBlend/40.0))));
	addCondition(condition);
}

bool Creature::addCondition(Condition* condition)
{
	if(condition == NULL){
		return false;
	}

	Condition* prevCond = getCondition(condition->getType(), condition->getId());

	if(prevCond){
		prevCond->addCondition(this, condition);
		delete condition;
		return true;
	}

	if(condition->startCondition(this)){
		conditions.push_back(condition);
		onAddCondition(condition->getType());
		return true;
	}

	delete condition;
	return false;
}

bool Creature::addCombatCondition(Condition* condition)
{
	//Caution: condition variable could be deleted after the call to addCondition
	ConditionType_t type = condition->getType();
	if(!addCondition(condition)){
		return false;
	}

	onAddCombatCondition(type);
	return true;
}

void Creature::removeCondition(ConditionType_t type)
{
	for(ConditionList::iterator it = conditions.begin(); it != conditions.end();){
		if((*it)->getType() == type){
			Condition* condition = *it;
			it = conditions.erase(it);

			condition->endCondition(this, CONDITIONEND_ABORT);
			delete condition;

			onEndCondition(type);
		}
		else{
			++it;
		}
	}
}

void Creature::removeCondition(ConditionType_t type, ConditionId_t id)
{
	for(ConditionList::iterator it = conditions.begin(); it != conditions.end();){
		if((*it)->getType() == type && (*it)->getId() == id){
			Condition* condition = *it;
			it = conditions.erase(it);

			condition->endCondition(this, CONDITIONEND_ABORT);
			delete condition;

			onEndCondition(type);
		}
		else{
			++it;
		}
	}
}

void Creature::removeCondition(const Creature* attacker, ConditionType_t type)
{
	ConditionList tmpList = conditions;

	for(ConditionList::iterator it = tmpList.begin(); it != tmpList.end(); ++it){
		if((*it)->getType() == type){
			onCombatRemoveCondition(attacker, *it);
		}
	}
}

void Creature::removeCondition(Condition* condition)
{
	ConditionList::iterator it = std::find(conditions.begin(), conditions.end(), condition);

	if(it != conditions.end()){
		Condition* condition = *it;
		it = conditions.erase(it);

		condition->endCondition(this, CONDITIONEND_ABORT);
		onEndCondition(condition->getType());
		delete condition;
	}
}

Condition* Creature::getCondition(ConditionType_t type, ConditionId_t id) const
{
	for(ConditionList::const_iterator it = conditions.begin(); it != conditions.end(); ++it){
		if((*it)->getType() == type && (*it)->getId() == id){
			return *it;
		}
	}

	return NULL;
}

Condition* Creature::getCondition(ConditionType_t type) const
{
	//This one just returns the first one found.
	for(ConditionList::const_iterator it = conditions.begin(); it != conditions.end(); ++it){
		if((*it)->getType() == type){
			return *it;
		}
	}

	return NULL;
}

void Creature::executeConditions(uint32_t interval)
{
	for(ConditionList::iterator it = conditions.begin(); it != conditions.end();){
		//(*it)->executeCondition(this, newticks);
		//if((*it)->getTicks() <= 0){

		if(!(*it)->executeCondition(this, interval)){
			ConditionType_t type = (*it)->getType();

			Condition* condition = *it;
			it = conditions.erase(it);

			condition->endCondition(this, CONDITIONEND_TICKS);
			delete condition;

			onEndCondition(type);
		}
		else{
			++it;
		}
	}
}

bool Creature::hasCondition(ConditionType_t type, bool checkTime /*= true*/) const
{
	if(isSuppress(type)){
		return false;
	}

	for(ConditionList::const_iterator it = conditions.begin(); it != conditions.end(); ++it){
		if((*it)->getType() == type && (!checkTime || ((*it)->getEndTime() == 0 || (*it)->getEndTime() >= OTSYS_TIME())) ){
			return true;
		}
	}

	return false;
}

bool Creature::isImmune(CombatType_t type) const
{
	return ((getDamageImmunities() & (uint32_t)type) == (uint32_t)type);
}

bool Creature::isImmune(ConditionType_t type, bool aggressive /* = true */) const
{
	return ((getConditionImmunities() & (uint32_t)type) == (uint32_t)type);
}

bool Creature::isSuppress(ConditionType_t type) const
{
	return ((getConditionSuppressions() & (uint32_t)type) == (uint32_t)type);
}

std::string Creature::getDescription(int32_t lookDistance) const
{
	std::string str = "a creature";
	return str;
}

std::string Creature::getXRayDescription() const
{
	std::stringstream ret;
	ret << "Health: [" << getHealth() << "/" << getMaxHealth() << "]" << std::endl;
	ret << "Mana: [" << getMana() << "/" << getMaxMana() << "]" << std::endl;
	ret << Thing::getXRayDescription();
	return ret.str();
}

int32_t Creature::getStepDuration(Direction dir) const
{
	int32_t stepDuration = getStepDuration();

	if(dir == NORTHWEST || dir == NORTHEAST || dir == SOUTHWEST || dir == SOUTHEAST){
		stepDuration *= 1;
	}

	return stepDuration;
}

int32_t Creature::getStepDuration() const
{
	if(isRemoved()){
		return 0;
	}

	int32_t duration = 0;
	const Tile* tile = getTile();
	if(tile && tile->ground){
		uint32_t groundId = tile->ground->getID();
		uint16_t groundSpeed = Item::items[groundId].speed;
		uint32_t stepSpeed = getStepSpeed();
		if(stepSpeed != 0){
			duration = (1000 * groundSpeed) / stepSpeed;
		}
	}

	return duration * lastStepCost;
}

int64_t Creature::getEventStepTicks() const
{
	int64_t ret = getWalkDelay();

	if(ret <= 0){
		ret = getStepDuration();
	}

	return ret;
}

bool Creature::isInRange(Creature * target)
{
	Position pos = getPosition();
	Position targetPosition = target->getPosition();

	if (std::abs(pos.x - targetPosition.x) <= range && std::abs(pos.y - targetPosition.y) <= range)
		return g_game.isSightClear(pos, targetPosition, true);
	else
		return false;
}

void Creature::getCreatureLight(LightInfo& light) const
{
	light = internalLight;
}

void Creature::setNormalCreatureLight()
{
	internalLight.level = 0;
	internalLight.color = 0;
}

bool Creature::registerCreatureEvent(const std::string& name)
{
	CreatureEvent* event = g_creatureEvents->getEventByName(name);
	if(event){
		CreatureEventType_t type = event->getEventType();
		if(!hasEventRegistered(type)){
			// was not added, so set the bit in the bitfield
			scriptEventsBitField = scriptEventsBitField | ((uint32_t)1 << type);
		}

		eventsList.push_back(event);
		return true;
	}

	return false;
}

CreatureEventList Creature::getCreatureEvents(CreatureEventType_t type)
{
	CreatureEventList typeList;
	if(hasEventRegistered(type)){
		CreatureEventList::iterator it;
		for(it = eventsList.begin(); it != eventsList.end(); ++it){
			if((*it)->getEventType() == type){
				typeList.push_back(*it);
			}
		}
	}

	return typeList;
}

void Creature::onDieEvent(Item* corpse)
{
	CreatureEventList dieEvents = getCreatureEvents(CREATURE_EVENT_DIE);
	for(CreatureEventList::iterator it = dieEvents.begin(); it != dieEvents.end(); ++it){
		(*it)->executeOnDie(this, corpse);
	}
}

void Creature::onKillEvent(Creature* target, bool lastHit)
{
	CreatureEventList killEvents = getCreatureEvents(CREATURE_EVENT_KILL);
	for(CreatureEventList::iterator it = killEvents.begin(); it != killEvents.end(); ++it){
		(*it)->executeOnKill(this, target, lastHit);
	}
}

FrozenPathingConditionCall::FrozenPathingConditionCall(const Position& _targetPos)
{
	targetPos = _targetPos;
}

bool FrozenPathingConditionCall::isInRange(const Position& startPos, const Position& testPos,
	const FindPathParams& fpp) const
{
	int32_t dxMin = ((fpp.fullPathSearch || (startPos.x - targetPos.x) <= 0) ? fpp.maxTargetDist : 0);
	int32_t dxMax = ((fpp.fullPathSearch || (startPos.x - targetPos.x) >= 0) ? fpp.maxTargetDist : 0);
	int32_t dyMin = ((fpp.fullPathSearch || (startPos.y - targetPos.y) <= 0) ? fpp.maxTargetDist : 0);
	int32_t dyMax = ((fpp.fullPathSearch || (startPos.y - targetPos.y) >= 0) ? fpp.maxTargetDist : 0);

	if(testPos.x > targetPos.x + dxMax || testPos.x < targetPos.x - dxMin){
		return false;
	}

	if(testPos.y > targetPos.y + dyMax || testPos.y < targetPos.y - dyMin){
		return false;
	}

	return true;
}

bool FrozenPathingConditionCall::operator()(const Position& startPos, const Position& testPos,
	const FindPathParams& fpp, int32_t& bestMatchDist) const
{
	if(!isInRange(startPos, testPos, fpp)){
		return false;
	}

	if(fpp.clearSight && !g_game.isSightClear(testPos, targetPos, true)){
		return false;
	}

	int32_t testDist = std::max(std::abs(targetPos.x - testPos.x), std::abs(targetPos.y - testPos.y));
	if(fpp.maxTargetDist == 1){
		if(testDist < fpp.minTargetDist || testDist > fpp.maxTargetDist){
			return false;
		}

		return true;
	}
	else if(testDist <= fpp.maxTargetDist){
		if(testDist < fpp.minTargetDist){
			return false;
		}

		if(testDist == fpp.maxTargetDist){
			bestMatchDist = 0;
			return true;
		}
		else if(testDist > bestMatchDist){
			//not quite what we want, but the best so far
			bestMatchDist = testDist;
			return true;
		}
	}

	return false;
}
