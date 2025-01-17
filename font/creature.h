//////////////////////////////////////////////////////////////////////
// OpenTibia - an opensource roleplaying game
//////////////////////////////////////////////////////////////////////
// base class for every creature
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


#ifndef __OTSERV_CREATURE_H__
#define __OTSERV_CREATURE_H__

#include "definitions.h"
#include "templates.h"
#include "map.h"
#include "position.h"
#include "condition.h"
#include "const.h"
#include "tile.h"
#include "enums.h"
#include "creatureevent.h"

#include <list>

#define NUMBER_OF_SKILLS  11

typedef std::list<Condition*> ConditionList;
typedef std::list<CreatureEvent*> CreatureEventList;

enum slots_t {
	SLOT_FIRST = 1,
	SLOT_HEAD = SLOT_FIRST,
	SLOT_ARMOR,
	SLOT_BELT,
	SLOT_LEGS,
	SLOT_FEET,
	SLOT_NECKLACE,
	SLOT_RIGHT,
	SLOT_RING,
	SLOT_GLOOVES,
	SLOT_ROBE,
	SLOT_LEFT,
	SLOT_BACKPACK,
	SLOT_BAG,
	SLOT_BRACELET,
	SLOT_EXTRA,
	
	// Special slot, covers two, not a real slot
	SLOT_HAND = 12,

	SLOT_AMMO,
	SLOT_TWO_HAND = SLOT_HAND, // alias

	// Last real slot is depot
	SLOT_LAST = SLOT_EXTRA
};

struct FindPathParams{
	bool fullPathSearch;
	bool clearSight;
	bool allowDiagonal;
	bool keepDistance;
	int32_t maxSearchDist;
	int32_t minTargetDist;
	int32_t maxTargetDist;

	FindPathParams()
	{
		clearSight = true;
		fullPathSearch = true;
		allowDiagonal = true;
		keepDistance = false;
		maxSearchDist = -1;
		minTargetDist = -1;
		maxTargetDist = -1;
	}
};

enum ZoneType_t{
    ZONE_PROTECTION,
    ZONE_NOPVP,
    ZONE_PVP,
    ZONE_NOLOGOUT,
    ZONE_NORMAL
};


class Map;
class Thing;
class Container;
class Player;
class Monster;
class Npc;
class Item;
class Tile;

#define EVENT_CREATURECOUNT 10
#define EVENT_CREATURE_THINK_INTERVAL 1000
#define EVENT_CHECK_CREATURE_INTERVAL (EVENT_CREATURE_THINK_INTERVAL / EVENT_CREATURECOUNT)
#define EVENT_CREATURE_INTERVAL 100

class FrozenPathingConditionCall {
public:
	FrozenPathingConditionCall(const Position& _targetPos);
	virtual ~FrozenPathingConditionCall() {}
	
	virtual bool operator()(const Position& startPos, const Position& testPos,
		const FindPathParams& fpp, int32_t& bestMatchDist) const;

	bool isInRange(const Position& startPos, const Position& testPos,
		const FindPathParams& fpp) const;

protected:
	Position targetPos;
};

//////////////////////////////////////////////////////////////////////
// Defines the Base class for all creatures and base functions which
// every creature has

class Creature : public AutoID, virtual public Thing
{
protected:
	Creature();
	
public:
	virtual ~Creature();


	void onAttacking(uint32_t interval);
	virtual void onAttacked();
	virtual void doAttacking() {}
	uint16_t getAttackSpeed() const;
	void resetAttackTicks();
	Item* getInventoryItem(slots_t slot) const;
	Item* getWeapon() const;
	Item* getShield() const;
	
	bool whereDmgTook(Creature *attacker, int32_t& blockType, double & defenseBodyFactor, int32_t & defenseItemFactor);
	virtual int32_t getBlockDmg(const double defenseBodyFactor, const int32_t defenseItemFactor) { return 0; }
	virtual bool hasConcentration() { return true; }
	virtual int32_t getAttackDmg(struct _weaponDamage_ * wd) { return -1; }


