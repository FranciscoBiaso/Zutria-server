//////////////////////////////////////////////////////////////////////
// OpenTibia - an opensource roleplaying game
//////////////////////////////////////////////////////////////////////
// Implementation of tibia v7.x protocol
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

#include "protocolgame.h"
#include "networkmessage.h"
#include "outputmessage.h"
#include "items.h"
#include "tile.h"
#include "player.h"
#include "chat.h"
#include "configmanager.h"
#include "actions.h"
#include "game.h"
#include "ioplayer.h"
#include "house.h"
#include "waitlist.h"
#include "ban.h"
#include "ioaccount.h"
#include "connection.h"
#include "creatureevent.h"

#include <string>
#include <iostream>
#include <sstream>
#include <ctime>
#include <list>

#include <boost/function.hpp>

extern Game g_game;
extern ConfigManager g_config;
extern Actions actions;

#ifdef __PROTOCOL_77__
extern RSA* g_otservRSA;
#endif // __PROTOCOL_77__

extern BanManager g_bans;
extern CreatureEvents* g_creatureEvents;
Chat g_chat;

#ifdef __ENABLE_SERVER_DIAGNOSTIC__
uint32_t ProtocolGame::ProtocolGameCount = 0;
#endif

#ifdef __SERVER_PROTECTION__
#error "You should not define __SERVER_PROTECTION__"
#define ADD_TASK_INTERVAL 40
#define CHECK_TASK_INTERVAL 5000
#else
#define ADD_TASK_INTERVAL -1
#endif
	
// Helping templates to add dispatcher tasks
template<class T1, class f1, class r>
void ProtocolGame::addGameTask(r (Game::*f)(f1), T1 p1)
{
	if(m_now > m_nextTask || m_messageCount < 5){
		Dispatcher::getDispatcher().addTask(
			createTask(boost::bind(f, &g_game, p1)));

		m_nextTask = m_now + ADD_TASK_INTERVAL;
	}
	else{
		m_rejectCount++;
		//std::cout << "reject task" << std::endl;
	}
}

template<class T1, class T2, class f1, class f2, class r>
void ProtocolGame::addGameTask(r (Game::*f)(f1, f2), T1 p1, T2 p2)
{
	if(m_now > m_nextTask || m_messageCount < 5){
		Dispatcher::getDispatcher().addTask(
			createTask(boost::bind(f, &g_game, p1, p2)));

		m_nextTask = m_now + ADD_TASK_INTERVAL;
	}
	else{
		m_rejectCount++;
		//std::cout << "reject task" << std::endl;
	}
}

template<class T1, class T2, class T3,
class f1, class f2, class f3,
class r>
void ProtocolGame::addGameTask(r (Game::*f)(f1, f2, f3), T1 p1, T2 p2, T3 p3)
{
	if(m_now > m_nextTask || m_messageCount < 5){
		Dispatcher::getDispatcher().addTask(
			createTask(boost::bind(f, &g_game, p1, p2, p3)));

		m_nextTask = m_now + ADD_TASK_INTERVAL;
	}
	else{
		m_rejectCount++;
		//std::cout << "reject task" << std::endl;
	}
}

template<class T1, class T2, class T3, class T4,
class f1, class f2, class f3, class f4,
class r>
void ProtocolGame::addGameTask(r (Game::*f)(f1, f2, f3, f4), T1 p1, T2 p2, T3 p3, T4 p4)
{
	if(m_now > m_nextTask || m_messageCount < 5){
		Dispatcher::getDispatcher().addTask(
			createTask(boost::bind(f, &g_game, p1, p2, p3, p4)));

		m_nextTask = m_now + ADD_TASK_INTERVAL;
	}
	else{
		m_rejectCount++;
		//std::cout << "reject task" << std::endl;
	}
}

template<class T1, class T2, class T3, class T4, class T5,
class f1, class f2, class f3, class f4, class f5,
class r>
void ProtocolGame::addGameTask(r (Game::*f)(f1, f2, f3, f4, f5), T1 p1, T2 p2, T3 p3, T4 p4, T5 p5)
{
	if(m_now > m_nextTask || m_messageCount < 5){
		Dispatcher::getDispatcher().addTask(
			createTask(boost::bind(f, &g_game, p1, p2, p3, p4, p5)));

		m_nextTask = m_now + ADD_TASK_INTERVAL;
	}
	else{
		m_rejectCount++;
		//std::cout << "reject task" << std::endl;
	}
}

template<class T1, class T2, class T3, class T4, class T5, class T6,
class f1, class f2, class f3, class f4, class f5, class f6,
class r>
void ProtocolGame::addGameTask(r (Game::*f)(f1, f2, f3, f4, f5, f6), T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6)
{
	if(m_now > m_nextTask || m_messageCount < 5){
		Dispatcher::getDispatcher().addTask(
			createTask(boost::bind(f, &g_game, p1, p2, p3, p4, p5, p6)));

		m_nextTask = m_now + ADD_TASK_INTERVAL;
	}
	else{
		m_rejectCount++;
		//std::cout << "reject task" << std::endl;
	}
}

template<class T1, class T2, class T3, class T4, class T5, class T6, class T7,
class f1, class f2, class f3, class f4, class f5, class f6, class f7,
class r>
void ProtocolGame::addGameTask(r (Game::*f)(f1, f2, f3, f4, f5, f6, f7), T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7)
{
	if(m_now > m_nextTask || m_messageCount < 5){
		Dispatcher::getDispatcher().addTask(
			createTask(boost::bind(f, &g_game, p1, p2, p3, p4, p5, p6, p7)));

		m_nextTask = m_now + ADD_TASK_INTERVAL;
	}
	else{
		m_rejectCount++;
		//std::cout << "reject task" << std::endl;
	}
}

template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8,
class f1, class f2, class f3, class f4, class f5, class f6, class f7, class f8,
class r>
void ProtocolGame::addGameTask(r (Game::*f)(f1, f2, f3, f4, f5, f6, f7, f8), T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8)
{
	if(m_now > m_nextTask || m_messageCount < 5){
		Dispatcher::getDispatcher().addTask(
			createTask(boost::bind(f, &g_game, p1, p2, p3, p4, p5, p6, p7, p8)));

		m_nextTask = m_now + ADD_TASK_INTERVAL;
	}
	else{
		m_rejectCount++;
		//std::cout << "reject task" << std::endl;
	}
}

ProtocolGame::ProtocolGame(Connection* connection) :
	Protocol(connection)
{
	player = NULL;
	m_nextTask = 0;
	m_nextPing = 0;
	m_lastTaskCheck = 0;
	m_messageCount = 0;
	m_rejectCount = 0;
	m_debugAssertSent = false;
	m_acceptPackets = false;
	eventConnect = 0;

}

ProtocolGame::~ProtocolGame()
{
	player = NULL;

#ifdef __ENABLE_SERVER_DIAGNOSTIC__
	ProtocolGameCount--;
#endif
}

void ProtocolGame::setPlayer(Player* p)
{
	player = p;
}

void ProtocolGame::releaseProtocol()
{
	//dispatcher thread
	if(player){
		if(player->client == this){
			player->client = NULL;
		}
	}

	Protocol::releaseProtocol();
}

void ProtocolGame::deleteProtocolTask()
{
	//dispatcher thread
	if(player){
		#ifdef __DEBUG_NET_DETAIL__
		std::cout << "Deleting ProtocolGame - Protocol:" << this << ", Player: " << player << std::endl;
		#endif

		g_game.FreeThing(player);
		player = NULL;
	}

	Protocol::deleteProtocolTask();
}

bool ProtocolGame::login(const std::string& name)
{
	//dispatcher thread
	Player* _player = g_game.getPlayerByName(name);
	if(!_player || g_config.getBoolean(ConfigManager::ALLOW_CLONES))
	{
		player = new Player(name, this);
		player->useThing2();
		player->setID();

		if(!IOPlayer::instance()->loadPlayer(player, name, true))
		{
			disconnectClient(0x14, "Your character could not be loaded.");
			return false;
		}

		if(g_bans.isBanished(player->getAccount()) && !player->hasFlag(PlayerFlag_CannotBeBanned))
		{
			disconnectClient(0x14, "Your account is banished!");
			return false;
		}

		if(g_bans.isNameLocked(player->getName()))
		{
			disconnectClient(0x14, "Your character has been name locked.");
			return false;
		}		
		
		if(g_game.getGameState() == GAME_STATE_CLOSING && !player->hasFlag(PlayerFlag_CanAlwaysLogin))
		{
            disconnectClient(0x14, "The game is just going down.\nPlease try again later.");
            return false;
        }

		if(g_game.getGameState() == GAME_STATE_CLOSED && !player->hasFlag(PlayerFlag_CanAlwaysLogin))
		{
			disconnectClient(0x14, "Server is closed.");
			return false;
		}

		if(g_config.getBoolean(ConfigManager::CHECK_ACCOUNTS) && !player->hasFlag(PlayerFlag_CanAlwaysLogin) &&
		   g_game.getPlayerByAccount(player->getAccount()))
		{
            disconnectClient(0x14, "You may only login with one character per account.");
            return false;
        }
        
        if(!WaitingList::getInstance()->clientLogin(player))
		{
			int32_t currentSlot = WaitingList::getInstance()->getClientSlot(player);
			int32_t retryTime = WaitingList::getTime(currentSlot);
			std::stringstream ss;

			ss << "Too many players online.\n" << "You are at place "
				<< currentSlot << " on the waiting list.";

			OutputMessage_ptr output = OutputMessagePool::getInstance()->getOutputMessage(this, false);
			if(output){
				TRACK_MESSAGE(output);
			    output->AddByte(0x16);
			    output->AddString(ss.str());
			    output->AddByte(retryTime);
			    OutputMessagePool::getInstance()->send(output);
			}
			getConnection()->closeConnection();
			return false;
		}

		if(!IOPlayer::instance()->loadPlayer(player, name))
		{
	        disconnectClient(0x14, "Your character could not be loaded.");
            return false;
        }
        	
		if(!g_game.placeCreature(player, player->getLoginPosition()))
		{
			if(!g_game.placeCreature(player, player->getTemplePosition(), false, true))
			{
				disconnectClient(0x14, "Temple position is wrong. Contact the administrator.");
				return false;
			}
		}

 		player->lastip = player->getIP();
		player->lastLoginSaved = std::max(time(NULL), player->lastLoginSaved + 1);
		m_acceptPackets = true;
		return true;
	}
	else
	{
		if(eventConnect != 0 || g_config.getBoolean(ConfigManager::KICK_ON_LOGIN))
		{
			//A task has already been scheduled just bail out (should not be overriden)
			disconnectClient(0x14, "You are already logged in.");
			return false;
		}

		if(_player->client)
		{
			g_chat.removeUserFromAllChannels(_player);
			_player->disconnect();
			_player->isConnecting = true;
			addRef();
			eventConnect = Scheduler::getScheduler().addEvent(
				createSchedulerTask(1000, boost::bind(&ProtocolGame::connect, this, _player->getID())));
			return true;
		}

		addRef();
		return connect(_player->getID());
		
	}

	return false;
}

