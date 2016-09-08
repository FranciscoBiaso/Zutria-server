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

#include "combat.h"

#include "configmanager.h"
#include "game.h"
#include "creature.h"
#include "player.h"
#include "const.h"
#include "tools.h"
#include "spells.h"
#include "graphicsmath.h"

#include <sstream>

extern Game g_game;
extern Weapons* g_weapons;
extern ConfigManager g_config;
extern Spells* g_spells;

Combat::Combat()
{
	params.condition = NULL;
	params.valueCallback = NULL;
	params.tileCallback = NULL;
	params.targetCallback = NULL;
	area = NULL;

	formulaType = FORMULA_UNDEFINED;
	mina = 0.0;
	minb = 0.0;
	maxa = 0.0;
	maxb = 0.0;
}

Combat::~Combat()
{
	delete params.condition;
	delete params.valueCallback;
	delete params.tileCallback;
	delete params.targetCallback;
	delete area;
}

bool Combat::getMinMaxValues(Creature* creature, Creature* target, int32_t& min, int32_t& max) const
{
	if(!creature){
		return false;
	}

	if(creature->getCombatValues(min, max)){
		return true;
	}
	else if(Player* player = creature->getPlayer()){
		if(params.valueCallback){
			params.valueCallback->getMinMaxValues(player, min, max, params.useCharges);
			return true;
		}
		else{
			switch(formulaType){
				case FORMULA_LEVELMAGIC:
				{
					//max = (int32_t)((player->getLevel() * 2 + player->getMagicLevel() * 3) * 1. * mina + minb);
					//min = (int32_t)((player->getLevel() * 2 + player->getMagicLevel() * 3) * 1. * maxa + maxb);
					max = maxa;
					min = mina;
					return true;
				}

				case FORMULA_SKILL:
				{
					Item* tool = player->getWeapon();
					const Weapon* weapon = g_weapons->getWeapon(tool);

					min = (int32_t)minb;

					if(weapon){
						max = (int32_t)(weapon->getWeaponDamage(player, target, tool, true) * maxa + maxb);
						if(params.useCharges && tool->hasCharges() && g_config.getBoolean(ConfigManager::REMOVE_WEAPON_CHARGES)){
							int32_t newCharge = std::max((int32_t)0, ((int32_t)tool->getCharges()) - 1);
							g_game.transformItem(tool, tool->getID(), newCharge);
						}
					}
					else{
						max = (int32_t)maxb;
					}

                    return true;
				}
				
				case FORMULA_VALUE:
                {
                    min = (int32_t)mina;
                    max = (int32_t)maxa;
                    
                    return true;
                }

				default:
					min = 0;
					max = 0;
					return false;
			}

			//std::cout << "No callback set for combat" << std::endl;
		}
	}
	else if(formulaType == FORMULA_VALUE){
		min = (int32_t)mina;
		max = (int32_t)maxa;
		return true;
	}
	
	return false;
}

void Combat::getCombatArea(const Position& centerPos, const Position& targetPos, const AreaCombat* area,
	std::list<std::pair<Tile*,uint8_t>>& list)
{
	if(area){
		area->getList(centerPos, targetPos, list);
	}
	else if(targetPos.x >= 0 && targetPos.y >= 0 && targetPos.z >= 0 &&
		targetPos.x <= 0xFFFF && targetPos.y <= 0xFFFF && targetPos.z < MAP_MAX_LAYERS)
	{
		Tile* tile = g_game.getTile(targetPos.x, targetPos.y, targetPos.z);
		if(!tile) {
            tile = new Tile(targetPos.x, targetPos.y, targetPos.z);
            g_game.setTile(tile);
        }
        
		list.push_back(std::make_pair(tile,0));
	}
}

CombatType_t Combat::ConditionToDamageType(ConditionType_t type)
{
	switch(type){
		case CONDITION_FIRE:
			return COMBAT_FIREDAMAGE;
			break;

		case CONDITION_ENERGY:
			return COMBAT_ENERGYDAMAGE;
			break;

		case CONDITION_POISON:
			return COMBAT_POISONDAMAGE;
			break;

		case CONDITION_BLEEDING:
			return COMBAT_BLEEDING;
			break;

		default:
			break;
	}

	return COMBAT_NONE;
}

ConditionType_t Combat::DamageToConditionType(CombatType_t type)
{
	switch(type){
		case COMBAT_FIREDAMAGE: return CONDITION_FIRE;
		case COMBAT_ENERGYDAMAGE: return CONDITION_ENERGY;
		case COMBAT_POISONDAMAGE: return CONDITION_POISON;
		case COMBAT_BLEEDING: return CONDITION_BLEEDING;
		default: break;
	}

	return CONDITION_NONE;
}

bool Combat::isPlayerCombat(const Creature* target)
{
	if(target->getPlayer()){
		return true;
	}

	if(target->isSummon() && target->getMaster()->getPlayer()){
		return true;
	}

	return false;
}

ReturnValue Combat::canTargetCreature(const Player* player, const Creature* target)
{
	if(player == target){
		return RET_YOUMAYNOTATTACKTHISPLAYER;
	}

	if(!player->hasFlag(PlayerFlag_IgnoreProtectionZone)){
		//pz-zone
		if(player->getZone() == ZONE_PROTECTION){
			return RET_YOUMAYNOTATTACKAPERSONWHILEINPROTECTIONZONE;
		}
		if(target->getZone() == ZONE_PROTECTION){
			return RET_YOUMAYNOTATTACKAPERSONINPROTECTIONZONE;
		}

		//nopvp-zone
		if(isPlayerCombat(target)){
			if(player->getZone() == ZONE_NOPVP){
				return RET_ACTIONNOTPERMITTEDINANOPVPZONE;
			}
			if(target->getZone() == ZONE_NOPVP){
				return RET_YOUMAYNOTATTACKAPERSONINPROTECTIONZONE;
			}
		}
	}

	if(player->hasFlag(PlayerFlag_CannotUseCombat) || !target->isAttackable()){
		if(target->getPlayer()){
			return RET_YOUMAYNOTATTACKTHISPLAYER;
		}
		else
			return RET_YOUMAYNOTATTACKTHISCREATURE;
		
	}

	if (g_config.getBoolean(ConfigManager::TEAM_MODE)) {
		const Player* targetPlayer = target->getPlayer();
		if (player && targetPlayer &&
			player->getGuildId() == targetPlayer->getGuildId()) 
		{
			return RET_YOUMAYNOTATTACKTHISPLAYER;
		}
	}

#ifdef __SKULLSYSTEM__
	if(player->hasSafeMode() && target->getPlayer()) {
		if(player->getParty()) {
		    if(player->getParty()->isPlayerMember(target->getPlayer()) ||
				player->getParty()->getLeader() == target) 
			{
		        return Combat::canDoCombat(player, target);
			}
		}
		if(target->getPlayer()->getSkull() == SKULL_NONE) {
			if(!Combat::isInPvpZone(player, target)) {
				return RET_TURNSECUREMODETOATTACKUNMARKEDPLAYERS;
			}
		}
	}
#endif

	return Combat::canDoCombat(player, target);
}