	virtual Creature* getCreature() {return this;}
	virtual const Creature* getCreature()const {return this;}
	virtual Player* getPlayer() {return NULL;}
	virtual const Player* getPlayer() const {return NULL;}
	virtual Npc* getNpc() {return NULL;}
	virtual const Npc* getNpc() const {return NULL;}
	virtual Monster* getMonster() {return NULL;}
	virtual const Monster* getMonster() const {return NULL;}

	bool isDying;
	
	void getPathToFollowCreature();

	virtual const std::string& getName() const = 0;
	virtual const std::string& getNameDescription() const = 0;
	virtual std::string getXRayDescription() const;
	virtual std::string getDescription(int32_t lookDistance) const;

	void setID(){this->id = auto_id | this->idRange();}
	void setRemoved() {isInternalRemoved = true;}

	virtual uint32_t idRange() = 0;
	uint32_t getID() const { return id; }
	virtual void removeList() = 0;
	virtual void addList() = 0;

	virtual bool canSee(const Position& pos) const;
	virtual bool canSeeCreature(const Creature* creature) const;
	//virtual bool canBeSeen(const Creature* viewer, bool checkVisibility = true) const;
	//virtual bool canWalkthrough(const Creature* creature) const;

	virtual RaceType_t getRace() const {return RACE_NONE;}
	Direction getDirection() const { return direction;}
	void setDirection(Direction dir) { direction = dir;}

	const Position& getMasterPos() const { return masterPos;}
	void setMasterPos(const Position& pos, uint32_t radius = 1) { masterPos = pos; masterRadius = radius;}

	virtual int getThrowRange() const {return 1;}
	virtual bool isPushable() const {return (getWalkDelay() <= 0);}
	virtual bool isRemoved() const {return isInternalRemoved;}
	virtual bool canSeeInvisibility() const { return false;}

	int32_t getWalkDelay(Direction dir) const;
	int32_t getWalkDelay() const;
	int64_t getTimeSinceLastMove() const;
	
	virtual int64_t getEventStepTicks() const;
	int32_t getStepDuration(Direction dir) const;
	int32_t getStepDuration() const;

	virtual int32_t getStepSpeed() const {return getSpeed();}
	int32_t getSpeed() const {return baseSpeed + varSpeed;}
	void setSpeed(int32_t varSpeedDelta)
	{
        int32_t oldSpeed = getSpeed();
        varSpeed = varSpeedDelta;
        if(getSpeed() <= 0){
            stopEventWalk();
        }
        else if(oldSpeed <= 0 && !listWalkDir.empty()){
            addEventWalk();
        }
    }

	void setBaseSpeed(uint32_t newBaseSpeed) {baseSpeed = newBaseSpeed;}
	int getBaseSpeed() {return baseSpeed;}

	virtual int32_t getHealth() const {return health;}
	virtual int32_t getMaxHealth() const {return healthMax;}
	virtual int32_t getMana() const {return mana;}
	virtual int32_t getMaxMana() const {return manaMax;}

	const Outfit_t getCurrentOutfit() const {return currentOutfit;}
	const void setCurrentOutfit(Outfit_t outfit) {currentOutfit = outfit;}
	const uint32_t getAttackOutfit() const { return m_attackOutfit; }
	const uint32_t getBreathOutfit() const { return m_breathOutfit; }
	const uint32_t getWalkAttackOutfit() const { return m_walkAttackOutfit; }
	
	const void setAttackOutfit(uint32_t attackOutfit) { m_attackOutfit = attackOutfit; }
	const void setBreathOutfit(uint32_t breathOutfit) { m_breathOutfit = breathOutfit; }
	const void setWalkAttackOutfit(uint32_t walkAttackOutfit) { m_walkAttackOutfit = walkAttackOutfit; }
	const Outfit_t getDefaultOutfit() const {return defaultOutfit;}
	bool isInvisible() const {return hasCondition(CONDITION_INVISIBLE);}
	ZoneType_t getZone() const {
        const Tile* tile = getTile();
        if(tile->hasFlag(TILESTATE_PROTECTIONZONE)){
            return ZONE_PROTECTION;
        }
        else if(tile->hasFlag(TILESTATE_NOPVPZONE)){
            return ZONE_NOPVP;
        }
        else if(tile->hasFlag(TILESTATE_PVPZONE)){
            return ZONE_PVP; 
        }
        else{
            return ZONE_NORMAL;
        }
    }