bool ProtocolGame::connect(uint32_t playerId)
{

	unRef();
	eventConnect = 0;
	Player* _player = g_game.getPlayerByID(playerId);
	if(!_player || _player->isRemoved() || _player->client)
	{
		disconnectClient(0x14, "You are already logged in.");
		return false;
	}

	player = _player;
	player->useThing2();
	player->isConnecting = false;
	player->client = this;
	player->client->sendAddCreature(player, false);
	player->sendIcons();
	player->lastip = player->getIP();
	player->lastLoginSaved = std::max(time(NULL), player->lastLoginSaved + 1);
	m_acceptPackets = true;
	return true;
}

bool ProtocolGame::logout(bool forced)
{
	//dispatcher thread
	if(!player)
		return false;

	if(!player->isRemoved()){
		if(!forced){
			if(player->getTile()->hasFlag(TILESTATE_NOLOGOUT)){
				player->sendCancelMessage(RET_YOUCANNOTLOGOUTHERE);
				return false;
			}

			if(player->hasCondition(CONDITION_INFIGHT)){
				player->sendCancelMessage(RET_YOUMAYNOTLOGOUTDURINGAFIGHT);
				return false;
			}

			//scripting event - onLogOut
			if(!g_creatureEvents->playerLogOut(player)){
				//Let the script handle the error message
				return false;
			}
		}
	}

	if(Connection* connection = getConnection()){
		connection->closeConnection();
	}

	return g_game.removeCreature(player);
}

bool ProtocolGame::parseFirstPacket(NetworkMessage& msg)
{
	if(g_game.getGameState() == GAME_STATE_SHUTDOWN)
	{
		getConnection()->closeConnection();
		return false;
	}		
	
	uint32_t accnumber = msg.GetU32();
	const std::string password = msg.GetString();
	const std::string name = msg.GetString();

	
	if(g_game.getGameState() == GAME_STATE_STARTUP)
	{
		disconnectClient(0x14, "Gameworld is starting up. Please wait.");
		return false;
	}
	
	if(g_bans.isDeleted(accnumber))
	{
        disconnectClient(0x14, "Your account has been deleted!");
        return false;
    }
	
	if(g_bans.isIpDisabled(getIP()))
	{
		disconnectClient(0x14, "Too many connections attempts from this IP. Try again later.");
		return false;
	}
	
	if(g_bans.isIpBanished(getIP()))
	{
		disconnectClient(0x14, "Your IP is banished!");
		return false;
	}
	
	std::string dataTable_pass;
	if(!(IOAccount::instance()->getPassword(accnumber, name, dataTable_pass)) && passwordTest(password, dataTable_pass))
	{
		g_bans.addLoginAttempt(getIP(), false);
		getConnection()->closeConnection();
		return false;
	}
	
	g_bans.addLoginAttempt(getIP(), true);
	Dispatcher::getDispatcher().addTask(
		createTask(boost::bind(&ProtocolGame::login, this, name)));

	return true;
}

void ProtocolGame::onRecvFirstMessage(NetworkMessage& msg)
{
	parseFirstPacket(msg);
}

void ProtocolGame::disconnectClient(uint8_t error, const char* message)
{
	OutputMessage_ptr output = OutputMessagePool::getInstance()->getOutputMessage(this, false);
	if(output){
		TRACK_MESSAGE(output);
		output->AddByte(error);
		output->AddString(message);
		OutputMessagePool::getInstance()->send(output);
	}
	disconnect();
}

void ProtocolGame::disconnect()
{
	if(getConnection()){
	    getConnection()->closeConnection();
	}
}

void ProtocolGame::parsePacket(NetworkMessage &msg)
{
    if(!player || !m_acceptPackets ||
		g_game.getGameState() == GAME_STATE_SHUTDOWN || msg.getMessageLength() <= 0)
		return;

	m_now = OTSYS_TIME();

	uint8_t recvbyte = msg.GetByte();
	//std::cout << (int)recvbyte << std::endl;
	//a dead player can not performs actions
	if((player->isRemoved() || player->getHealth() <= 0) && recvbyte != 0x14)
	{
		return;
	}

	bool kickPlayer = false;

	switch(recvbyte){
		case 0x14: // logout
			parseLogout(msg);
			break;

		case 0x1E: // keep alive / ping response
			parseRecievePing(msg);
			break;

		case 0x64: // move with steps
			parseAutoWalk(msg);
			break;

		case 0x65: // move north
			parseMove(msg, NORTH);
			break;

		case 0x66: // move east
			parseMove(msg, EAST);
			break;

		case 0x67: // move south
			parseMove(msg, SOUTH);
			break;

		case 0x68: // move west
			parseMove(msg, WEST);
			break;

		case 0x69: // stop-autowalk
			parseStopAutoWalk(msg);
			break;

		case 0x6A:
			parseMove(msg, NORTHEAST);
			break;

		case 0x6B:
			parseMove(msg, SOUTHEAST);
			break;

		case 0x6C:
			parseMove(msg, SOUTHWEST);
			break;

		case 0x6D:
			parseMove(msg, NORTHWEST);
			break;

		case 0x6F: // turn north
			parseTurn(msg, NORTH);
			break;

		case 0x70: // turn east
			parseTurn(msg, EAST);
			break;

		case 0x71: // turn south
			parseTurn(msg, SOUTH);
			break;

		case 0x72: // turn west
			parseTurn(msg, WEST);
			break;

		case 0x78: // throw item
			parseThrow(msg);
			break;

		case 0x7D: // Request trade
			parseRequestTrade(msg);
			break;

		case 0x7E: // Look at an item in trade
			parseLookInTrade(msg);
			break;

		case 0x7F: // Accept trade
			parseAcceptTrade(msg);
			break;

		case 0x80: // Close/cancel trade
			parseCloseTrade();
			break;

		case 0x82: // use item
			parseUseItem(msg);
			break;

		case 0x83: // use item
			parseUseItemEx(msg);
			break;

		case 0x84: // battle window
			parseBattleWindow(msg);
			break;

		case 0x85:	//rotate item
			parseRotateItem(msg);
			break;

		case 0x87: // close container
			parseCloseContainer(msg);
			break;

		case 0x88: // "up-arrow" - container
			parseUpArrowContainer(msg);
			break;

		case 0x89:
			parseTextWindow(msg);
			break;

		case 0x8A:
			parseHouseWindow(msg);
			break;

		case 0x8C: // throw item
			parseLookAt(msg);
			break;

		case 0x96:  // say something
			parseSay(msg);
			break;

		case 0x97: // request Channels
			parseGetChannels(msg);
			break;

		case 0x98: // open Channel
			parseOpenChannel(msg);
			break;

		case 0x99: // close Channel
			parseCloseChannel(msg);
			break;

		case 0x9A: // open priv
			parseOpenPriv(msg);
			break;
	
		case 0x9B: //process report
			parseProcessRuleViolation(msg);
			break;

		case 0x9C: //gm closes report
			parseCloseRuleViolation(msg);
			break;

		case 0x9D: //player cancels report
			parseCancelRuleViolation(msg);
			break;

		case 0xA0: // set attack and follow mode
			parseFightModes(msg);
			break;

		case 0xA1: // attack
			parseAttack(msg);
			break;

		case 0xA2: //follow
			parseFollow(msg);
			break;
		
		case 0xA3:
			parseInviteToParty(msg);
			break;

		case 0xA4:
			parseJoinParty(msg);
			break;
    
		case 0xA5:
			parseRevokePartyInvitation(msg);
			break;
    
		case 0xA6:
			parsePassPartyLeadership(msg);
			break;
        
		case 0xA7:
			parseLeaveParty(msg);
			break;

		case 0xAA:
			parseCreatePrivateChannel(msg);
			break;
		
		case 0xAB:
			parseChannelInvite(msg);
			break;

		case 0xAC:
			parseChannelExclude(msg);
			break;

		case 0xBE: // cancel move
			parseCancelMove(msg);
			break;


		case 0xBF: // try to add money
			parsePlayerTryAddMoney(msg);
			break;



		case 0xC9: //client request to resend the tile
			parseUpdateTile(msg);
			break;

		case 0xCA: //client request to resend the container (happens when you store more than container maxsize)
			parseUpdateContainer(msg);
			break;

		case 0xD2: // request outfit
			parseRequestOutfit(msg);
			break;

		case 0xD3: // set outfit
			parseSetOutfit(msg);
			break;

		case 0xDC:
			parseAddVip(msg);
			break;

		case 0xDD:
			parseRemoveVip(msg);
			break;

		case 0xE6:
			parseBugReport(msg);
			break;

		case 0xE7:
			parseViolationWindow(msg);
			break;
		
		case 0xE8:
			parseDebugAssert(msg);
			break;

		case 0xFA: // 250 add skill point
			parseAddSkillButtonClick(msg);
			break;

		case 0xFB: // 251 try to add Spell Level point
			parseAddSpellLevelButtonClick(msg);
			break;

		case 0xFC: // 252 spell action
			parseSpell(msg);
			break;
	
		case 0xFD: //253 // breath
			parseBreath(msg);
			break;

		case 0xFE: //254 // active target spell requested by client
			parseTargetSpell(msg);
			break;

		case 0xFF: //255 // parse NPC left click
			parseNpcLeftClick(msg);
			break;

		default:
			std::cout << "Unknown packet header: " << std::hex << (int)recvbyte
				<< std::dec << ", player " << player->getName() << std::endl;
			kickPlayer = true;
			break;
		}

		if(msg.isOverrun()){ //we've got a badass over here
			std::cout << "msg.isOverrun() == true, player " << player->getName() << std::endl;
			kickPlayer = true;
	}

	if(kickPlayer){
		player->kickPlayer();
	}
}

void ProtocolGame::GetTileDescription(const Tile* tile, NetworkMessage_ptr msg)
{
	int count = 0;
	if(tile->ground)
	{
		msg->AddItem(tile->ground);
		count++;
	}

	ItemVector::const_iterator it;
	for(it = tile->topItems.begin(); ((it != tile->topItems.end()) && (count < 10)); ++it)
	{
		msg->AddItem(*it);
		count++;
	}

	CreatureVector::const_iterator itc;
	for(itc = tile->creatures.begin(); ((itc != tile->creatures.end()) && (count < 10)); ++itc)
	{
		bool known;
		uint32_t removedKnown;
		checkCreatureAsKnown((*itc)->getID(), known, removedKnown);
		AddCreature(msg,*itc, known, removedKnown);
		count++;
	}

	for(it = tile->downItems.begin(); ((it != tile->downItems.end()) && (count < 10)); ++it)
	{
		msg->AddItem(*it);
		count++;
	}
}

void ProtocolGame::GetMapDescription(uint16_t x, uint16_t y, unsigned char z,
	uint16_t width, uint16_t height, NetworkMessage_ptr msg)
{
	int skip = -1;
	int startz, endz, zstep = 0;

	//underground
	if (z > 7) 
	{
		startz = z - 2;
		endz = std::min(MAP_MAX_LAYERS - 1, z + 2);
		zstep = 1;
	}
	//sea to sky
	else 
	{
		startz = 7;
		endz = 0;

		zstep = -1;
	}

	for(int nz = startz; nz != endz + zstep; nz += zstep)
	{
		GetFloorDescription(msg, x, y, nz, width, height, z - nz, skip);
	}

	if(skip >= 0)
	{
		msg->AddByte(skip);
		msg->AddByte(0xFF);	
	}
}