ReturnValue Combat::canDoCombat(const Creature* caster, const Tile* tile, bool isAggressive)
{
	if(tile->hasProperty(BLOCKPROJECTILE)){
		return RET_NOTENOUGHROOM;
	}

	//if(!tile->ground){
	//	return RET_NOTPOSSIBLE;
	//}

	if(tile->floorChange()){
		return RET_NOTENOUGHROOM;
	}

	if(tile->getTeleportItem()){
		return RET_NOTENOUGHROOM;
	}

	if(caster){
		if(caster->getPosition().z < tile->getPosition().z){
			return RET_FIRSTGODOWNSTAIRS;
		}

		if(caster->getPosition().z > tile->getPosition().z){
			return RET_FIRSTGOUPSTAIRS;
		}

		if(const Player* player = caster->getPlayer()){
			if(player->hasFlag(PlayerFlag_IgnoreProtectionZone)){
				return RET_NOERROR;
			}
		}
	}

    //pz-zone
	if(isAggressive && tile->hasFlag(TILESTATE_PROTECTIONZONE)){
		return RET_ACTIONNOTPERMITTEDINPROTECTIONZONE;
	}

	return RET_NOERROR;
}

bool Combat::isInPvpZone(const Creature* attacker, const Creature* target)
{
	if(attacker->getZone() != ZONE_PVP){
		return false;
	}

	if(target->getZone() != ZONE_PVP){
		return false;
	}

	return true;
}

ReturnValue Combat::canDoCombat(const Creature* attacker, const Creature* target)
{
	if(attacker){
		if(const Player* targetPlayer = target->getPlayer()){
			if(targetPlayer->hasFlag(PlayerFlag_CannotBeAttacked)){
				return RET_YOUMAYNOTATTACKTHISPLAYER;
			}

			if(const Player* attackerPlayer = attacker->getPlayer()){
				if(attackerPlayer->hasFlag(PlayerFlag_CannotAttackPlayer)){
					return RET_YOUMAYNOTATTACKTHISPLAYER;
				}
			}

			if(attacker->isSummon()){
				if(const Player* masterAttackerPlayer = attacker->getMaster()->getPlayer()){
					if(masterAttackerPlayer->hasFlag(PlayerFlag_CannotAttackPlayer)){
						return RET_YOUMAYNOTATTACKTHISPLAYER;
					}
				}
			}
		}
		else if(target->getMonster()){
			if(const Player* attackerPlayer = attacker->getPlayer()){
				if(attackerPlayer->hasFlag(PlayerFlag_CannotAttackMonster)){
					return RET_YOUMAYNOTATTACKTHISCREATURE;
				}
			}
		}

		if(attacker->getPlayer() || (attacker->isSummon() && attacker->getMaster()->getPlayer()) ){
			//nopvp-zone
			if(target->getPlayer() && target->getTile()->hasFlag(TILESTATE_NOPVPZONE)){
				return RET_ACTIONNOTPERMITTEDINANOPVPZONE;
			}

			if(g_game.getWorldType() == WORLD_TYPE_NO_PVP){
				if(target->getPlayer()){
					if(!isInPvpZone(attacker, target)){
						return RET_YOUMAYNOTATTACKTHISPLAYER;
					}
				}

				if(target->isSummon() && target->getMaster()->getPlayer()){
					if(!isInPvpZone(attacker, target)){
						return RET_YOUMAYNOTATTACKTHISCREATURE;
					}
				}
			}
		}
	}

	return RET_NOERROR;
}

void Combat::setPlayerCombatValues(formulaType_t _type, double _mina, double _minb, double _maxa, double _maxb)
{
	formulaType = _type;
	mina = _mina;
	minb = _minb;
	maxa = _maxa;
	maxb = _maxb;
}



bool Combat::setCallback(CallBackParam_t key)
{
	switch(key){
		case CALLBACKPARAM_LEVELMAGICVALUE:
		{
			delete params.valueCallback;
			params.valueCallback = new ValueCallback(FORMULA_LEVELMAGIC);
			return true;
		}

		case CALLBACKPARAM_SKILLVALUE:
		{
			delete params.valueCallback;
			params.valueCallback = new ValueCallback(FORMULA_SKILL);
			return true;
		}

		case CALLBACKPARAM_TARGETTILECALLBACK:
		{
			delete params.tileCallback;
			params.tileCallback = new TileCallback();
			break;
		}

		case CALLBACKPARAM_TARGETCREATURECALLBACK:
		{
			delete params.targetCallback;
			params.targetCallback = new TargetCallback();
			break;
		}

		default:
		{
			std::cout << "Combat::setCallback - Unknown callback type: " << (uint32_t)key << std::endl;
			break;
		}
	}

	return false;
}

CallBack* Combat::getCallback(CallBackParam_t key)
{
	switch(key){
		case CALLBACKPARAM_LEVELMAGICVALUE:
		case CALLBACKPARAM_SKILLVALUE:
		{
			return params.valueCallback;
		}

		case CALLBACKPARAM_TARGETTILECALLBACK:
		{
			return params.tileCallback;
		}

		case CALLBACKPARAM_TARGETCREATURECALLBACK:
		{
			return params.targetCallback;
		}
	}

	return NULL;
}

bool Combat::CombatHealthFunc(Creature* caster, Creature* target, const CombatParams& params, void* data)
{
	Combat2Var* var = (Combat2Var*)data;
	int32_t healthChange = random_range(var->minChange, var->maxChange, DISTRO_NORMAL);

	if(g_game.combatBlockHit(params.combatType, caster, target, healthChange, params.blockedByShield, params.blockedByArmor))
	{
		return false;
	}

	//if(healthChange < 0)
	//{
	//	if (caster && target) 
	//	{
	//		Player* casterPlayer = caster->getPlayer();
	//		Player* targetPlayer = target->getPlayer();
	//		//player vs player
	//		if (casterPlayer && targetPlayer) 
	//		{
	//			healthChange = healthChange / 2;

	//			if (g_config.getBoolean(ConfigManager::TEAM_MODE) && 
	//				casterPlayer->getGuildId() == targetPlayer->getGuildId()) 
	//			{
	//				int damagePercent = g_config.getNumber(ConfigManager::DAMAGE_PERCENT);
	//				healthChange = (healthChange * damagePercent) / 100;
	//			}
	//		}
	//	}
	//}

	bool result = g_game.combatChangeHealth(params.combatType, params.hitEffect, params.hitTextColor, caster, target, healthChange);

	if(result)
	{
		CombatConditionFunc(caster, target, params, NULL);
		CombatDispelFunc(caster, target, params, NULL);
	}

	return result;
}