	//walk functions
	bool startAutoWalk(std::list<Direction>& listDir);
	void addEventWalk();
	void stopEventWalk();

	//walk events
	virtual void onWalk(Direction& dir);
	virtual void onWalkAborted() {};
	virtual void onWalkComplete() {};

	//follow functions
	virtual Creature* getFollowCreature() const { return followCreature; };
	virtual bool setFollowCreature(Creature* creature, bool fullPathSearch = false);

	//follow events
	virtual void onFollowCreature(const Creature* creature) {};
	virtual void onFollowCreatureComplete(const Creature* creature) {};

	//combat functions
	Creature* getAttackedCreature() { return attackedCreature; }
	virtual bool setAttackedCreature(Creature* creature);
	virtual int blockHit(Creature* attacker, CombatType_t combatType, int * blockType, void * data = nullptr);
	virtual int getAvoindanceDefense() { return 0; }

	

	double getSkillValue(uint8_t id) const { return this->m_skills[id]; }
	void setSkillValue(uint8_t id, double value) { this->m_skills[id] = value; }
	virtual uint8_t getAttackSkillType(WeaponType_t) const;


	void setMaster(Creature* creature) {master = creature;}
	Creature* getMaster() {return master;}
	bool isSummon() const {return master != NULL;}
	const Creature* getMaster() const {return master;}

	virtual void addSummon(Creature* creature);
	virtual void removeSummon(const Creature* creature);
	const std::list<Creature*>& getSummons() {return summons;}

	virtual int32_t getShieldDefense() const { return 0; }
	virtual int32_t getWeaponDefense() const { return 0; }
	virtual int32_t getDefenseFactors() const {return 0;}
	virtual int32_t getDefense() const {return 0;}
	virtual float getAttackFactor() const {return 1.0f;}
	virtual float getDefenseFactor() const {return 1.0f;}

	void addCombatBleeding(double hpToBleend);

	bool addCondition(Condition* condition);
	bool addCombatCondition(Condition* condition);
	void removeCondition(ConditionType_t type, ConditionId_t id);
	void removeCondition(ConditionType_t type);
	void removeCondition(Condition* condition);
	void removeCondition(const Creature* attacker, ConditionType_t type);
	Condition* getCondition(ConditionType_t type, ConditionId_t id) const;
	Condition* getCondition(ConditionType_t type) const;
	void executeConditions(uint32_t interval);
	bool hasCondition(ConditionType_t type, bool checkTime = true) const;
	virtual bool isImmune(ConditionType_t type, bool aggressive = true) const;
	virtual bool isImmune(CombatType_t type) const;
	virtual bool isSuppress(ConditionType_t type) const;
	virtual uint32_t getDamageImmunities() const { return 0; }
	virtual uint32_t getConditionImmunities() const { return 0; }
	virtual uint32_t getConditionSuppressions() const { return 0; }
	virtual bool isAttackable() const { return true;}
    bool isIdle() const { return checkCreatureVectorIndex == 0;}
	virtual void changeHealth(int32_t healthChange);
	virtual void changeMana(int32_t manaChange);

    virtual void gainHealth(Creature* caster, int32_t healthGain);
	virtual void drainHealth(Creature* attacker, CombatType_t combatType, int32_t damage);
	virtual void drainMana(Creature* attacker, int32_t manaLoss);

	virtual bool challengeCreature(Creature* creature) {return false;}
	virtual bool convinceCreature(Creature* creature) {return false;}