void ProtocolGame::GetFloorDescription(NetworkMessage_ptr msg, int x, int y, int z, int width, int height, int offset, int& skip)
{
	Tile* tile;

	for(int nx = 0; nx < width; nx++)
	{
		for(int ny = 0; ny < height; ny++)
		{
			tile = g_game.getTile(x + nx + offset, y + ny + offset, z);
			if(tile)
			{
				if(skip >= 0)
				{
					msg->AddByte(skip);
					msg->AddByte(0xFF);
				}
				skip = 0;

				GetTileDescription(tile, msg);
			}
			else 
			{
				skip++;
				if(skip == 0xFF)
				{
					msg->AddByte(0xFF);
					msg->AddByte(0xFF);
					skip = -1;
				}
			}
		}
	}
}

void ProtocolGame::checkCreatureAsKnown(uint32_t id, bool &known, uint32_t &removedKnown)
{
	// loop through the known player and check if the given player is in
	std::list<uint32_t>::iterator i;
	for(i = knownCreatureList.begin(); i != knownCreatureList.end(); ++i)
	{
		if((*i) == id){
			// know... make the creature even more known...
			knownCreatureList.erase(i);
			knownCreatureList.push_back(id);

			known = true;
			return;
		}
	}

	// ok, he is unknown...
	known = false;

	// ... but not in future
	knownCreatureList.push_back(id);

	// to many known creatures?
	if(knownCreatureList.size() > 150) //150 for 7.8x
	{
		// lets try to remove one from the end of the list
		for (int n = 0; n < 150; n++){
			removedKnown = knownCreatureList.front();

			Creature *c = g_game.getCreatureByID(removedKnown);
			if ((!c) || (!canSee(c)))
				break;

			// this creature we can't remove, still in sight, so back to the end
			knownCreatureList.pop_front();
			knownCreatureList.push_back(removedKnown);
		}

		// hopefully we found someone to remove :S, we got only 150 tries
		// if not... lets kick some players with debug errors :)
		knownCreatureList.pop_front();
	}
	else{
		// we can cache without problems :)
		removedKnown = 0;
	}
}

bool ProtocolGame::canSee(const Creature* c) const
{
	if(c->isRemoved())
		return false;

	return canSee(c->getPosition());
}

bool ProtocolGame::canSee(const Position& pos) const
{
	return canSee(pos.x, pos.y, pos.z);
}

bool ProtocolGame::canSee(int x, int y, int z) const
{
#ifdef __DEBUG__
	if(z < 0 || z >= MAP_MAX_LAYERS) {
		std::cout << "WARNING! ProtocolGame::canSee() Z-value is out of range!" << std::endl;
	}
#endif

	const Position& myPos = player->getPosition();

	if(myPos.z <= 7){
		//we are on ground level or above (7 -> 0)
		//view is from 7 -> 0
		if(z > 7){
			return false;
		}
	}
	else if(myPos.z >= 8){
		//we are underground (8 -> 15)
		//view is +/- 2 from the floor we stand on
		if(std::abs(myPos.z - z) > 2){
			return false;
		}
	}

	//negative offset means that the action taken place is on a lower floor than ourself
	int offsetz = myPos.z - z;

	if ((x >= myPos.x - cMaxViewLeft + offsetz) && (x <= myPos.x + cMaxViewRight + offsetz) &&
		(y >= myPos.y - cMaxViewTop + offsetz) && (y <= myPos.y + cMaxViewBottom + offsetz))
		return true;

	return false;
}

//********************** Parse methods *******************************
void ProtocolGame::parseLogout(NetworkMessage& msg)
{
    Dispatcher::getDispatcher().addTask(
	    createTask(boost::bind(&ProtocolGame::logout, this, false)));
}

void ProtocolGame::parseCreatePrivateChannel(NetworkMessage& msg)
{
	addGameTask(&Game::playerCreatePrivateChannel, player->getID());
}

void ProtocolGame::parseChannelInvite(NetworkMessage& msg)
{
	const std::string name = msg.GetString();
	
	addGameTask(&Game::playerChannelInvite, player->getID(), name);
}

void ProtocolGame::parseChannelExclude(NetworkMessage& msg)
{
	const std::string name = msg.GetString();
	
	addGameTask(&Game::playerChannelExclude, player->getID(), name);
}

void ProtocolGame::parseGetChannels(NetworkMessage& msg)
{
	addGameTask(&Game::playerRequestChannels, player->getID());
}

void ProtocolGame::parseOpenChannel(NetworkMessage& msg)
{
	uint16_t channelId = msg.GetU16();
	
	addGameTask(&Game::playerOpenChannel, player->getID(), channelId);
}

void ProtocolGame::parseCloseChannel(NetworkMessage &msg)
{
	uint16_t channelId = msg.GetU16();

	addGameTask(&Game::playerCloseChannel, player->getID(), channelId);
}

void ProtocolGame::parseOpenPriv(NetworkMessage& msg)
{
	const std::string receiver = msg.GetString();

	addGameTask(&Game::playerOpenPrivateChannel, player->getID(), receiver);
}

void ProtocolGame::parseProcessRuleViolation(NetworkMessage& msg)
{
	const std::string reporter = msg.GetString();

	addGameTask(&Game::playerProcessRuleViolation, player->getID(), reporter);
}

void ProtocolGame::parseCloseRuleViolation(NetworkMessage& msg)
{
	const std::string reporter = msg.GetString();

	addGameTask(&Game::playerCloseRuleViolation, player->getID(), reporter);
}

void ProtocolGame::parseCancelRuleViolation(NetworkMessage& msg)
{
	addGameTask(&Game::playerCancelRuleViolation, player->getID());
}

void ProtocolGame::parseCancelMove(NetworkMessage& msg)
{
	addGameTask(&Game::playerCancelAttackAndFollow, player->getID());
}

void ProtocolGame::parseDebug(NetworkMessage& msg)
{
	int dataLength = msg.getMessageLength() - 3;
	if(dataLength != 0){
		printf("data: ");
		int data = msg.GetByte();
		while(dataLength > 0){
			printf("%d ", data);
			if(--dataLength > 0)
				data = msg.GetByte();
		}
		printf("\n");
	}
}

void ProtocolGame::parseBreath(NetworkMessage& msg)
{
	uint8_t breath = msg.GetByte();
	addGameTask(&Game::playerSaveBreath, player->getID(), breath);
}


void ProtocolGame::parseTargetSpell(NetworkMessage& msg)
{
	uint16_t itemID = msg.GetSpriteId();
	Position toPos = msg.GetPosition();
	uint16_t toSpriteId = msg.GetU16();
	uint8_t toStackPos = msg.GetByte();

	addGameTask(&Game::playerUseTargetSpell, player->getID(),itemID, toPos, toStackPos, toSpriteId);
}


void ProtocolGame::parseNpcLeftClick(NetworkMessage& msg)
{
	addGameTask(&Game::npcLeftClick, player->getID(), msg.GetString());
}

void ProtocolGame::parseRecievePing(NetworkMessage& msg)
{
	if(m_now > m_nextPing){
        Dispatcher::getDispatcher().addTask(
            createTask(boost::bind(&Game::playerReceivePing, &g_game, player->getID())));
        
        m_nextPing = m_now + 2000;
    }
}

void ProtocolGame::parseAutoWalk(NetworkMessage& msg)
{
	// first we get all directions...
	std::list<Direction> path;
	size_t numdirs = msg.GetByte();
	for (size_t i = 0; i < numdirs; ++i) {
		uint8_t rawdir = msg.GetByte();
		Direction dir = SOUTH;

		switch(rawdir) {
		case 1: dir = EAST; break;
		case 2: dir = NORTHEAST; break;
		case 3: dir = NORTH; break;
		case 4: dir = NORTHWEST; break;
		case 5: dir = WEST; break;
		case 6: dir = SOUTHWEST; break;
		case 7: dir = SOUTH; break;
		case 8: dir = SOUTHEAST; break;

		default:
			continue;
	    };

		/*
		#ifdef __DEBUG__
		std::cout << "Walk by mouse: Direction: " << dir << std::endl;
		#endif
		*/

		path.push_back(dir);
	}

	addGameTask(&Game::playerAutoWalk, player->getID(), path);
}

void ProtocolGame::parseStopAutoWalk(NetworkMessage& msg)
{
	addGameTask(&Game::playerStopAutoWalk, player->getID());
}

void ProtocolGame::parseMove(NetworkMessage& msg, Direction dir)
{
	if(!player->m_attacking)
		addGameTask(&Game::playerMove, player->getID(), dir);
}

void ProtocolGame::parseTurn(NetworkMessage& msg, Direction dir)
{
	addGameTask(&Game::playerTurn, player->getID(), dir);
}

void ProtocolGame::parseRequestOutfit(NetworkMessage& msg)
{
	addGameTask(&Game::playerRequestOutfit, player->getID());
}

void ProtocolGame::parseSetOutfit(NetworkMessage& msg)
{
	Outfit_t newOutfit;
	
	newOutfit.lookType = msg.GetByte();
	newOutfit.lookHead = msg.GetU32();
	newOutfit.lookBody = msg.GetU32();
	newOutfit.lookLegs = msg.GetU32();
	newOutfit.lookFeet = msg.GetU32();
	newOutfit.addons = msg.GetByte();
    
    addGameTask(&Game::playerChangeOutfit, player->getID(), newOutfit);
}

void ProtocolGame::parseUseItem(NetworkMessage& msg)
{
	Position pos = msg.GetPosition();
	uint16_t spriteId = msg.GetSpriteId();
	uint8_t stackpos = msg.GetByte();
	uint8_t index = msg.GetByte();

/*
#ifdef __DEBUG__
	std::cout << "parseUseItem: " << "x: " << pos.x << ", y: " << (int)pos.y <<  ", z: " << (int)pos.z << ", item: " << (int)itemId << ", stack: " << (int)stackpos << ", index: " << (int)index << std::endl;
#endif
*/

	addGameTask(&Game::playerUseItem, player->getID(), pos, stackpos, index, spriteId);
}

void ProtocolGame::parseUseItemEx(NetworkMessage& msg)
{
	Position fromPos = msg.GetPosition();
	uint16_t fromSpriteId = msg.GetSpriteId();
	uint8_t fromStackPos = msg.GetByte();
	Position toPos = msg.GetPosition();
	uint16_t toSpriteId = msg.GetU16();
	uint8_t toStackPos = msg.GetByte();

	addGameTask(&Game::playerUseItemEx, player->getID(), fromPos, fromStackPos, fromSpriteId, toPos, toStackPos, toSpriteId);
}

void ProtocolGame::parseBattleWindow(NetworkMessage &msg)
{
	Position fromPos = msg.GetPosition();
	uint16_t spriteId = msg.GetSpriteId();
	uint8_t fromStackPos = msg.GetByte();
	uint32_t creatureId = msg.GetU32();

	addGameTask(&Game::playerUseBattleWindow, player->getID(), fromPos, fromStackPos, creatureId, spriteId);
}