bool Combat::CombatPhysicalHealthFunc(Creature* caster, Creature* target, const CombatParams& params, void* data)
{
	struct _weaponDamage_* wDamage = (struct _weaponDamage_*)data;
	//the attack was failed or blocked?

	if (g_game.combatBlockPhysicalHit(params.combatType, caster, target, wDamage))
		return false;

	bool result = false;
	switch (wDamage->damageType)
	{
		//blood only
		case DAMAGE_MIN:
			result = g_game.combatChangeHealth(params.combatType, NM_ME_DRAW_BLOOD_MIN, params.hitTextColor, caster, target, wDamage);
			break;
		case DAMAGE_MEDIUM:
			result = g_game.combatChangeHealth(params.combatType, NM_ME_DRAW_BLOOD_MEDIUM, params.hitTextColor, caster, target,wDamage);
			break;
		case DAMAGE_MAX:
			result = g_game.combatChangeHealth(params.combatType, NM_ME_DRAW_BLOOD_MAX, params.hitTextColor, caster, target,wDamage);
			break;
		//blood + perforation min
		case (DAMAGE_MIN | DAMAGE_PERFORATION_MIN):
			result = g_game.combatChangeHealth(params.combatType, NM_ME_DRAW_BLOOD_PLUS_PERFORATION, params.hitTextColor, caster, target,wDamage);
			break;
		case (DAMAGE_MEDIUM | DAMAGE_PERFORATION_MIN):
			result = g_game.combatChangeHealth(params.combatType, NM_ME_DRAW_BLOOD_MEDIUM_PLUS_PERFORATION, params.hitTextColor, caster, target,wDamage);
			break;
		case (DAMAGE_MAX | DAMAGE_PERFORATION_MIN):
			result = g_game.combatChangeHealth(params.combatType, NM_ME_DRAW_BLOOD_MAX_PLUS_PERFORATION, params.hitTextColor, caster, target,wDamage);
			break;
		//blood + perforation medium
		case (DAMAGE_MIN | DAMAGE_PERFORATION_MED):
			result = g_game.combatChangeHealth(params.combatType, NM_ME_DRAW_BLOOD_MIN_PLUS_PERFORATION_MED, params.hitTextColor, caster, target,wDamage);
			break;
		case (DAMAGE_MEDIUM | DAMAGE_PERFORATION_MED):
			result = g_game.combatChangeHealth(params.combatType, NM_ME_DRAW_BLOOD_MED_PLUS_PERFORATION_MED, params.hitTextColor, caster, target,wDamage);
			break;
		case (DAMAGE_MAX | DAMAGE_PERFORATION_MED):
			result = g_game.combatChangeHealth(params.combatType, NM_ME_DRAW_BLOOD_MAX_PLUS_PERFORATION_MED, params.hitTextColor, caster, target,wDamage);
			break;
		//blood + perforation MAX
		case (DAMAGE_MIN | DAMAGE_PERFORATION_MAX):
			result = g_game.combatChangeHealth(params.combatType, NM_ME_DRAW_BLOOD_MIN_PLUS_PERFORATION_MAX, params.hitTextColor, caster, target,wDamage);			
			break;
		case (DAMAGE_MEDIUM | DAMAGE_PERFORATION_MAX):
			result = g_game.combatChangeHealth(params.combatType, NM_ME_DRAW_BLOOD_MED_PLUS_PERFORATION_MAX, params.hitTextColor, caster, target,wDamage);
			break;
		case (DAMAGE_MAX | DAMAGE_PERFORATION_MAX):
			result = g_game.combatChangeHealth(params.combatType, NM_ME_DRAW_BLOOD_MAX_PLUS_PERFORATION_MAX, params.hitTextColor, caster, target,wDamage);
			break;
		//blood + slash
		case (DAMAGE_MIN | DAMAGE_SLASH_MIN):
			result = g_game.combatChangeHealth(params.combatType, NM_ME_DRAW_BLOOD_PLUS_SLASH, params.hitTextColor, caster, target,wDamage);
			break;
		case (DAMAGE_MEDIUM | DAMAGE_SLASH_MIN):
			result = g_game.combatChangeHealth(params.combatType, NM_ME_DRAW_BLOOD_MEDIUM_PLUS_SLASH, params.hitTextColor, caster, target,wDamage);
			break;
		case (DAMAGE_MAX | DAMAGE_SLASH_MIN):
			result = g_game.combatChangeHealth(params.combatType, NM_ME_DRAW_BLOOD_MAX_PLUS_SLASH, params.hitTextColor, caster, target,wDamage);
			break;
		//blood + slash MED
		case (DAMAGE_MIN | DAMAGE_SLASH_MED):
			result = g_game.combatChangeHealth(params.combatType, NM_ME_DRAW_BLOOD_PLUS_SLASH_MED, params.hitTextColor, caster, target,wDamage);
			break;
		case (DAMAGE_MEDIUM | DAMAGE_SLASH_MED):
			result = g_game.combatChangeHealth(params.combatType, NM_ME_DRAW_BLOOD_MEDIUM_PLUS_SLASH_MED, params.hitTextColor, caster, target,wDamage);
			break;
		case (DAMAGE_MAX | DAMAGE_SLASH_MED):
			result = g_game.combatChangeHealth(params.combatType, NM_ME_DRAW_BLOOD_PLUS_SLASH_MED, params.hitTextColor, caster, target,wDamage);
			break;
		//blood + slash MAX
		case (DAMAGE_MIN | DAMAGE_SLASH_MAX):
			result = g_game.combatChangeHealth(params.combatType, NM_ME_DRAW_BLOOD_PLUS_SLASH_MAX, params.hitTextColor, caster, target,wDamage);
			break;
		case (DAMAGE_MEDIUM | DAMAGE_SLASH_MAX):
			result = g_game.combatChangeHealth(params.combatType, NM_ME_DRAW_BLOOD_MEDIUM_PLUS_SLASH_MAX, params.hitTextColor, caster, target,wDamage);
			break;
		case (DAMAGE_MAX | DAMAGE_SLASH_MAX):
			result = g_game.combatChangeHealth(params.combatType, NM_ME_DRAW_BLOOD_PLUS_SLASH_MAX, params.hitTextColor, caster, target,wDamage);
			break;
	}

	if (result)
	{
		CombatConditionFunc(caster, target, params, NULL);
		CombatDispelFunc(caster, target, params, NULL);
	}

	return result;
}

bool Combat::CombatManaFunc(Creature* caster, Creature* target, const CombatParams& params, void* data)
{
	Combat2Var* var = (Combat2Var*)data;
	int32_t manaChange = random_range(var->minChange, var->maxChange, DISTRO_NORMAL);

	if(manaChange < 0){
		if (caster && target) {
			Player* casterPlayer = caster->getPlayer();
			Player* targetPlayer = target->getPlayer();
			if (casterPlayer && targetPlayer) {
				manaChange = manaChange / 2;

				if (g_config.getBoolean(ConfigManager::TEAM_MODE) && 
					casterPlayer->getGuildId() == targetPlayer->getGuildId()) 
				{
					int damagePercent = g_config.getNumber(ConfigManager::DAMAGE_PERCENT);
					manaChange = (manaChange * damagePercent) / 100;
				}
			}
		}
	}

	bool result = g_game.combatChangeMana(caster, target, manaChange);

	if(result){
		CombatConditionFunc(caster, target, params, NULL);
		CombatDispelFunc(caster, target, params, NULL);
	}

	return result;
}