	virtual void onDie();
	virtual uint64_t getGainedExperience(Creature* attacker, bool useMultiplier = true) const;
	void addDamagePoints(Creature* attacker, int32_t damagePoints);
	void addHealPoints(Creature* caster, int32_t healthPoints);
	bool hasBeenAttacked(uint32_t attackerId);

	//combat event functions
	virtual void onAddCondition(ConditionType_t type);
	virtual void onAddCombatCondition(ConditionType_t type);
	virtual void onEndCondition(ConditionType_t type);
	virtual void onTickCondition(ConditionType_t type, bool& bRemove);
	virtual void onCombatRemoveCondition(const Creature* attacker, Condition* condition);
	virtual void onAttackedCreature(Creature* target);
	virtual void onAttackedCreatureDrainHealth(Creature* target, int32_t points);
	virtual void onTargetCreatureGainHealth(Creature* target, int32_t points);
	virtual void onAttackedCreatureKilled(Creature* target);
	virtual void onKilledCreature(Creature* target, bool lastHit);
	virtual void onGainExperience(uint64_t gainExp);
	virtual void onAttackedCreatureBlockHit(Creature* target, int blockType);
	virtual void onBlockHit(int blockType);
	virtual void onChangeZone(ZoneType_t zone);
	virtual void onAttackedCreatureChangeZone(ZoneType_t zone);
	virtual void onIdleStatus();

	bool isInRange(Creature * target);


	virtual void getCreatureLight(LightInfo& light) const;
	virtual void setNormalCreatureLight();
	void setCreatureLight(LightInfo& light) {internalLight = light;}

	virtual void onThink(uint32_t interval);
	virtual void onWalk();
	virtual bool getNextStep(Direction& dir);

	virtual void onAddTileItem(const Tile* tile, const Position& pos, const Item* item);
	virtual void onUpdateTileItem(const Tile* tile, const Position& pos, uint32_t stackpos,
	    const Item* oldItem, const ItemType& oldType, const Item* newItem, const ItemType& newType);
	virtual void onRemoveTileItem(const Tile* tile, const Position& pos, uint32_t stackpos,
	    const ItemType& iType, const Item* item);
	virtual void onUpdateTile(const Tile* tile, const Position& pos);

	virtual void onCreatureAppear(const Creature* creature, bool isLogin);
	virtual void onCreatureDisappear(const Creature* creature, uint32_t stackpos, bool isLogout);
	virtual void onCreatureMove(const Creature* creature, const Tile* newTile, const Position& newPos,
		const Tile* oldTile, const Position& oldPos, uint32_t oldStackPos, bool teleport);

	virtual void onAttackedCreatureDissapear(bool isLogout) {};
	virtual void onFollowCreatureDissapear(bool isLogout) {};

	virtual void onCreatureTurn(const Creature* creature, uint32_t stackPos) { };
	virtual void onCreatureSay(const Creature* creature, MessageClasses type, const std::string& text) { };

	virtual void onCreatureChangeOutfit(const Creature* creature, const Outfit_t& outfit) { };
	virtual void onCreatureConvinced(const Creature* convincer, const Creature* creature) {};
	virtual void onCreatureChangeVisible(const Creature* creature, bool visible);
	virtual void onPlacedCreature() {};
	virtual void onRemovedCreature() {};

	virtual WeaponType_t getWeaponType() {return WEAPON_TYPE_NONE;}
	virtual bool getCombatValues(int32_t& min, int32_t& max) {return false;}

	size_t getSummonCount() const {return summons.size();}
	void setDropLoot(bool _lootDrop) {lootDrop = _lootDrop;}
	void setLossSkill(bool _skillLoss) {skillLoss = _skillLoss;}

	//creature script events
	bool registerCreatureEvent(const std::string& name);

	virtual void setParent(Cylinder* cylinder){
		_tile = dynamic_cast<Tile*>(cylinder);
		Thing::setParent(cylinder);
	}

	virtual const Position& getPosition() const {return _tile->getTilePosition();}
	virtual Tile* getTile(){return _tile;}
	virtual const Tile* getTile() const{return _tile;}
	int32_t getWalkCache(const Position& pos) const;