void ProtocolGame::parseCloseContainer(NetworkMessage& msg)
{
	uint8_t cid = msg.GetByte();
	
	addGameTask(&Game::playerCloseContainer, player->getID(), cid);
}

void ProtocolGame::parseUpArrowContainer(NetworkMessage& msg)
{
	uint8_t cid = msg.GetByte();
	
	addGameTask(&Game::playerMoveUpContainer, player->getID(), cid);
}

void ProtocolGame::parseUpdateTile(NetworkMessage& msg)
{
    Position pos = msg.GetPosition();
    
    //addGameTask(&Game::playerUpdateTile, player->getID(), pos);
}

void ProtocolGame::parseUpdateContainer(NetworkMessage& msg)
{
	uint8_t cid = msg.GetByte();
	
	addGameTask(&Game::playerUpdateContainer, player->getID(), cid);
}

void ProtocolGame::parseThrow(NetworkMessage& msg)
{
	Position fromPos = msg.GetPosition();
	uint16_t spriteId = msg.GetSpriteId();
	uint8_t fromStackpos = msg.GetByte();
	Position toPos = msg.GetPosition();
	uint8_t count = msg.GetByte();

	/*
	std::cout << "parseThrow: " << "from_x: " << (int)fromPos.x << ", from_y: " << (int)fromPos.y
	<<  ", from_z: " << (int)fromPos.z << ", item: " << (int)itemId << ", fromStackpos: "
	<< (int)fromStackpos << " to_x: " << (int)toPos.x << ", to_y: " << (int)toPos.y
	<<  ", to_z: " << (int)toPos.z
	<< ", count: " << (int)count << std::endl;
	*/

	if(toPos != fromPos){
		addGameTask(&Game::playerMoveThing, player->getID(), fromPos, spriteId, fromStackpos, toPos, count);
    }
}

void ProtocolGame::parseLookAt(NetworkMessage& msg)
{
	Position pos = msg.GetPosition();
	uint16_t spriteId = msg.GetSpriteId();
	uint8_t stackpos = msg.GetByte();

/*
#ifdef __DEBUG__
	ss << "You look at x: " << x <<", y: " << y << ", z: " << z << " and see Item # " << itemId << ".";
#endif
*/

	addGameTask(&Game::playerLookAt, player->getID(), pos, spriteId, stackpos);
}

void ProtocolGame::parseSay(NetworkMessage& msg)
{
	MessageClasses type = (MessageClasses)msg.GetByte();

	std::string receiver;
	uint16_t channelId = 0;
	
	switch(type){
        case MSG_PLAYER_PRIVATE_FROM:
            receiver = msg.GetString();
            break;
        case MSG_PLAYER_PRIVATE_TO:
            channelId = msg.GetU16();
            break;
        default:
            break;
    }
	
	const std::string text = msg.GetString();
	
	addGameTask(&Game::playerSay, player->getID(), channelId, type, receiver, text);
}

void ProtocolGame::parseFightModes(NetworkMessage& msg)
{
	uint8_t rawFightMode = msg.GetByte(); //1 - offensive, 2 - balanced, 3 - defensive
	uint8_t rawChaseMode = msg.GetByte(); // 0 - stand while fightning, 1 - chase opponent
	uint8_t rawSafeMode = msg.GetByte();

    bool safeMode = (rawSafeMode == 1);
	chaseMode_t chaseMode = CHASEMODE_STANDSTILL;

	if(rawChaseMode == 0){
		chaseMode = CHASEMODE_STANDSTILL;
	}
	else if(rawChaseMode == 1){
		chaseMode = CHASEMODE_FOLLOW;
	}

	fightMode_t fightMode = FIGHTMODE_ATTACK;

	if(rawFightMode == 1){
		fightMode = FIGHTMODE_ATTACK;
	}
	else if(rawFightMode == 2){
		fightMode = FIGHTMODE_BALANCED;
	}
	else if(rawFightMode == 3){
		fightMode = FIGHTMODE_DEFENSE;
	}

	addGameTask(&Game::playerSetFightModes, player->getID(), fightMode, chaseMode, safeMode);
}

void ProtocolGame::parseAttack(NetworkMessage& msg)
{
	uint32_t creatureId = msg.GetU32();
	
	addGameTask(&Game::playerSetAttackedCreature, player->getID(), creatureId);
}

void ProtocolGame::parseFollow(NetworkMessage& msg)
{
    uint32_t creatureId = msg.GetU32(); 

	addGameTask(&Game::playerFollowCreature, player->getID(), creatureId);
}

void ProtocolGame::parseInviteToParty(NetworkMessage& msg)
{
    uint32_t creatureId = msg.GetU32();
    
    addGameTask(&Game::playerInviteToParty, player->getID(), creatureId);
}

void ProtocolGame::parseJoinParty(NetworkMessage& msg)
{
    uint32_t creatureId = msg.GetU32();
    
    addGameTask(&Game::playerJoinParty, player->getID(), creatureId);
}

void ProtocolGame::parseRevokePartyInvitation(NetworkMessage& msg)
{
    uint32_t creatureId = msg.GetU32();
    
    addGameTask(&Game::playerRevokePartyInvitation, player->getID(), creatureId);
}

void ProtocolGame::parsePassPartyLeadership(NetworkMessage& msg)
{
    uint32_t creatureId = msg.GetU32();
    
    addGameTask(&Game::playerPassPartyLeadership, player->getID(), creatureId);
}

void ProtocolGame::parseLeaveParty(NetworkMessage& msg)
{
    addGameTask(&Game::playerLeaveParty, player->getID());
}

void ProtocolGame::parseTextWindow(NetworkMessage& msg)
{
	uint32_t windowTextId = msg.GetU32();
	const std::string newText = msg.GetString();
	
	addGameTask(&Game::playerWriteItem, player->getID(), windowTextId, newText);
}

void ProtocolGame::parseHouseWindow(NetworkMessage &msg)
{
	uint8_t doorId = msg.GetByte();
	uint32_t id = msg.GetU32();
	const std::string text = msg.GetString();
	
	addGameTask(&Game::playerUpdateHouseWindow, player->getID(), doorId, id, text);
}


void ProtocolGame::parseAddSkillButtonClick(NetworkMessage& msg)
{
	uint32_t creatureId = msg.GetU32();
	uint8_t skillId = msg.GetByte();

	addGameTask(&Game::playerAddSkillPoint, player->getID(), skillId);
}

void ProtocolGame::parseAddSpellLevelButtonClick(NetworkMessage& msg)
{
	uint8_t spellId = msg.GetByte();

	addGameTask(&Game::playerAddSpellLevel, player->getID(), spellId);
}

void ProtocolGame::parseSpell(NetworkMessage& msg)
{
	uint8_t spellId = msg.GetByte();
	addGameTask(&Game::playerUseSpell, player->getID(), spellId);
}

void ProtocolGame::parsePlayerTryAddMoney(NetworkMessage& msg)
{
	Position pos = msg.GetPosition();
	uint16_t spriteId = msg.GetSpriteId();
	uint8_t stackpos = msg.GetByte();

	addGameTask(&Game::playerTryAddMoney, player->getID(), pos, stackpos, spriteId);
}

void ProtocolGame::parseRequestTrade(NetworkMessage& msg)
{
	Position pos = msg.GetPosition();
	uint16_t spriteId = msg.GetSpriteId();
	uint8_t stackpos = msg.GetByte();
	uint32_t playerId = msg.GetU32();

	addGameTask(&Game::playerRequestTrade, player->getID(), pos, stackpos, playerId, spriteId);
}

void ProtocolGame::parseAcceptTrade(NetworkMessage& msg)
{
	addGameTask(&Game::playerAcceptTrade, player->getID());
}

void ProtocolGame::parseLookInTrade(NetworkMessage& msg)
{
	bool counterOffer = (msg.GetByte() == 0x01);
	int index = msg.GetByte();

	addGameTask(&Game::playerLookInTrade, player->getID(), counterOffer, index);
}

void ProtocolGame::parseCloseTrade()
{
	addGameTask(&Game::playerCloseTrade, player->getID());
}

void ProtocolGame::parseAddVip(NetworkMessage& msg)
{
	const std::string name = msg.GetString();
	if(name.size() > 32)
		return;

	addGameTask(&Game::playerRequestAddVip, player->getID(), name);
}

void ProtocolGame::parseRemoveVip(NetworkMessage& msg)
{
	uint32_t guid = msg.GetU32();
	
	addGameTask(&Game::playerRequestRemoveVip, player->getID(), guid);
}

void ProtocolGame::parseRotateItem(NetworkMessage& msg)
{
	Position pos = msg.GetPosition();
	uint16_t spriteId = msg.GetSpriteId();
	uint8_t stackpos = msg.GetByte();

	addGameTask(&Game::playerRotateItem, player->getID(), pos, stackpos, spriteId);
}

void ProtocolGame::parseBugReport(NetworkMessage &msg)
{
	std::string report = msg.GetString();

	addGameTask(&Game::bugReport, player->getID(), report);
}

void ProtocolGame::parseViolationWindow(NetworkMessage &msg)
{
	std::string name = msg.GetString();
	uint8_t reason = msg.GetByte();
	uint8_t action = msg.GetByte();
	std::string comment = msg.GetString();
#ifdef __PROTOCOL_77__
	uint16_t statementId = msg.GetU16();
	uint16_t channelId = msg.GetU16();
#endif // __PROTOCOL_77__
	bool IPBanishment = (msg.GetByte() == 0x01);

	addGameTask(&Game::violationWindow, player->getID(), name, reason, comment, action, IPBanishment);
}

void ProtocolGame::parseDebugAssert(NetworkMessage& msg)
{
    if(!g_config.getBoolean(ConfigManager::SAVE_CLIENT_DEBUG_ASSERTIONS)){
        return;
    }
    
    //only accept 1 report each time
    if(m_debugAssertSent){
        return;
    }
    
    m_debugAssertSent = true;
    
    std::string assertLine = msg.GetString();
    std::string date = msg.GetString();
    std::string description = msg.GetString();
    std::string comment = msg.GetString();
    
    //write it in the assertions file
    FILE* f = fopen("client_assertions.txt", "a");
    char bufferDate[32], bufferIp[32];
    time_t tmp = time(NULL);
    formatIP(getIP(), bufferIp);
    formatDate(tmp, bufferDate);
    fprintf(f, "----- %s - %s (%s) -----\n", bufferDate, player->getName().c_str(), bufferIp);
    fprintf(f, "%s\n%s\n%s\n%s\n", assertLine.c_str(), date.c_str(), description.c_str(), comment.c_str());
    fclose(f);
}

//********************** Send methods  *******************************
void ProtocolGame::sendOpenPrivateChannel(const std::string& receiver)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(msg){
		TRACK_MESSAGE(msg);
        msg->AddByte(0xAD);
        msg->AddString(receiver);
    }
}

void ProtocolGame::sendCreatureOutfit(const Creature* creature, const Outfit_t& outfit)
{
	if(canSee(creature)){
		NetworkMessage_ptr msg = getOutputBuffer();
		if(msg){
			TRACK_MESSAGE(msg);
            msg->AddByte(0x8E);
            msg->AddU32(creature->getID());
			AddCreatureOutfit(msg, creature, outfit);
        }
	}
}