bool Combat::CombatConditionFunc(Creature* caster, Creature* target, const CombatParams& params, void* data)
{
	bool result = false;

	if(params.condition){
		if(caster == target || !target->isImmune(params.condition->getType())){
			Condition* conditionCopy = params.condition->clone();
			if(caster){
				conditionCopy->setParam(CONDITIONPARAM_OWNER, caster->getID());
			}

			//TODO: infight condition until all aggressive conditions has ended
			result = target->addCombatCondition(conditionCopy);
		}
	}

	return result;
}

bool Combat::CombatDispelFunc(Creature* caster, Creature* target, const CombatParams& params, void* data)
{
	if(target->hasCondition(params.dispelType)){
		target->removeCondition(caster, params.dispelType);
		return true;
	}

	return false;
}

bool Combat::CombatNullFunc(Creature* caster, Creature* target, const CombatParams& params, void* data)
{
	CombatConditionFunc(caster, target, params, NULL);
	CombatDispelFunc(caster, target, params, NULL);
	return true;
}

void Combat::combatTileEffects(const SpectatorVec& list, Creature* caster, Tile* tile, const CombatParams& params)
{
	if(params.itemId != 0){
		uint32_t itemId = params.itemId;
		Player* p_caster = NULL;
		if(caster){
			if(caster->getPlayer()){
				p_caster = caster->getPlayer();
			} 
			else if(caster->isSummon()){
				p_caster = caster->getMaster()->getPlayer();
			}
		}
		if(p_caster){
			if(g_game.getWorldType() == WORLD_TYPE_NO_PVP || tile->hasFlag(TILESTATE_NOPVPZONE)){
				if(itemId == ITEM_FIREFIELD_PVP){
					itemId = ITEM_FIREFIELD_NOPVP;
				}
				else if(itemId == ITEM_POISONFIELD_PVP){
					itemId = ITEM_POISONFIELD_NOPVP;
				}
				else if(itemId == ITEM_ENERGYFIELD_PVP){
					itemId = ITEM_ENERGYFIELD_NOPVP;
				}
			}
			else if(params.isAggressive){
				const ItemType& it = Item::items[itemId];
				if(!it.blockSolid){
					p_caster->addInFightTicks(true);
				}
				else{
					p_caster->addInFightTicks();
				}
			}
		}

		Item* item = Item::CreateItem(itemId);

		if(caster){
			item->setOwner(caster->getID());
		}

		ReturnValue ret = g_game.internalAddItem(tile, item);
		if(ret == RET_NOERROR){
			g_game.startDecay(item);
		}
		else{
			delete item;
		}
	}

	if(params.tileCallback){
		params.tileCallback->onTileCombat(caster, tile);
	}

	if(params.impactEffect != NM_ME_NONE){
		g_game.addMagicEffect(list, tile->getPosition(), params.impactEffect);
	}
}

void Combat::postCombatEffects(Creature* caster, const Position& pos, const CombatParams& params)
{
	if(caster && params.distanceEffect != NM_ME_NONE){
		addDistanceEffect(caster, caster->getPosition(), pos, params.distanceEffect);
	}
}

void Combat::addDistanceEffect(Creature* caster, const Position& fromPos, const Position& toPos,
	uint8_t effect)
{
	uint8_t distanceEffect = effect;

	if(caster && distanceEffect != NM_ME_NONE){
		g_game.addDistanceEffect(fromPos, toPos, distanceEffect);
	}
}


void Combat::CombatFunc(Creature* caster, const Position& pos,
	const AreaCombat* area, const CombatParams& params, COMBATFUNC func, void* data)
{
	std::list<std::pair<Tile*,uint8_t>> tileList;

	if (caster){
		getCombatArea(caster->getPosition(), pos, area, tileList);
	}
	else{
		getCombatArea(pos, pos, area, tileList);
	}

	SpectatorVec list;
	uint32_t maxX = 0;
	uint32_t maxY = 0;
	uint32_t diff;

	//calculate the max viewable range
	for (auto it = tileList.begin(); it != tileList.end(); ++it){
		diff = std::abs((*it).first->getPosition().x - pos.x);
		if (diff > maxX){
			maxX = diff;
		}

		diff = std::abs((*it).first->getPosition().y - pos.y);
		if (diff > maxY){
			maxY = diff;
		}
	}

	g_game.getSpectators(list, pos, false, true, maxX + Map::maxViewportX, maxX + Map::maxViewportX,
		maxY + Map::maxViewportY, maxY + Map::maxViewportY);

	for (auto it = tileList.begin(); it != tileList.end(); ++it){
		bool bContinue = true;

		if (canDoCombat(caster, (*it).first, params.isAggressive) == RET_NOERROR){
			for (CreatureVector::iterator cit = (*it).first->creatures.begin(); bContinue && cit != (*it).first->creatures.end(); ++cit){
				if (params.targetCasterOrTopMost){

					if (g_config.getBoolean(ConfigManager::UH_TRAP) == false &&
						(caster && caster->getTile() == (*it).first)){
						if (*cit == caster){
							bContinue = false;
						}
					}
					else if (*cit == (*it).first->getTopCreature()){
						bContinue = false;
					}

					if (bContinue){
						continue;
					}
				}

				if (!params.isAggressive || (caster != *cit && Combat::canDoCombat(caster, *cit) == RET_NOERROR)){
					func(caster, *cit, params, data);

					if (params.targetCallback){
						params.targetCallback->onTargetCombat(caster, *cit);
					}
				}
			}

			combatTileEffects(list, caster, (*it).first, params);
		}
	}

	postCombatEffects(caster, pos, params);
}



void Combat::doCombat(Creature* caster, Creature* target) const
{
	//target combat callback function

	if(params.combatType != COMBAT_NONE)
	{
		int32_t minChange = 0;
		int32_t maxChange = 0;
		getMinMaxValues(caster, target, minChange, maxChange);

		if(params.combatType != COMBAT_MANADRAIN)
			doCombatHealth(caster, target, minChange, maxChange, params);
		else
			doCombatMana(caster, target, minChange, maxChange, params);
	}
	else
		doCombatDefault(caster, target, params);
}