	static bool canSee(const Position& myPos, const Position& pos, uint32_t viewRangeX, uint32_t viewRangeY);
    bool getHasFollowPath() {return hasFollowPath;}

protected:
	static const int32_t mapWalkWidth = Map::maxViewportX * 2 + 1;
	static const int32_t mapWalkHeight = Map::maxViewportY * 2 + 1;
	bool localMapCache[mapWalkHeight][mapWalkWidth];

	virtual bool useCacheMap() const {return false;}

	Tile* _tile;
	uint32_t id;
	bool isInternalRemoved;
	bool isMapLoaded;
	bool isUpdatingPath;
	// The creature onThink event vector this creature belongs to
	// The value stored here is actually 1 larger than the index,
	// this is to allow 0 to represent the special value of not
	// being stored in any onThink vector
	size_t checkCreatureVectorIndex;
	
	int32_t health, healthMax;
	int32_t mana, manaMax;

	Outfit_t currentOutfit;
	Outfit_t defaultOutfit;
	uint32_t m_attackOutfit;
	uint32_t m_breathOutfit;
	uint32_t m_walkAttackOutfit;

	Position masterPos;
	int32_t masterRadius;
	uint64_t lastStep;
	uint32_t lastStepCost;
	uint32_t extraStepDuration;
	uint32_t baseSpeed;
	int32_t varSpeed;
	bool skillLoss;
	bool lootDrop;
	int range;


	double m_skills[NUMBER_OF_SKILLS];

	Direction direction;
	ConditionList conditions;
	LightInfo internalLight;

	//summon variables
	Creature* master;
	std::list<Creature*> summons;

	//follow variables
	Creature* followCreature;
	uint32_t eventWalk;
	std::list<Direction> listWalkDir;
	uint32_t walkUpdateTicks;
	bool hasFollowPath;
	bool forceUpdateFollowPath;
	
	//combat variables
	Creature* attackedCreature;

	struct CountBlock_t{
		int32_t total;
		int64_t ticks;
	};

	typedef std::map<uint32_t, CountBlock_t> CountMap;
	CountMap damageMap;
	CountMap healMap;
	uint32_t lastHitCreature;
	uint32_t blockCount;
	uint32_t blockTicks;

    //creature script events
	bool hasEventRegistered(CreatureEventType_t event){
		return (0 != (scriptEventsBitField & ((uint32_t)1 << event)));
	}
	uint32_t scriptEventsBitField;
	typedef std::list<CreatureEvent*> CreatureEventList;
	CreatureEventList eventsList;
	CreatureEventList getCreatureEvents(CreatureEventType_t type);
	void onDieEvent(Item* corpse);
	void onKillEvent(Creature* target, bool lastHit);

	void updateMapCache();
#ifdef __DEBUG__
	void validateMapCache();
#endif
	void updateTileCache(const Tile* tile, int32_t dx, int32_t dy);
	void updateTileCache(const Tile* tile, const Position& pos);
	void onCreatureDisappear(const Creature* creature, bool isLogout);
	virtual bool hasExtraSwing() {return false;}

	virtual uint64_t getLostExperience() const { return 0; };
	virtual double getDamageRatio(Creature* attacker) const;
	bool getKillers(Creature** lastHitCreature, Creature** mostDamageCreature);
	virtual void dropLoot(Container* corpse) {};
	virtual uint16_t getLookCorpse() const { return 0; }
	virtual void getPathSearchParams(const Creature* creature, FindPathParams& fpp) const;
	virtual void die() {};
	virtual void dropCorpse();
	virtual Item* getCorpse();

	friend class Game;
	friend class Map;
	friend class Commands;
	friend class LuaScriptInterface;

	uint32_t m_attackTicks;
	uint16_t m_attackSpeed;
	bool m_attacking;
	//Creature * defenderCreature;
	Item* inventory[16];
	//inventory variables
	bool inventoryAbilities[16];
};


#endif