void ProtocolGame::sendCreatureInvisible(const Creature* creature)
{
	if(canSee(creature)){
		NetworkMessage_ptr msg = getOutputBuffer();
		if(msg){
			TRACK_MESSAGE(msg);
		    msg->AddByte(0x8E);
		    msg->AddU32(creature->getID());
		    AddCreatureInvisible(msg, creature);
        }
	}
}

void ProtocolGame::sendCreatureLight(const Creature* creature)
{
	if(canSee(creature)){
		NetworkMessage_ptr msg = getOutputBuffer();
		if(msg){
			TRACK_MESSAGE(msg);
			AddCreatureLight(msg, creature);
		}
	}
}

void ProtocolGame::sendWorldLight(const LightInfo& lightInfo)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(msg){
		TRACK_MESSAGE(msg);
		AddWorldLight(msg, lightInfo);
	}
}

void ProtocolGame::sendCreatureSkull(const Creature* creature)
{
	if(canSee(creature)){
		NetworkMessage_ptr msg = getOutputBuffer();
		if(msg){
			TRACK_MESSAGE(msg);
			msg->AddByte(0x90);
			msg->AddU32(creature->getID());
#ifdef __SKULLSYSTEM__
			msg->AddByte(player->getSkullClient(creature->getPlayer()));
#else
			msg->AddByte(SKULL_NONE);
#endif
		}
	}
}

void ProtocolGame::sendCreatureShield(const Creature* creature)
{
	if(canSee(creature)){
		NetworkMessage_ptr msg = getOutputBuffer();
		if(msg){
			TRACK_MESSAGE(msg);
			msg->AddByte(0x91);
			msg->AddU32(creature->getID());
			msg->AddByte(player->getPartyShield(creature->getPlayer()));
		}
	}
}

void ProtocolGame::sendCreatureSquare(const Creature* creature, SquareColor_t color)
{
	if(canSee(creature)){
		NetworkMessage_ptr msg = getOutputBuffer();
		if(msg){
			TRACK_MESSAGE(msg);
			msg->AddByte(0x86);
			msg->AddU32(creature->getID());
			msg->AddByte((uint8_t)color);
		}
	}
}

void ProtocolGame::sendStats()
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(msg){
		TRACK_MESSAGE(msg);
		AddPlayerStats(msg);
	}
}

void ProtocolGame::sendTextMessage(uint8_t targetGUI, MessageClasses mclass, MessageColors color, const std::string& message)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(msg){
		TRACK_MESSAGE(msg);
		AddTextMessage(msg, targetGUI, mclass, color, message);
	}
}

void ProtocolGame::sendClosePrivate(uint16_t channelId)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(msg){
		TRACK_MESSAGE(msg);
		msg->AddByte(0xB3);
		msg->AddU16(channelId);
	}
}

void ProtocolGame::sendCreatePrivateChannel(uint16_t channelId, const std::string& channelName)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(msg){
		TRACK_MESSAGE(msg);
		msg->AddByte(0xB2);
		msg->AddU16(channelId);
		msg->AddString(channelName);
	}
}

void ProtocolGame::sendChannelsDialog()
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(msg){
		TRACK_MESSAGE(msg);
		ChannelList list;
		list = g_chat.getChannelList(player);

		msg->AddByte(0xAB);
		msg->AddByte(list.size());

		while(list.size()){
			ChatChannel *channel;
			channel = list.front();
			list.pop_front();

			msg->AddU16(channel->getId());
			msg->AddString(channel->getName());
		}
	}
}

void ProtocolGame::sendChannel(uint16_t channelId, const std::string& channelName)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(msg){
		TRACK_MESSAGE(msg);
		msg->AddByte(0xAC);
		msg->AddU16(channelId);
		msg->AddString(channelName);
	}
}

void ProtocolGame::sendRuleViolationsChannel(uint16_t channelId)
{
	/*NetworkMessage_ptr msg = getOutputBuffer();
	if(msg){
		TRACK_MESSAGE(msg);
		msg->AddByte(0xAE);
		msg->AddU16(channelId);
		RuleViolationsMap::const_iterator it = g_game.getRuleViolations().begin();
		for( ; it != g_game.getRuleViolations().end(); ++it){
			RuleViolation& rvr = *it->second;
			if(rvr.isOpen && rvr.reporter){
				AddCreatureSpeak(msg, rvr.reporter, SPEAK_RVR_CHANNEL, rvr.text, channelId, rvr.time);
			}
		}
	}*/
}

void ProtocolGame::sendRemoveReport(const std::string& name)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(msg){
		TRACK_MESSAGE(msg);
		msg->AddByte(0xAF);
		msg->AddString(name);
	}
}

void ProtocolGame::sendRuleViolationCancel(const std::string& name)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(msg){
		TRACK_MESSAGE(msg);
		msg->AddByte(0xB0);
		msg->AddString(name);
	}
}

void ProtocolGame::sendUpdateBalance(uint32_t countMoney)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if (msg) {
		TRACK_MESSAGE(msg);
		msg->AddByte(0xa8);
		msg->AddU32(countMoney);
	}
}

void ProtocolGame::sendLockRuleViolation()
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(msg){
		TRACK_MESSAGE(msg);
		msg->AddByte(0xB1);
	}
}

void ProtocolGame::sendIcons(int icons)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(msg){
		TRACK_MESSAGE(msg);
		msg->AddByte(0xA2);
		msg->AddByte(icons);
	}
}

void ProtocolGame::sendContainer(uint32_t cid, const Container* container, bool hasParent)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(msg){
		TRACK_MESSAGE(msg);
		msg->AddByte(0x6E);
		msg->AddByte(cid);
		msg->AddItemId(container);
		msg->AddString(container->getName());
		msg->AddByte(container->capacity());
		msg->AddByte(hasParent ? 0x01 : 0x00);
		if(container->size() > 255){
			msg->AddByte(255);
		}
		else{
			msg->AddByte(container->size());
		}

		ItemList::const_iterator cit;
		uint32_t i = 0;
		for(cit = container->getItems(); cit != container->getEnd() && i < 255; ++cit, ++i){
			msg->AddItem(*cit);
		}
	}
}

void ProtocolGame::sendTradeItemRequest(const Player* player, const Item* item, bool ack)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(msg){
		TRACK_MESSAGE(msg);
		if(ack){
			msg->AddByte(0x7D);
		}
		else{
			msg->AddByte(0x7E);
		}

		msg->AddString(player->getName());

		if(const Container* tradeContainer = item->getContainer()){
			std::list<const Container*> listContainer;
			ItemList::const_iterator it;
			Container* tmpContainer = NULL;

			listContainer.push_back(tradeContainer);

			std::list<const Item*> listItem;
			listItem.push_back(tradeContainer);

			while(listContainer.size() > 0) {
				const Container* container = listContainer.front();
				listContainer.pop_front();

				for(it = container->getItems(); it != container->getEnd(); ++it){
					if((tmpContainer = (*it)->getContainer())){
						listContainer.push_back(tmpContainer);
					}

					listItem.push_back(*it);
				}
			}

			msg->AddByte(listItem.size());
			while(listItem.size() > 0) {
				const Item* item = listItem.front();
				listItem.pop_front();
				msg->AddItem(item);
			}
		}
		else {
			msg->AddByte(1);
			msg->AddItem(item);
		}
	}
}

void ProtocolGame::sendCloseTrade()
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(msg){
		TRACK_MESSAGE(msg);
		msg->AddByte(0x7F);
	}
}

void ProtocolGame::sendCloseContainer(uint32_t cid)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(msg){
		TRACK_MESSAGE(msg);
		msg->AddByte(0x6F);
		msg->AddByte(cid);
	}
}

void ProtocolGame::sendNpcWindow(uint16_t windowId)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if (msg){
		TRACK_MESSAGE(msg);
		msg->AddByte(0xFF);		
		msg->AddU16(windowId);
	}
}

void ProtocolGame::sendCreatureTurn(const Creature* creature, uint32_t stackpos)
{
    if(stackpos < 10){
    	if(canSee(creature)){
	    	NetworkMessage_ptr msg = getOutputBuffer();
	    	if(msg){
				TRACK_MESSAGE(msg);
	    		msg->AddByte(0x6B);
	    		msg->AddPosition(creature->getPosition());
	    		msg->AddByte(stackpos);
	    		msg->AddU16(0x63);
	    		msg->AddU32(creature->getID());
	    		msg->AddByte(creature->getDirection());
            }
		}
	}
}

void ProtocolGame::sendCreatureSay(const Creature* creature, MessageClasses type, const std::string& text)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(msg){
		TRACK_MESSAGE(msg);
		AddCreatureSpeak(msg, creature, type, text, 0);
	}
}

void ProtocolGame::sendToChannel(const Creature * creature, MessageClasses type, const std::string& text, uint16_t channelId, uint32_t time /*= 0*/)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(msg){
		TRACK_MESSAGE(msg);
		AddCreatureSpeak(msg, creature, type, text, channelId, time);
	}
}

void ProtocolGame::sendCancel(const std::string& message)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(msg){
		TRACK_MESSAGE(msg);
		AddTextMessage(msg,MSG_TARGET_BOTTOM_CENTER_MAP, MSG_INFORMATION, MSG_COLOR_ORGANE, message);
	}
}

void ProtocolGame::sendCancelTarget()
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(msg){
		TRACK_MESSAGE(msg);
		msg->AddByte(0xA3);
	}
}

void ProtocolGame::sendChangeSpeed(const Creature* creature, uint32_t speed)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(msg){
		TRACK_MESSAGE(msg);
		msg->AddByte(0x8F);
		msg->AddU32(creature->getID());
		msg->AddU16(speed);
	}
}

void ProtocolGame::sendCancelWalk()
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(msg){
		TRACK_MESSAGE(msg);
		msg->AddByte(0xB5);
		msg->AddByte(player->getDirection());
	}
}

void ProtocolGame::sendSkills()
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(msg){
		TRACK_MESSAGE(msg);
		
		AddPlayerSkills(msg);
	}
}

void ProtocolGame::sendOnPlayerAttack(uint32_t creatureId)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if (msg) 
	{
		//TRACK_MESSAGE(msg);
		msg->AddByte(SendProtocolCodes::OnPlayerAttack);
		msg->AddU32(creatureId);		
	}
}

void ProtocolGame::sendSpellLearned(unsigned char spellId, unsigned char spellLevel)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if (msg){
		TRACK_MESSAGE(msg);
		AddSpellLearned(msg, spellId, spellLevel);
	}
}

void ProtocolGame::sendPing()
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(msg){
		TRACK_MESSAGE(msg);
		msg->AddByte(0x1E);
	}
}

void ProtocolGame::sendDistanceShoot(const Position& from, const Position& to, uint8_t type)
{
	if(canSee(from) || canSee(to)){
		NetworkMessage_ptr msg = getOutputBuffer();
		if(msg){
			TRACK_MESSAGE(msg);
			AddDistanceShoot(msg, from, to, type);
		}
	}
}

void ProtocolGame::sendMagicEffect(const Position& pos, uint8_t type)
{
	if(canSee(pos)){
		NetworkMessage_ptr msg = getOutputBuffer();
		if(msg){
			TRACK_MESSAGE(msg);
			AddMagicEffect(msg, pos, type);
		}
	}
}