bool Combat::setParam(CombatParam_t param, float value)
{
	switch (param){
		case COMBATPARAM_COMBATTYPE:
		{
			params.combatType = (CombatType_t)(int)value;
			return true;
		}

		case COMBATPARAM_EFFECT:
		{
			params.impactEffect = (uint8_t)value;
			return true;
		}

		case COMBATPARAM_DISTANCEEFFECT:
		{
			params.distanceEffect = (uint8_t)value;
			return true;
		}

		case COMBATPARAM_BLOCKEDBYARMOR:
		{
			params.blockedByArmor = ((int)value != 0);
			return true;
		}

		case COMBATPARAM_BLOCKEDBYSHIELD:
		{
			params.blockedByShield = ((int)value != 0);
			return true;
		}

		case COMBATPARAM_TARGETCASTERORTOPMOST:
		{
			params.targetCasterOrTopMost = ((int)value != 0);
			return true;
		}

		case COMBATPARAM_CREATEITEM:
		{
			params.itemId = value;
			return true;
		}

		case COMBATPARAM_AGGRESSIVE:
		{
			params.isAggressive = ((bool)value != 0);
			return true;
		}

		case COMBATPARAM_DISPEL:
		{
			params.dispelType = (ConditionType_t)(int)value;
			return true;
		}

		case COMBATPARAM_USECHARGES:
		{
			params.useCharges = (value != 0);
			return true;
		}

		case COMBATPARAM_HITEFFECT:
		{
			params.hitEffect = (MagicEffectClasses)(int)value;
			return true;
		}

		case COMBATPARAM_HITTEXTCOLOR:
		{
			params.hitTextColor = (TextColor_t)(int)value;
			return true;
		}

		case COMBATPARAM_MAGIC_INTERVALS:
		{
			params.magicIntervals = value;
			return true;
		}

		case COMBATPARAM_COORDINATE_SYSTEM:
		{
			params.coordinateSystem = (uint8_t)value;
			return true;
		}

		case COMBATPARAM_DRAW_FUNCTION:
		{
			params.drawFunction = (uint8_t)value;
			return true;
		}

		case COMBATPARAM_COEFFICIENT_A:
		{
			params.coefficientA = value;
			return true;
		}

		case COMBATPARAM_COEFFICIENT_B:
		{
			params.coefficientB = value;
			return true;
		}

		case COMBATPARAM_THETA:
		{
			params.tetha = value;
			return true;
		}
		
		case COMBATPARAM_RANDOMGROWTH:
		{
			//true or false
			params.randomGrowth = (uint32_t)value;
			return true;
		}

		case COMBATPARAM_ONLYDISTANCE:
		{
			//true or false
			params.onlyDistance = (bool)value;
			return true;
		}

		default:
		{
			break;
		}
	}

	return false;
}

bool listPred(std::pair<Tile *, uint8_t> & a, std::pair<Tile *, uint8_t> &b)
{
	return (a).second < (b).second;
}

void Combat::doCombat(Creature* caster, const Position& pos)
{
	//area combat callback function
	if (params.combatType != COMBAT_NONE)
	{
		int32_t minChange = 0;
		int32_t maxChange = 0;
		getMinMaxValues(caster, NULL, minChange, maxChange);

		if (params.combatType != COMBAT_MANADRAIN)
		{

		}
			
		//check coordinateSystem
		if (params.coordinateSystem == 1) // polar coordinate 
		{
			AreaCombat * areaCombat = new AreaCombat();
			areaCombat->setupArea(1);
			
			Position oldPosition(0,0,0);
			Position effectPosition;
			for (float theta = 0; theta <= params.tetha; theta += 1)
			{
				switch (params.drawFunction)
				{
					case 0: //arquimedes spiral
						//polarFormula("arquimedesSpiral", theta * 3.141516 / 180.0, params.coefficientA, params.coefficientB, &x, &y);
					break;			
				}
				effectPosition = Position((float)caster->getPosition().x + params.coefficientA * std::sin((4 * theta * 3.14159265359) / 180.0) *std::cos((theta * 3.14159265359) / 180.0),
					(float)caster->getPosition().y + params.coefficientA * std::sin((4 *theta * 3.14159265359) / 180.0) *std::sin((theta * 3.14159265359) / 180.0)
					, 0 + caster->getPosition().z);
				if ((int)oldPosition.x != (int)effectPosition.x || (int)oldPosition.y != (int)effectPosition.y)
				{
					uint8_t dir = caster->getDirection();
					Position distanceMagic;
					
					Scheduler::getScheduler().addEvent(createSchedulerTask(20 + theta * params.magicIntervals,
						boost::bind(&Combat::doCombatHealth, caster,
						effectPosition,
						areaCombat, minChange, maxChange, params)));
					oldPosition = effectPosition;
				}
			}
		}
		else if (params.coordinateSystem == 2) // cartesian 
		{						
			std::list<std::pair<Tile *, uint8_t>> tileList;
			area->getList(caster->getPosition(), pos, tileList);
			tileList.sort(listPred);

			uint8_t maxTime = tileList.end()->second;
			uint8_t distanceEffect;
			while (!tileList.empty())
			{
				//random growth spell
				if (params.randomGrowth)
				{
					AreaCombat * areaCombat = new AreaCombat();
					areaCombat->setupArea(1);
					int randomNumber = std::rand() % 100;
					if (tileList.begin()->second == 1)
					{
						//create the first spell
						doCombatHealth(caster,
							tileList.begin()->first->getPosition(),
							areaCombat,
							minChange, maxChange,
							params);
						//remove the distance effect from others
						params.distanceEffect = NM_ME_NONE;
					}
					else
					{
						AreaCombat * areaCombat = new AreaCombat();
						areaCombat->setupArea(1);
						//fire spell -- stoker
						if (params.itemId == 595)
						{
							float escale[3] = { 0.7, 0.2, 0.1 };//fire scales 
							if (tileList.begin()->second >= 80)//hight percentage lets burn with intenssive fire (80%)
								escale[0] = 0.7, escale[1] = 0.2, escale[2] = 0.1;
							else if (tileList.begin()->second >= 60)
								escale[0] = 0.60, escale[1] = 0.25, escale[2] = 0.15;
							else if (tileList.begin()->second >= 40)
								escale[0] = 0.40, escale[1] = 0.40, escale[2] = 0.20;
							else if (tileList.begin()->second >= 20)
								escale[0] = 0.20, escale[1] = 0.50, escale[2] = 0.30;
							else 
								escale[0] = 0.05, escale[1] = 0.45, escale[2] = 0.70;


							if (randomNumber <= tileList.begin()->second * escale[0])
								params.itemId = 595;
							else if (randomNumber <= tileList.begin()->second * escale[1])
								params.itemId = 517;
							else
								params.itemId = 596;
						}

						Scheduler::getScheduler().addEvent(createSchedulerTask(20 + params.magicIntervals * (100 - tileList.begin()->second),
							boost::bind(&Combat::doCombatHealth, caster,
							tileList.begin()->first->getPosition(),
							areaCombat,
							minChange, maxChange,
							params)));

						//restore fire item id
						params.itemId = 595;
					}
				}
				else
				{						
					AreaCombat * areaCombat = new AreaCombat();
					areaCombat->setupArea(1);
					Scheduler::getScheduler().addEvent(createSchedulerTask(20 + params.magicIntervals * tileList.begin()->second,
						boost::bind(&Combat::doCombatHealth, caster,
						tileList.begin()->first->getPosition(),
						areaCombat,
						minChange, maxChange,
						params)));
				}					
				tileList.pop_front();
			}
		

		}
		
	}
	else
		CombatFunc(caster, pos, area, params, CombatNullFunc, NULL);
}

void Combat::doCombatHealth(Creature* caster, Creature* target,
	int32_t minChange, int32_t maxChange, const CombatParams& params)
{
	if(!params.isAggressive || (caster != target && Combat::canDoCombat(caster, target) == RET_NOERROR))
	{
		Combat2Var var;
		var.minChange = minChange;
		var.maxChange = maxChange;
		CombatHealthFunc(caster, target, params, (void*)&var);

		if(params.impactEffect != NM_ME_NONE)
			g_game.addMagicEffect(target->getPosition(), params.impactEffect);

		if(caster && params.distanceEffect != NM_ME_NONE)
			addDistanceEffect(caster, caster->getPosition(), target->getPosition(), params.distanceEffect);
	}
}