void ProtocolGame::sendAnimatedText(const Position& pos, uint8_t color, std::string text)
{
	if(canSee(pos)){
		NetworkMessage_ptr msg = getOutputBuffer();
		if(msg){
			TRACK_MESSAGE(msg);
			AddAnimatedText(msg, pos, color, text);
		}
	}
}

void ProtocolGame::sendAnimatedTexts(const Position& pos, uint8_t colorOne, uint8_t colorTwo, std::string textOne, std::string textTwo)
{
	if (canSee(pos)) {
		NetworkMessage_ptr msg = getOutputBuffer();
		if (msg) {
			TRACK_MESSAGE(msg);
			AddAnimatedTexts(msg, pos, colorOne, colorTwo, textOne, textTwo);
		}
	}
}


void ProtocolGame::sendCreatureHealth(const Creature* creature)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(msg){
		TRACK_MESSAGE(msg);
		AddCreatureHealth(msg, creature);
	}
}

//tile
void ProtocolGame::sendAddTileItem(const Tile* tile, const Position& pos, const Item* item)
{
	if(canSee(pos)){
		NetworkMessage_ptr msg = getOutputBuffer();
		if(msg){
			TRACK_MESSAGE(msg);
			AddTileItem(msg, pos, item);
		}
	}
}

void ProtocolGame::sendUpdateTileItem(const Tile* tile, const Position& pos, uint32_t stackpos, const Item* item)
{
	if(canSee(pos)){
		NetworkMessage_ptr msg = getOutputBuffer();
		if(msg){
			TRACK_MESSAGE(msg);
			UpdateTileItem(msg, pos, stackpos, item);
		}
	}
}

void ProtocolGame::sendRemoveTileItem(const Tile* tile, const Position& pos, uint32_t stackpos)
{
	if(canSee(pos)){
		NetworkMessage_ptr msg = getOutputBuffer();
		if(msg){
			TRACK_MESSAGE(msg);
			RemoveTileItem(msg, pos, stackpos);
		}
	}
}

void ProtocolGame::sendUpdateTile(const Tile* tile, const Position& pos)
{
	if(canSee(pos)){
		NetworkMessage_ptr msg = getOutputBuffer();
		if(msg){
			TRACK_MESSAGE(msg);
			msg->AddByte(0x69);
			msg->AddPosition(pos);

			if(tile){
                GetTileDescription(tile, msg);
                msg->AddByte(0);
                msg->AddByte(0xFF);
            }
            else{
                msg->AddByte(0x01);
                msg->AddByte(0xFF);
            }
		}
	}
}

void ProtocolGame::sendAddCreature(const Creature* creature, bool isLogin)
{
	if(canSee(creature->getPosition()))
	{
		NetworkMessage_ptr msg = getOutputBuffer();
		if(msg)
		{
			TRACK_MESSAGE(msg);
			if(creature == player)
			{
				msg->AddByte(SEND_PROTOCOL::protocol_login);				
				//player id
				msg->AddU32(player->getID());				
				//server beat
				msg->AddByte(0x32);
				msg->AddByte(0x00);
				//ser bug report
				msg->AddByte(false);
				
				//map description
				AddMapDescription(msg, player->getPosition());

				if(isLogin)
				{
					AddMagicEffect(msg, player->getPosition(), NM_ME_ENERGY_AREA);
				}

				AddInventoryItem(msg, SLOT_HEAD, player->getInventoryItem(SLOT_HEAD));
				AddInventoryItem(msg, SLOT_NECKLACE, player->getInventoryItem(SLOT_NECKLACE));
				AddInventoryItem(msg, SLOT_BACKPACK, player->getInventoryItem(SLOT_BACKPACK));
				AddInventoryItem(msg, SLOT_ARMOR, player->getInventoryItem(SLOT_ARMOR));
				AddInventoryItem(msg, SLOT_RIGHT, player->getInventoryItem(SLOT_RIGHT));
				AddInventoryItem(msg, SLOT_LEFT, player->getInventoryItem(SLOT_LEFT));
				AddInventoryItem(msg, SLOT_LEGS, player->getInventoryItem(SLOT_LEGS));
				AddInventoryItem(msg, SLOT_FEET, player->getInventoryItem(SLOT_FEET));
				AddInventoryItem(msg, SLOT_RING, player->getInventoryItem(SLOT_RING));
		    	//AddInventoryItem(msg, SLOT_AMMO, player->getInventoryItem(SLOT_AMMO));
				
				player->updateInventoryWeigth();
				AddPlayerSkills(msg);
				AddPlayerFirstStats(msg);
				AddPlayerTreeSpells(msg);

				//gameworld light-settings
				LightInfo lightInfo;
				g_game.getWorldLightInfo(lightInfo);
				AddWorldLight(msg, lightInfo);

				//player light level
				AddCreatureLight(msg, creature);

				AddPlayerBreath(msg, player->getBreath());

				if(isLogin)
				{
					std::string tempstring = g_config.getString(ConfigManager::LOGIN_MSG);
					if(tempstring.size() > 0)
					{
						AddTextMessage(msg, MSG_TARGET_BOTTOM_CENTER_MAP, MSG_INFORMATION, MSG_COLOR_WHITE, tempstring.c_str());
					}

					if(player->getLastLoginSaved() != 0)
					{
						tempstring = "Sua �ltima visita foi em ";
						time_t lastLogin = player->getLastLoginSaved();
						tempstring += ctime(&lastLogin);
						tempstring.erase(tempstring.length() -1);
						tempstring += ".";
						AddTextMessage(msg, MSG_TARGET_BOTTOM_CENTER_MAP, MSG_INFORMATION, MSG_COLOR_WHITE, tempstring.c_str());
					}
					else
					{
						tempstring = "Bem vindo ao servidor ";
						tempstring += g_config.getString(ConfigManager::SERVER_NAME);
						tempstring += ".";
						//tempstring += ". Please choose an outfit.";
						//sendOutfitWindow(player);
					}
				}

				for(VIPListSet::iterator it = player->VIPList.begin(); it != player->VIPList.end(); it++)
				{
					bool online;
					std::string vip_name;
					if(IOPlayer::instance()->getNameByGuid((*it), vip_name))
					{
						online = (g_game.getPlayerByName(vip_name) != NULL);
						sendVIP((*it), vip_name, online);
					}
				}
			}
			else
			{
				AddTileCreature(msg, creature->getPosition(), creature);

				if (isLogin)
				{
					AddMagicEffect(msg, creature->getPosition(), NM_ME_ENERGY_AREA);
				}
			}
		}
	}

}

void ProtocolGame::sendRemoveCreature(const Creature* creature, const Position& pos, uint32_t stackpos, bool isLogout)
{
	if(canSee(pos)){
		NetworkMessage_ptr msg = getOutputBuffer();
		if(msg){
			TRACK_MESSAGE(msg);
			RemoveTileItem(msg, pos, stackpos);

			if(isLogout){
				AddMagicEffect(msg, pos, NM_ME_PUFF);
			}
		}
	}
}

void ProtocolGame::sendMoveCreature(const Creature* creature, const Tile* newTile, const Position& newPos,
		const Tile* oldTile, const Position& oldPos, uint32_t oldStackPos, bool teleport)
{
	if(creature == player)
	{
		NetworkMessage_ptr msg = getOutputBuffer();
		if(msg)
		{
			TRACK_MESSAGE(msg);
			if(teleport || oldStackPos >= 10)
			{
				RemoveTileItem(msg, oldPos, oldStackPos);
				AddMapDescription(msg, newPos);
			}
			else
			{
				//player got down
				if(oldPos.z == 7 && newPos.z >= 8)
					RemoveTileItem(msg, oldPos, oldStackPos);
				else
				{
					msg->AddByte(0x6D);
					msg->AddPosition(oldPos);
					msg->AddByte(oldStackPos);
					msg->AddPosition(newPos);
				}

				//floor change down
				if (newPos.z > oldPos.z) {
					MoveDownCreature(msg, creature, newPos, oldPos, oldStackPos);
				}
				//floor change up
				else if (newPos.z < oldPos.z) {
					MoveUpCreature(msg, creature, newPos, oldPos, oldStackPos);
				}

				int dif = 0;
				// north, for old x
				if (oldPos.y > newPos.y) 
				{
					dif = std::abs(oldPos.y - newPos.y);				
					if (dif == 1)
					{
						msg->AddByte(0x65);
						GetMapDescription(oldPos.x - cMaxViewLeft, newPos.y - cMaxViewTop, newPos.z, cMaxViewLeft + cMaxViewRight + 1, 1, msg);
					}
					else
					{
						for (int i = dif - 1; i >= 0; i--)
						{
							msg->AddByte(0x65);
							GetMapDescription(oldPos.x - cMaxViewLeft, newPos.y - cMaxViewTop + i, newPos.z, cMaxViewLeft + cMaxViewRight + 1, 1, msg);
						}
					}
				}
				// south, for old x
				else if (oldPos.y < newPos.y) 
				{ 
					dif = std::abs(oldPos.y - newPos.y);
					
					if (dif == 1)
					{
						msg->AddByte(0x67);
						GetMapDescription(oldPos.x - cMaxViewLeft, newPos.y + cMaxViewBottom, newPos.z, cMaxViewLeft + cMaxViewRight + 1, 1, msg);
					}
					else
					{
						for (int i = dif - 1; i >= 0; i--)
						{
							msg->AddByte(0x67);
							GetMapDescription(oldPos.x - cMaxViewLeft, newPos.y + cMaxViewBottom - i, newPos.z, cMaxViewLeft + cMaxViewRight + 1, 1, msg);
						}
					}
				}

				if (oldPos.x < newPos.x) { // east, [with new y]
					dif = std::abs(oldPos.x - newPos.x);

					if (dif == 1)
					{
						msg->AddByte(0x66);
						GetMapDescription(newPos.x + cMaxViewRight, newPos.y - cMaxViewTop, newPos.z, 1, cMaxViewTop + cMaxViewBottom + 1, msg);
					}
					else
					{
						for (int i = dif - 1; i >= 0; i--)
						{
							msg->AddByte(0x66);
							GetMapDescription(newPos.x + cMaxViewRight - i, newPos.y - cMaxViewTop, newPos.z, 1, cMaxViewTop + cMaxViewBottom + 1, msg);
						}
					}
					
				}
				else if (oldPos.x > newPos.x) { // west, [with new y]
					dif = std::abs(oldPos.x - newPos.x);
					if (dif == 1)
					{
						msg->AddByte(0x68);
						GetMapDescription(newPos.x - cMaxViewLeft, newPos.y - cMaxViewTop, newPos.z, 1, cMaxViewTop + cMaxViewBottom + 1, msg);
					}
					else
					{
						for (int i = dif - 1; i >= 0; i--)
						{
							msg->AddByte(0x68);
							GetMapDescription(newPos.x - cMaxViewLeft + i, newPos.y - cMaxViewTop, newPos.z, 1, cMaxViewTop + cMaxViewBottom + 1, msg);
						}
					}
					
				}
			}
		}
	}
	else if(canSee(oldPos) && canSee(creature->getPosition()))
	{
		if(teleport || (oldPos.z == 7 && newPos.z >= 8) || oldStackPos >= 10)
		{
			sendRemoveCreature(creature, oldPos, oldStackPos, false);
			sendAddCreature(creature, false);
		}
		else
		{
			NetworkMessage_ptr msg = getOutputBuffer();
			if(msg){
				msg->AddByte(0x6D);
				msg->AddPosition(oldPos);
				msg->AddByte(oldStackPos);
				msg->AddPosition(newPos);
			}
		}
	}
	else if(canSee(oldPos))
		sendRemoveCreature(creature, oldPos, oldStackPos, false);
	else if(canSee(creature->getPosition()))
		sendAddCreature(creature, false);
}