void Combat::doCombatHealth(Creature* caster, Creature* target, struct _weaponDamage_ *wd, const CombatParams& params)
{
	if (!params.isAggressive || (caster != target && Combat::canDoCombat(caster, target) == RET_NOERROR))
	{

		CombatPhysicalHealthFunc(caster, target, params, (struct _weaponDamage_*)wd);

		if (params.impactEffect != NM_ME_NONE)
			g_game.addMagicEffect(target->getPosition(), params.impactEffect);

		if (caster && params.distanceEffect != NM_ME_NONE)
			addDistanceEffect(caster, caster->getPosition(), target->getPosition(), params.distanceEffect);
	}
}

void Combat::doCombatHealth(Creature* caster, const Position& pos,
	const AreaCombat* area, int32_t minChange, int32_t maxChange, const CombatParams& params)
{
	Combat2Var var;
	var.minChange = minChange;
	var.maxChange = maxChange;

	CombatFunc(caster, pos, area, params, CombatHealthFunc, (void*)&var);
}

void Combat::doCombatMana(Creature* caster, Creature* target,
	int32_t minChange, int32_t maxChange, const CombatParams& params)
{
	if(!params.isAggressive || (caster != target && Combat::canDoCombat(caster, target) == RET_NOERROR)){
		Combat2Var var;
		var.minChange = minChange;
		var.maxChange = maxChange;
		CombatManaFunc(caster, target, params, (void*)&var);

		if(params.targetCallback){
			params.targetCallback->onTargetCombat(caster, target);
		}

		if(params.impactEffect != NM_ME_NONE){
			g_game.addMagicEffect(target->getPosition(), params.impactEffect);
		}

		if(caster && params.distanceEffect != NM_ME_NONE){
			addDistanceEffect(caster, caster->getPosition(), target->getPosition(), params.distanceEffect);
		}
	}
}

void Combat::doCombatMana(Creature* caster, const Position& pos,
	const AreaCombat* area, int32_t minChange, int32_t maxChange, const CombatParams& params)
{
	Combat2Var var;
	var.minChange = minChange;
	var.maxChange = maxChange;

	CombatFunc(caster, pos, area, params, CombatManaFunc, (void*)&var);
}

void Combat::doCombatCondition(Creature* caster, const Position& pos, const AreaCombat* area,
	const CombatParams& params)
{
	CombatFunc(caster, pos, area, params, CombatConditionFunc, NULL);
}

void Combat::doCombatCondition(Creature* caster, Creature* target, const CombatParams& params)
{
	if(!params.isAggressive || (caster != target && Combat::canDoCombat(caster, target) == RET_NOERROR)){
		CombatConditionFunc(caster, target, params, NULL);

		if(params.targetCallback){
			params.targetCallback->onTargetCombat(caster, target);
		}

		if(params.impactEffect != NM_ME_NONE){
			g_game.addMagicEffect(target->getPosition(), params.impactEffect);
		}

		if(caster && params.distanceEffect != NM_ME_NONE){
			addDistanceEffect(caster, caster->getPosition(), target->getPosition(), params.distanceEffect);
		}
	}
}

void Combat::doCombatDispel(Creature* caster, const Position& pos, const AreaCombat* area,
	const CombatParams& params)
{
	CombatFunc(caster, pos, area, params, CombatDispelFunc, NULL);
}

void Combat::doCombatDispel(Creature* caster, Creature* target, const CombatParams& params)
{
	if(!params.isAggressive || (caster != target && Combat::canDoCombat(caster, target) == RET_NOERROR)){
		CombatDispelFunc(caster, target, params, NULL);

		if(params.targetCallback){
			params.targetCallback->onTargetCombat(caster, target);
		}

		if(params.impactEffect != NM_ME_NONE){
			g_game.addMagicEffect(target->getPosition(), params.impactEffect);
		}

		if(caster && params.distanceEffect != NM_ME_NONE){
			addDistanceEffect(caster, caster->getPosition(), target->getPosition(), params.distanceEffect);
		}
	}
}

void Combat::doCombatDefault(Creature* caster, Creature* target, const CombatParams& params)
{
	if(!params.isAggressive || (caster != target && Combat::canDoCombat(caster, target) == RET_NOERROR)){
        const SpectatorVec& list = g_game.getSpectators(target->getTile()->getPosition());
		CombatNullFunc(caster, target, params, NULL);
		combatTileEffects(list, caster, target->getTile(), params);

		if(params.targetCallback){
			params.targetCallback->onTargetCombat(caster, target);
		}

		if(params.impactEffect != NM_ME_NONE){
			g_game.addMagicEffect(target->getPosition(), params.impactEffect);
		}

		if(caster && params.distanceEffect != NM_ME_NONE){
			addDistanceEffect(caster, caster->getPosition(), target->getPosition(), params.distanceEffect);
		}
	}
}

//**********************************************************

void ValueCallback::getMinMaxValues(Player* player, int32_t& min, int32_t& max, bool useCharges) const
{
	//"onGetPlayerMinMaxValues"(...)
	if(m_scriptInterface->reserveScriptEnv()){
		ScriptEnviroment* env = m_scriptInterface->getScriptEnv();
		lua_State* L = m_scriptInterface->getLuaState();

		if(!env->setCallbackId(m_scriptId, m_scriptInterface))
			return;

		uint32_t cid = env->addThing(player);

		m_scriptInterface->pushFunction(m_scriptId);
		lua_pushnumber(L, cid);

		int32_t parameters = 1;

		switch(type){
			case FORMULA_LEVELMAGIC:
			{
				//"onGetPlayerMinMaxValues"(cid, level, maglevel)
				lua_pushnumber(L, player->getLevel());
				lua_pushnumber(L, player->getMagicLevel());
				parameters += 2;
				break;
			}

			case FORMULA_SKILL:
			{
				//"onGetPlayerMinMaxValues"(cid, attackSkill, attackValue, attackFactor)
				Item* tool = player->getWeapon();
				int32_t attackSkill = player->getWeaponSkill(tool);
				int32_t attackValue = 7;
				if(tool){
					//attackValue = tool->getAttack();

					if(useCharges && tool->hasCharges() && g_config.getNumber(ConfigManager::REMOVE_WEAPON_CHARGES)){
						int32_t newCharge = std::max(0, tool->getCharges() - 1);
						g_game.transformItem(tool, tool->getID(), newCharge);
					}
                }
				float attackFactor = player->getAttackFactor();

				lua_pushnumber(L, attackSkill);
				lua_pushnumber(L, attackValue);
				lua_pushnumber(L, attackFactor);
				parameters += 3;
				break;
			}

			default:
				std::cout << "ValueCallback::getMinMaxValues - unknown callback type" << std::endl;
				return;
				break;
		}

		int size0 = lua_gettop(L);
		if(lua_pcall(L, parameters, 2 /*nReturnValues*/, 0) != 0){
			LuaScriptInterface::reportError(NULL, std::string(LuaScriptInterface::popString(L)));
		}
		else{
			max = LuaScriptInterface::popNumber(L);
			min = LuaScriptInterface::popNumber(L);
		}

		if((lua_gettop(L) + parameters /*nParams*/  + 1) != size0){
			LuaScriptInterface::reportError(NULL, "Stack size changed!");
		}

		env->resetCallback();
		m_scriptInterface->releaseScriptEnv();
	}
	else{
		std::cout << "[Error] Call stack overflow. ValueCallback::getMinMaxValues" << std::endl;
		return;
	}
}

//**********************************************************

void TileCallback::onTileCombat(Creature* creature, Tile* tile) const
{
	//"onTileCombat"(cid, pos)
	if(m_scriptInterface->reserveScriptEnv()){
		ScriptEnviroment* env = m_scriptInterface->getScriptEnv();
		lua_State* L = m_scriptInterface->getLuaState();

		if(!env->setCallbackId(m_scriptId, m_scriptInterface))
			return;

		uint32_t cid = 0;

		if(creature){
			cid = env->addThing(creature);
		}

		m_scriptInterface->pushFunction(m_scriptId);
		lua_pushnumber(L, cid);
		m_scriptInterface->pushPosition(L, tile->getPosition(), 0);

		m_scriptInterface->callFunction(2);

		env->resetCallback();
		m_scriptInterface->releaseScriptEnv();
	}
	else{
		std::cout << "[Error] Call stack overflow. TileCallback::onTileCombat" << std::endl;
		return;
	}
}

//**********************************************************

void TargetCallback::onTargetCombat(Creature* creature, Creature* target) const
{
	//"onTargetCombat"(cid, target)
	if(m_scriptInterface->reserveScriptEnv()){
		ScriptEnviroment* env = m_scriptInterface->getScriptEnv();
		lua_State* L = m_scriptInterface->getLuaState();

		if(!env->setCallbackId(m_scriptId, m_scriptInterface))
			return;

		uint32_t cid = 0;

		if(creature){
			cid = env->addThing(creature);
		}

		uint32_t targetCid = env->addThing(target);

		m_scriptInterface->pushFunction(m_scriptId);
		lua_pushnumber(L, cid);
		lua_pushnumber(L, targetCid);

		int size0 = lua_gettop(L);
		if(lua_pcall(L, 2, 0 /*nReturnValues*/, 0) != 0){
			LuaScriptInterface::reportError(NULL, std::string(LuaScriptInterface::popString(L)));
		}

		if((lua_gettop(L) + 2 /*nParams*/  + 1) != size0){
			LuaScriptInterface::reportError(NULL, "Stack size changed!");
		}

		env->resetCallback();
		m_scriptInterface->releaseScriptEnv();
	}
	else{
		std::cout << "[Error] Call stack overflow. TargetCallback::onTargetCombat" << std::endl;
		return;
	}
}

//**********************************************************

void AreaCombat::clear()
{
	for(AreaCombatMap::iterator it = areas.begin(); it != areas.end(); ++it){
		delete it->second;
	}

	areas.clear();
}

AreaCombat::AreaCombat(const AreaCombat& rhs)
{
    hasExtArea = rhs.hasExtArea;
    
    for(AreaCombatMap::const_iterator it = rhs.areas.begin(); it != rhs.areas.end(); ++it){
        areas[it->first] = new MatrixArea(*it->second);
    }
}

bool AreaCombat::getList(const Position& centerPos, const Position& targetPos, std::list<std::pair<Tile*,uint8_t>>& list) const
{
	Tile* tile = g_game.getTile(targetPos.x, targetPos.y, targetPos.z);

	const MatrixArea* area = getArea(centerPos, targetPos);
	if(!area){
		return false;
	}

	Position tmpPos = targetPos;

	size_t cols = area->getCols();
	size_t rows = area->getRows();

	uint32_t centerY, centerX;
	area->getCenter(centerY, centerX);

	tmpPos.x -= centerX;
	tmpPos.y -= centerY;

	for(size_t y = 0; y < rows; ++y){
		for(size_t x = 0; x < cols; ++x){

			if(area->getValue(y, x) != 0){
				if(tmpPos.x >= 0 && tmpPos.y >= 0 && tmpPos.z >= 0 &&
					tmpPos.x <= 0xFFFF && tmpPos.y <= 0xFFFF && tmpPos.z < MAP_MAX_LAYERS)
				{
					if(g_game.isSightClear(targetPos, tmpPos, true)){
						tile = g_game.getTile(tmpPos.x, tmpPos.y, tmpPos.z);
						if(!tile){
							tile = new Tile(tmpPos.x, tmpPos.y, tmpPos.z);
							g_game.setTile(tile);
						}
						list.push_back(std::make_pair(tile, area->getValue(y, x)));
					}
				}
			}

			tmpPos.x += 1;
		}

		tmpPos.x -= cols;
		tmpPos.y += 1;
	}

	return true;
}

int32_t round_(float v)
{
	int32_t t = (int32_t)std::floor(v);
	if((v - t) > 0.5){
		return t + 1;
	}
	else{
		return t;
	}
}

void AreaCombat::copyArea(const MatrixArea* input, MatrixArea* output, MatrixOperation_t op) const
{
	uint32_t centerY, centerX;
	input->getCenter(centerY, centerX);

	if(op == MATRIXOPERATION_COPY){
		for(unsigned int y = 0; y < input->getRows(); ++y){
			for(unsigned int x = 0; x < input->getCols(); ++x){
				(*output)[y][x] = (*input)[y][x];
			}
		}

		output->setCenter(centerY, centerX);
	}
	else if(op == MATRIXOPERATION_MIRROR){
		for(unsigned int y = 0; y < input->getRows(); ++y){
			int rx = 0;
			for(int x = input->getCols() - 1; x >= 0; --x){
				(*output)[y][rx++] = (*input)[y][x];
			}
		}

		output->setCenter(centerY, (input->getRows() - 1) - centerX);
	}
	else if(op == MATRIXOPERATION_FLIP){
		for(unsigned int x = 0; x < input->getCols(); ++x){
			int ry = 0;
			for(int y = input->getRows() - 1; y >= 0; --y){
				(*output)[ry++][x] = (*input)[y][x];
			}
		}

		output->setCenter((input->getCols() - 1) - centerY, centerX);
	}
	//rotation
	else{
		uint32_t centerX, centerY;
		input->getCenter(centerY, centerX);

		int32_t rotateCenterX = (output->getCols() / 2) - 1;
		int32_t rotateCenterY = (output->getRows() / 2) - 1;
		int32_t angle = 0;

		switch(op){
			case MATRIXOPERATION_ROTATE90:
				angle = 90;
				break;

			case MATRIXOPERATION_ROTATE180:
				angle = 180;
				break;

			case MATRIXOPERATION_ROTATE270:
				angle = 270;
				break;

			default:
				angle = 0;
				break;
		}
		double angleRad = 3.1416 * angle / 180.0;

		float a = std::cos(angleRad);
		float b = -std::sin(angleRad);
		float c = std::sin(angleRad);
		float d = std::cos(angleRad);

		for(int32_t x = 0; x < (int32_t)input->getCols(); ++x){
			for(int32_t y = 0; y < (int32_t)input->getRows(); ++y){
				//calculate new coordinates using rotation center
				int32_t newX = x - centerX;
				int32_t newY = y - centerY;

				//perform rotation
				int32_t rotatedX = round_(newX * a + newY * b);
				int32_t rotatedY = round_(newX * c + newY * d);

				//write in the output matrix using rotated coordinates
				(*output)[rotatedY + rotateCenterY][rotatedX + rotateCenterX] = (*input)[y][x];
			}
		}

		output->setCenter(rotateCenterY, rotateCenterX);
	}
}