//inventory
void ProtocolGame::sendAddInventoryItem(slots_t slot, const Item* item)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(msg){
		TRACK_MESSAGE(msg);
		AddInventoryItem(msg, slot, item);
	}
}

void ProtocolGame::sendUpdateInventoryItem(slots_t slot, const Item* item)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(msg){
		TRACK_MESSAGE(msg);
		UpdateInventoryItem(msg, slot, item);
	}
}

void ProtocolGame::sendRemoveInventoryItem(slots_t slot)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(msg){
		TRACK_MESSAGE(msg);
		RemoveInventoryItem(msg, slot);
	}
}

//containers
void ProtocolGame::sendAddContainerItem(uint8_t cid, const Item* item)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(msg){
		TRACK_MESSAGE(msg);
		AddContainerItem(msg, cid, item);
	}
}

void ProtocolGame::sendUpdateContainerItem(uint8_t cid, uint8_t slot, const Item* item)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(msg){
		TRACK_MESSAGE(msg);
		UpdateContainerItem(msg, cid, slot, item);
	}
}

void ProtocolGame::sendRemoveContainerItem(uint8_t cid, uint8_t slot)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(msg){
		TRACK_MESSAGE(msg);
		RemoveContainerItem(msg, cid, slot);
	}
}
void ProtocolGame::sendTextWindow(uint32_t windowTextId, Item* item, uint16_t maxlen, bool canWrite)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(msg){
		TRACK_MESSAGE(msg);
		msg->AddByte(0x96);
		msg->AddU32(windowTextId);
		msg->AddItemId(item);
		if(canWrite){
			msg->AddU16(maxlen);
			msg->AddString(item->getText());
		}
		else{
			msg->AddU16(item->getText().size());
			msg->AddString(item->getText());
		}

#ifdef __PROTOCOL_76__
		const std::string& writer = item->getWriter();
		if(writer.size()){
			msg->AddString(writer);
		}
		else{
			msg->AddString("");
		}
#endif // __PROTOCOL_76__
	}
}

void ProtocolGame::sendTextWindow(uint32_t windowTextId, uint32_t itemId, const std::string& text)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(msg){
		TRACK_MESSAGE(msg);
		msg->AddByte(0x96);
		msg->AddU32(windowTextId);
		msg->AddItemId(itemId);

		msg->AddU16(text.size());
		msg->AddString(text);

#ifdef __PROTOCOL_76__
		msg->AddString("");
#endif // __PROTOCOL_76__
	}
}

void ProtocolGame::sendHouseWindow(uint32_t windowTextId, House* _house,
	uint32_t listId, const std::string& text)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(msg){
		TRACK_MESSAGE(msg);
		msg->AddByte(0x97);
		msg->AddByte(0);
		msg->AddU32(windowTextId);
		msg->AddString(text);
	}
}

void ProtocolGame::sendOutfitWindow(const Player* player)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(msg)
	{
		//protocol that send basic information to open outfit window (client)
	    msg->AddByte(0xC8);

		//current outfit with current color
		AddCreatureOutfit(msg, player, player->getDefaultOutfit());

		//list with number of outfits that local player has with correspondent addons
		//pair(outfit,addons)
		
		//additional information to client can read this pack of bytes
		msg->AddByte(player->outfits.size());
		for (int i = 0; i < player->outfits.size(); i++)
		{		
			//adding outfit
			msg->AddByte(player->outfits[i].first);
			//adding addons
			msg->AddByte(player->outfits[i].second);						
		}
    }
}

void ProtocolGame::sendVIPLogIn(uint32_t guid)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(msg){
		TRACK_MESSAGE(msg);
		msg->AddByte(0xD3);
		msg->AddU32(guid);
	}
}

void ProtocolGame::sendVIPLogOut(uint32_t guid)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(msg){
		TRACK_MESSAGE(msg);
		msg->AddByte(0xD4);
		msg->AddU32(guid);
	}
}

void ProtocolGame::sendVIP(uint32_t guid, const std::string& name, bool isOnline)
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(msg){
		TRACK_MESSAGE(msg);
		msg->AddByte(0xD2);
		msg->AddU32(guid);
		msg->AddString(name);
		msg->AddByte(isOnline == true ? 1 : 0);
	}
}

void ProtocolGame::sendReLoginWindow()
{
	NetworkMessage_ptr msg = getOutputBuffer();
	if(msg){
		TRACK_MESSAGE(msg);
		msg->AddByte(0x28);
	}
}

////////////// Add common messages
void ProtocolGame::AddMapDescription(NetworkMessage_ptr msg, const Position& pos)
{
	msg->AddByte(SEND_PROTOCOL::protocol_map_description);
	msg->AddPosition(player->getPosition());	  

	GetMapDescription(pos.x - cMaxViewLeft, pos.y - cMaxViewTop, pos.z, cMaxViewLeft + cMaxViewRight + 1, cMaxViewBottom + cMaxViewTop + 1, msg);
	//GetMapDescription(pos.x - cMaxViewW, pos.y - cMaxViewH, pos.z,  cMaxViewW * 2, cMaxViewH * 2, msg);
}

void ProtocolGame::AddTextMessage(NetworkMessage_ptr msg, uint8_t guiTarget, MessageClasses mclass, MessageColors color, const std::string& message)
{
	msg->AddByte(0xB4);
	msg->AddByte(guiTarget);
	msg->AddByte(mclass);
	msg->AddByte(color);
	msg->AddString(message);
}

void ProtocolGame::AddAnimatedText(NetworkMessage_ptr msg, const Position& pos,
	uint8_t color, const std::string& text)
{
	msg->AddByte(0x84);
	msg->AddPosition(pos);
	msg->AddByte(color);
	msg->AddString(text);
}

void ProtocolGame::AddAnimatedTexts(NetworkMessage_ptr msg, const Position& pos,
	uint8_t colorOne, uint8_t colorTwo, const std::string& textOne, const std::string& textTwo)
{
	msg->AddByte(152);
	msg->AddPosition(pos);
	msg->AddByte(colorOne);
	msg->AddByte(colorTwo);
	msg->AddString(textOne);
	msg->AddString(textTwo);
}

void ProtocolGame::AddMagicEffect(NetworkMessage_ptr msg,const Position& pos, uint8_t type)
{
	msg->AddByte(0x83);
	msg->AddPosition(pos);
#ifdef __PROTOCOL_76__
	msg->AddByte(type + 1);
#else
	msg->AddByte(type);
#endif // __PROTOCOL_76__
}


void ProtocolGame::AddDistanceShoot(NetworkMessage_ptr msg, const Position& from, const Position& to,
	uint8_t type)
{
	msg->AddByte(0x85);
	msg->AddPosition(from);
	msg->AddPosition(to);
#ifdef __PROTOCOL_76__
	msg->AddByte(type + 1);
#else
	msg->AddByte(type);
#endif // __PROTOCOL_76__
}

void ProtocolGame::AddCreature(NetworkMessage_ptr msg, const Creature* creature, bool known, uint32_t remove)
{
	if(known)
	{
		msg->AddU16(0x62);
		msg->AddU32(creature->getID());
	}
	else{
		msg->AddU16(0x61);
		msg->AddU32(remove);
		msg->AddU32(creature->getID());
		msg->AddString(creature->getName());
	}

	msg->AddByte((int32_t)std::ceil(((float)creature->getHealth()) * 100 / std::max(creature->getMaxHealth(), (int32_t)1)));
	msg->AddByte((uint8_t)creature->getDirection());

	if(!creature->isInvisible())
	{
		AddCreatureOutfit(msg, creature, creature->getCurrentOutfit());
	}
	else
	{
		AddCreatureInvisible(msg, creature);
	}

	LightInfo lightInfo;
	creature->getCreatureLight(lightInfo);
	msg->AddByte(lightInfo.level);
	msg->AddByte(lightInfo.color);

	msg->AddU16(creature->getStepSpeed());
#ifdef __SKULLSYSTEM__
	msg->AddByte(player->getSkullClient(creature->getPlayer()));
#else
	msg->AddByte(SKULL_NONE);
#endif
	msg->AddByte(player->getPartyShield(creature->getPlayer()));
}

void ProtocolGame::AddPlayerStats(NetworkMessage_ptr msg)
{
	// connection protocol 0xA0
	msg->AddByte(0xA0);
	// all player status
	msg->AddU16(player->getHealth());
	msg->AddU16(player->getMana());
	msg->AddU32(player->getFreeCapacity());
	msg->AddU32(player->getExperience());
	msg->AddU16(player->getPlayerInfo(PLAYERINFO_LEVEL));
	msg->AddByte(player->getPlayerInfo(PLAYERINFO_LEVELPERCENT));
	msg->AddU16(player->getBaseSpeed());
}

void ProtocolGame::AddPlayerFirstStats(NetworkMessage_ptr msg)
{
	// connection protocol 0xFB
	msg->AddByte(0xFB);
	// all player status
	msg->AddU16(player->getHealth());
	msg->AddU16(player->getMana());
	msg->AddU32(player->getFreeCapacity()/100);
	msg->AddU32(player->getExperience());
	msg->AddU16(player->getPlayerInfo(PLAYERINFO_LEVEL));
	msg->AddByte(player->getPlayerInfo(PLAYERINFO_LEVELPERCENT));
	msg->AddU16(player->getBaseSpeed());
	msg->AddU16(player->getMaxHealth());
	msg->AddU16(player->getMaxMana());
}

void ProtocolGame::AddPlayerTreeSpells(NetworkMessage_ptr msg)
{
	// connection protocol 0xFC = 252
	msg->AddByte(0xFC);
	//sending player spells
	std::list<std::pair<unsigned char, unsigned char>> spells = player->getSpells();
	//number of spells
	msg->AddByte(spells.size());
	for (auto i = spells.begin(); i != spells.end(); i++)
	{
		//spell id
		msg->AddByte((*i).first);
		//spell level
		msg->AddByte((*i).second);
	}
}

void ProtocolGame::AddPlayerSkills(NetworkMessage_ptr msg)
{
	// conection protocol 0xA1
	msg->AddByte(0xA1);
	
	//activation skills
	msg->AddU16(player->getSkillValue(ATTR_VITALITY));
	msg->AddU16(player->getSkillValue(ATTR_FORCE));
	msg->AddU16(player->getSkillValue(ATTR_AGILITY));
	msg->AddU16(player->getSkillValue(ATTR_INTELLIGENCE));
	msg->AddU16(player->getSkillValue(ATTR_CONCENTRATION));
	msg->AddU16(player->getSkillValue(ATTR_STAMINA));

	//passive skills
	msg->AddU16(player->getSkillValue(ATTR_DISTANCE));
	msg->AddU16(player->getSkillValue(ATTR_MELEE));
	msg->AddU16(player->getSkillValue(ATTR_MENTALITY));
	msg->AddU16(player->getSkillValue(ATTR_TRAINER));
	msg->AddU16(player->getSkillValue(ATTR_DEFENSE));


	msg->AddU16(player->getLevelPoints());
	msg->AddByte(player->getUnusedMagicPoints());
}