MatrixArea* AreaCombat::createArea(const std::list<uint32_t>& list, uint32_t rows)
{
	unsigned int cols = list.size() / rows;
	MatrixArea* area = new MatrixArea(rows, cols);

	uint32_t x = 0;
	uint32_t y = 0;

	for(std::list<uint32_t>::const_iterator it = list.begin(); it != list.end(); ++it)
	{
		area->setValue(y, x, *it);
		
		if (*it == 1)		
			area->setCenter(y, x);


		++x;

		if(cols == x){
			x = 0;
			++y;
		}
	}

	return area;
}

void AreaCombat::setupArea(const std::list<uint32_t>& list, uint32_t rows)
{
	MatrixArea* area = createArea(list, rows);

	//NORTH
	areas[NORTH] = area;

	uint32_t maxOutput = std::max(area->getCols(), area->getRows()) * 2;

	//SOUTH
	MatrixArea* southArea = new MatrixArea(maxOutput, maxOutput);
	copyArea(area, southArea, MATRIXOPERATION_ROTATE180);
	areas[SOUTH] = southArea;

	//EAST
	MatrixArea* eastArea = new MatrixArea(maxOutput, maxOutput);
	copyArea(area, eastArea, MATRIXOPERATION_ROTATE90);
	areas[EAST] = eastArea;

	//WEST
	MatrixArea* westArea = new MatrixArea(maxOutput, maxOutput);
	copyArea(area, westArea, MATRIXOPERATION_ROTATE270);
	areas[WEST] = westArea;
}

void AreaCombat::setupArea(int32_t length, int32_t spread)
{
	std::list<uint32_t> list;

	uint32_t rows = length;
	int32_t cols = 1;

	if(spread != 0){
		cols = ((length - length % spread) / spread) * 2 + 1;
	}

	int32_t colSpread = cols;

	for(uint32_t y = 1; y <= rows; ++y){
		int32_t mincol = cols - colSpread + 1;
		int32_t maxcol = cols - (cols - colSpread);
		for(int32_t x = 1; x <= cols; ++x){
			if(y == rows && x == ((cols - cols % 2) / 2) + 1){
				list.push_back(3);
			}
			else if(x >= mincol && x <= maxcol){
				list.push_back(1);
			}
			else{
				list.push_back(0);
			}
		}

		if(spread > 0 && y % spread == 0){
			--colSpread;
		}
	}

	setupArea(list, rows);
}

void AreaCombat::setupArea(int32_t radius)
{
	int32_t area[13][13] = {
		{0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 8, 8, 7, 8, 8, 0, 0, 0, 0},
		{0, 0, 0, 8, 7, 6, 6, 6, 7, 8, 0, 0, 0},
		{0, 0, 8, 7, 6, 5, 5, 5, 6, 7, 8, 0, 0},
		{0, 8, 7, 6, 5, 4, 4, 4, 5, 6, 7, 8, 0},
		{0, 8, 6, 5, 4, 3, 2, 3, 4, 5, 6, 8, 0},
		{8, 7, 6, 5, 4, 2, 1, 2, 4, 5, 6, 7, 8},
		{0, 8, 6, 5, 4, 3, 2, 3, 4, 5, 6, 8, 0},
		{0, 8, 7, 6, 5, 4, 4, 4, 5, 6, 7, 8, 0},
		{0, 0, 8, 7, 6, 5, 5, 5, 6, 7, 8, 0, 0},
		{0, 0, 0, 8, 7, 6, 6, 6, 7, 8, 0, 0, 0},
		{0, 0, 0, 0, 8, 8, 7, 8, 8, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0}
	};

	std::list<uint32_t> list;

	for(int32_t y = 0; y < 13; ++y){
		for(int32_t x = 0; x < 13; ++x){
			if(area[y][x] == 1){
				list.push_back(1);
			}
			else if(area[y][x] > 0 && area[y][x] <= radius){
				list.push_back(2);
			}
			else{
				list.push_back(0);
			}
		}
	}

	setupArea(list, 13);
}

void AreaCombat::setupExtArea(const std::list<uint32_t>& list, uint32_t rows)
{
	if(list.empty()){
		return;
	}

	hasExtArea = true;
	MatrixArea* area = createArea(list, rows);

	//NORTH-WEST
	areas[NORTHWEST] = area;

	uint32_t maxOutput = std::max(area->getCols(), area->getRows()) * 2;

	//NORTH-EAST
	MatrixArea* neArea = new MatrixArea(maxOutput, maxOutput);
	copyArea(area, neArea, MATRIXOPERATION_MIRROR);
	areas[NORTHEAST] = neArea;

	//SOUTH-WEST
	MatrixArea* swArea = new MatrixArea(maxOutput, maxOutput);
	copyArea(area, swArea, MATRIXOPERATION_FLIP);
	areas[SOUTHWEST] = swArea;

	//SOUTH-EAST
	MatrixArea* seArea = new MatrixArea(maxOutput, maxOutput);
	copyArea(swArea, seArea, MATRIXOPERATION_MIRROR);
	areas[SOUTHEAST] = seArea;
}

//**********************************************************

void MagicField::onStepInField(Creature* creature, bool purposeful/*= true*/)
{
	//remove magic walls/wild growth
	if(isBlocking()){
		g_game.internalRemoveItem(this, 1);
	}
	else{
		const ItemType& it = items[getID()];
		if(it.condition){
			Condition* conditionCopy = it.condition->clone();
			uint32_t owner = getOwner();
			if(owner != 0 && purposeful){
				bool harmfulField = true;
				if(g_game.getWorldType() == WORLD_TYPE_NO_PVP || getTile()->hasFlag(TILESTATE_NOPVPZONE) ){
					Creature* creature = g_game.getCreatureByID(owner);
					if(creature){
						if(creature->getPlayer() || (creature->isSummon() && creature->getMaster()->getPlayer())){
							harmfulField = false;
						}
					}
				}
				if(   !harmfulField ||
					(OTSYS_TIME() - createTime <= g_config.getNumber(ConfigManager::FIELD_OWNERSHIP_DURATION)) ||
					creature->hasBeenAttacked(owner))
				{
					conditionCopy->setParam(CONDITIONPARAM_OWNER, owner);
				}
			}

			creature->addCondition(conditionCopy);
		}
	}
}