void ProtocolGame::AddSpellLearned(NetworkMessage_ptr msg, unsigned char spellId, unsigned char spellLevel)
{
	// conection protocol 0xFD -- decimal = 253
	msg->AddByte(0xFD);
	msg->AddByte(spellId);
	msg->AddByte(spellLevel);
}

void ProtocolGame::AddCreatureSpeak(NetworkMessage_ptr msg, const Creature* creature,
	MessageClasses type, std::string text, uint16_t channelId, uint32_t time /*= 0*/)
{
	msg->AddByte(0xAA);
	//Do not add name for anonymous channel talk
	msg->AddString(creature->getName());
    
	msg->AddByte(type);
	switch(type){
		case MSG_PLAYER_TALK:
		case MSG_PLAYER_WHISPER:
		case MSG_PLAYER_YELL:
		case MSG_MONSTER_TALK:
		case MSG_MONSTER_YELL:
			msg->AddPosition(creature->getPosition());
			break;
		case MSG_PLAYER_PRIVATE_TO:		
			msg->AddU16(channelId);
			break;
		/*case SPEAK_RVR_CHANNEL: 
		{
            uint32_t t = (OTSYS_TIME() / 1000) & 0xFFFFFFFF;
            msg->AddU32(t - time);
			break;
        } */

		default: break;
	}
	msg->AddString(text);
}

void ProtocolGame::AddCreatureHealth(NetworkMessage_ptr msg, const Creature* creature)
{
	msg->AddByte(0x8C);
	msg->AddU32(creature->getID());
	msg->AddByte((int32_t)std::ceil(((float)creature->getHealth()) * 100 / std::max(creature->getMaxHealth(), (int32_t)1)));
}

void ProtocolGame::AddCreatureInvisible(NetworkMessage_ptr msg, const Creature* creature)
{
	if(player->canSeeInvisibility()) {
		AddCreatureOutfit(msg, creature, creature->getCurrentOutfit());
	}
	else {
#ifdef __PROTOCOL_77__
	    msg->AddU16(0);
#else
		msg->AddByte(0);
#endif

	    msg->AddU16(0);
	}
}

void ProtocolGame::AddCreatureOutfit(NetworkMessage_ptr msg, const Creature* creature, const Outfit_t& outfit)
{
	msg->AddByte(outfit.lookType);
	
	if(outfit.lookType != 0)
	{
		msg->AddU32(outfit.lookHead);
		msg->AddU32(outfit.lookBody);
		msg->AddU32(outfit.lookLegs);
		msg->AddU32(outfit.lookFeet);
		msg->AddByte(outfit.addons);
		msg->AddU16(creature->getAttackOutfit());
		msg->AddU16(creature->getBreathOutfit());
		msg->AddU16(creature->getWalkAttackOutfit());
	}
	else{
		msg->AddItemId(outfit.lookTypeEx);
	}	
}

void ProtocolGame::AddWorldLight(NetworkMessage_ptr msg, const LightInfo& lightInfo)
{
	msg->AddByte(0x82);
	msg->AddByte(lightInfo.level);
	msg->AddByte(lightInfo.color);
}

void ProtocolGame::AddCreatureLight(NetworkMessage_ptr msg, const Creature* creature)
{
	LightInfo lightInfo;
	creature->getCreatureLight(lightInfo);
	msg->AddByte(0x8D);
	msg->AddU32(creature->getID());
	msg->AddByte(lightInfo.level);
	msg->AddByte(lightInfo.color);
}

void ProtocolGame::AddPlayerBreath(NetworkMessage_ptr msg, uint8_t breath)
{
	msg->AddByte(0xFE); //breath protocol
	msg->AddByte(breath);
}


//tile
void ProtocolGame::AddTileItem(NetworkMessage_ptr msg, const Position& pos, const Item* item)
{
	msg->AddByte(0x6A);
	msg->AddPosition(pos);
	msg->AddItem(item);
}

void ProtocolGame::AddTileCreature(NetworkMessage_ptr msg, const Position& pos, const Creature* creature)
{
	msg->AddByte(0x6A);
	msg->AddPosition(pos);

	bool known;
	uint32_t removedKnown;
	checkCreatureAsKnown(creature->getID(), known, removedKnown);
	AddCreature(msg, creature, known, removedKnown);
}

void ProtocolGame::UpdateTileItem(NetworkMessage_ptr msg, const Position& pos, uint32_t stackpos, const Item* item)
{
	if(stackpos < 10){
		msg->AddByte(0x6B);
		msg->AddPosition(pos);
		msg->AddByte(stackpos);
		msg->AddItem(item);
	}
}

void ProtocolGame::RemoveTileItem(NetworkMessage_ptr msg, const Position& pos, uint32_t stackpos)
{
	if(stackpos < 10){
		msg->AddByte(0x6C);
		msg->AddPosition(pos);
		msg->AddByte(stackpos);
	}
}

void ProtocolGame::MoveUpCreature(NetworkMessage_ptr msg, const Creature* creature,
	const Position& newPos, const Position& oldPos, uint32_t oldStackPos)
{
	if (creature == player)
	{
		//floor change up
		msg->AddByte(0xBE);

		//going to surface
		if (newPos.z == 7) 
		{
			int skip = -1;
			GetFloorDescription(msg, oldPos.x - 8, oldPos.y - 6, 5, 18, 14, 3, skip); //(floor 7 and 6 already set)
			GetFloorDescription(msg, oldPos.x - 8, oldPos.y - 6, 4, 18, 14, 4, skip);
			GetFloorDescription(msg, oldPos.x - 8, oldPos.y - 6, 3, 18, 14, 5, skip);
			GetFloorDescription(msg, oldPos.x - 8, oldPos.y - 6, 2, 18, 14, 6, skip);
			GetFloorDescription(msg, oldPos.x - 8, oldPos.y - 6, 1, 18, 14, 7, skip);
			GetFloorDescription(msg, oldPos.x - 8, oldPos.y - 6, 0, 18, 14, 8, skip);

			if (skip >= 0) {
				msg->AddByte(skip);
				msg->AddByte(0xFF);
			}
		}
		//underground, going one floor up (still underground)
		else if (newPos.z > 7) {
			int skip = -1;
			GetFloorDescription(msg, oldPos.x - 8, oldPos.y - 6, oldPos.z - 3, 18, 14, 3, skip);

			if (skip >= 0) {
				msg->AddByte(skip);
				msg->AddByte(0xFF);
			}
		}

	
		//moving up a floor up makes us out of sync
		//west
		msg->AddByte(0x68);
		GetMapDescription(oldPos.x - cMaxViewLeft, oldPos.y - cMaxViewTop + 1, newPos.z, 1, cMaxViewBottom + cMaxViewTop + 1, msg);

		//north
		msg->AddByte(0x65);
		GetMapDescription(oldPos.x - cMaxViewLeft, oldPos.y - cMaxViewTop, newPos.z, cMaxViewLeft + cMaxViewRight + 1, 1, msg);
	}
}

void ProtocolGame::MoveDownCreature(NetworkMessage_ptr msg, const Creature* creature,
	const Position& newPos, const Position& oldPos, uint32_t oldStackPos)
{
	if (creature == player) {
		//floor change down
		msg->AddByte(0xBF);

		//going from surface to underground
		if (newPos.z == 8) {
			int skip = -1;

			GetFloorDescription(msg, oldPos.x - cMaxViewLeft, oldPos.y - cMaxViewTop, newPos.z + 0, cMaxViewLeft + cMaxViewRight + 1,
								cMaxViewBottom + cMaxViewTop + 1, -1, skip);
			GetFloorDescription(msg, oldPos.x - cMaxViewLeft, oldPos.y - cMaxViewTop, newPos.z + 1, cMaxViewLeft + cMaxViewRight + 1,
								cMaxViewBottom + cMaxViewTop + 1, -2, skip);
			GetFloorDescription(msg, oldPos.x - cMaxViewLeft, oldPos.y - cMaxViewTop, newPos.z + 2, cMaxViewLeft + cMaxViewRight + 1,
								cMaxViewBottom + cMaxViewTop + 1, -3, skip);

			if (skip >= 0) {
				msg->AddByte(skip);
				msg->AddByte(0xFF);
			}
		}
		//going further down
		else if (newPos.z > oldPos.z && newPos.z > 8 && newPos.z < 14) {
			int skip = -1;
			GetFloorDescription(msg, oldPos.x - 8, oldPos.y - 6, newPos.z + 2, 18, 14, -3, skip);

			if (skip >= 0) {
				msg->AddByte(skip);
				msg->AddByte(0xFF);
			}
		}

		//moving down a floor makes us out of synceast
		//east
		msg->AddByte(0x66);
		GetMapDescription(oldPos.x + cMaxViewRight, oldPos.y - cMaxViewTop - 1, newPos.z, 1, cMaxViewBottom + cMaxViewTop + 1, msg);

		//south
		msg->AddByte(0x67);
		GetMapDescription(oldPos.x - cMaxViewLeft, oldPos.y + cMaxViewBottom, newPos.z, cMaxViewLeft + cMaxViewRight + 1, 1, msg);
	}
}


//inventory
void ProtocolGame::AddInventoryItem(NetworkMessage_ptr msg, slots_t slot, const Item* item)
{
	if(item == NULL){
		msg->AddByte(0x79);
		msg->AddByte(slot);
	}
	else{
		msg->AddByte(0x78);
		msg->AddByte(slot);
		msg->AddItem(item);
	}
}

void ProtocolGame::UpdateInventoryItem(NetworkMessage_ptr msg, slots_t slot, const Item* item)
{
	if(item == NULL){
		msg->AddByte(0x79);
		msg->AddByte(slot);
	}
	else{
		msg->AddByte(0x78);
		msg->AddByte(slot);
		msg->AddItem(item);
	}
}

void ProtocolGame::RemoveInventoryItem(NetworkMessage_ptr msg, slots_t slot)
{
	msg->AddByte(0x79);
	msg->AddByte(slot);
}

//containers
void ProtocolGame::AddContainerItem(NetworkMessage_ptr msg, uint8_t cid, const Item* item)
{
	msg->AddByte(0x70);
	msg->AddByte(cid);
	msg->AddItem(item);
}

void ProtocolGame::UpdateContainerItem(NetworkMessage_ptr msg, uint8_t cid, uint8_t slot, const Item* item)
{
	msg->AddByte(0x71);
	msg->AddByte(cid);
	msg->AddByte(slot);
	msg->AddItem(item);
}

void ProtocolGame::RemoveContainerItem(NetworkMessage_ptr msg, uint8_t cid, uint8_t slot)
{
	msg->AddByte(0x72);
	msg->AddByte(cid);
	msg->AddByte(slot);
}
