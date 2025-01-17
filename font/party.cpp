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

#include "party.h"
#include "player.h"

#include <sstream>

Party::Party(Player* _leader)
{
	if(_leader){
		setLeader(_leader);
		_leader->setParty(this);
		_leader->sendPlayerPartyIcons(_leader);
	}
}

Party::~Party()
{
}

void Party::disband()
{
	getLeader()->sendTextMessage(MSG_TARGET_BOTTOM_CENTER_MAP | MSG_TARGET_CONSOLE,MSG_INFORMATION,
		MSG_COLOR_ORGANE,"O time foi desfeito!");
	getLeader()->setParty(NULL);
	getLeader()->sendPlayerPartyIcons(getLeader());
	setLeader(NULL);

	for(uint32_t i = 0; i < inviteList.size(); ++i){
		inviteList[i]->removePartyInvitation(this);
	}
	inviteList.clear();
	
	for(uint32_t i = 0; i < memberList.size(); ++i){
		memberList[i]->setParty(NULL);
	}
	memberList.clear();

	delete this;
}

bool Party::invitePlayer(Player* player)
{
	if(!player || player->isRemoved()){
		return false;
	}

	std::stringstream ss;

	if(std::find(inviteList.begin(), inviteList.end(), player) != inviteList.end()){
		//already on the invitation list
		return false;
	}

	ss << player->getName() << " foi convidado.";
	getLeader()->sendTextMessage(MSG_TARGET_BOTTOM_CENTER_MAP | MSG_TARGET_CONSOLE,MSG_INFORMATION,
		MSG_COLOR_YELLOW, ss.str());

	inviteList.push_back(player);
	getLeader()->sendPlayerPartyIcons(player);
	player->sendPlayerPartyIcons(getLeader());
	player->addPartyInvitation(this);

	ss.str("");
	ss << getLeader()->getName() << " convidou voc� para entrar no time.";
	player->sendTextMessage(MSG_TARGET_BOTTOM_CENTER_MAP | MSG_TARGET_CONSOLE, MSG_INFORMATION,
		MSG_COLOR_YELLOW,
		ss.str());
	return true;
}

bool Party::joinParty(Player* player)
{
	if(!player || player->isRemoved()){
		return false;
	}

	std::stringstream ss;
	ss << player->getName() << " entrou no time.";
	broadcastPartyMessage(MSG_INFORMATION, ss.str());

	memberList.push_back(player);
	player->setParty(this);

	PlayerVector::iterator it = std::find(inviteList.begin(), inviteList.end(), player);
	if(it != inviteList.end()){
		inviteList.erase(it);
	}
	player->removePartyInvitation(this);

	updatePartyIcons(player, SHIELD_BLUE);

	ss.str("");
	ss << "Voc� entrou no time do jogador " << getLeader()->getName() << ".";
	player->sendTextMessage(MSG_TARGET_BOTTOM_CENTER_MAP, MSG_INFORMATION, MSG_COLOR_GREEN, ss.str());
	return true;
}

bool Party::removeInvite(Player* player)
{
	if(!player || player->isRemoved() || !isPlayerInvited(player)){
		return false;
	}

	PlayerVector::iterator it = std::find(inviteList.begin(), inviteList.end(), player);
	if(it != inviteList.end()){
		inviteList.erase(it);
	}
	player->removePartyInvitation(this);

	getLeader()->sendCreatureShield(player);
	player->sendCreatureShield(getLeader());

	if(disbandParty()){
		disband();
	}

	return true;
}

bool Party::revokeInvitation(Player* player)
{
	std::stringstream ss;
	ss << getLeader()->getName() << " n�o aceitou o convite.";
	player->sendTextMessage(MSG_TARGET_BOTTOM_CENTER_MAP, MSG_INFORMATION, MSG_COLOR_ORGANE, ss.str());

	ss.str("");
	ss << "O convite para o jogador " << player->getName() << " foi desfeito.";
	getLeader()->sendTextMessage(MSG_TARGET_BOTTOM_CENTER_MAP, MSG_INFORMATION, MSG_COLOR_ORGANE, ss.str());
	removeInvite(player);

	return true;
}

bool Party::passPartyLeadership(Player* player)
{
	if(!player || getLeader() == player || !isPlayerMember(player)){
		return false;
	}

	 //Remove it before to broadcast the message correctly
	PlayerVector::iterator it = std::find(memberList.begin(), memberList.end(), player);
	if(it != memberList.end()){
		memberList.erase(it);
	}

	std::stringstream ss;
	ss << player->getName() << " � o novo l�der do time.";
	broadcastPartyMessage(MSG_INFORMATION, ss.str(), true);

	Player* oldLeader = getLeader();
	setLeader(player);
	
	memberList.insert(memberList.begin(), oldLeader);
	updateInvitationIcons(oldLeader, SHIELD_NONE);
	updateInvitationIcons(getLeader(), SHIELD_WHITEYELLOW);
	updatePartyIcons(oldLeader, SHIELD_BLUE);

	updatePartyIcons(player, SHIELD_YELLOW);
	player->sendTextMessage(MSG_TARGET_CONSOLE | MSG_TARGET_TOP_CENTER_MAP, MSG_INFORMATION,
		MSG_COLOR_GREEN,"You are now the leader of the party.");
	return true;
}

bool Party::leaveParty(Player* player)
{
	if(!player){
		return false;
	}

	if(!isPlayerMember(player) && getLeader() != player){
		return false;
	}

	bool hasNoLeader = false;
	if(getLeader() == player){
		if(!memberList.empty()){
			passPartyLeadership(memberList.front());
		}
		else{
			hasNoLeader = true;
		}
	}

	//Since we already passed the leadership, we remove the player from the list
	PlayerVector::iterator it = std::find(memberList.begin(), memberList.end(), player);
	if(it != memberList.end()){
		memberList.erase(it);
	}

	it = std::find(inviteList.begin(), inviteList.end(), player);
	if(it != inviteList.end()){
		inviteList.erase(it);
	}

	player->sendTextMessage(MSG_TARGET_BOTTOM_CENTER_MAP, MSG_INFORMATION,
		MSG_COLOR_ORGANE, "Voc� deixou o time.");
	player->setParty(NULL);
	updatePartyIcons(player, SHIELD_NONE);
	updateInvitationIcons(player, SHIELD_NONE);

	std::stringstream ss;
	ss << player->getName() << " deixou o time.";
	broadcastPartyMessage(MSG_INFORMATION, ss.str());

	if(hasNoLeader || disbandParty()){
		disband();
	}

	return true;
}

bool Party::isPlayerMember(const Player* player) const
{
	PlayerVector::const_iterator it = std::find(memberList.begin(), memberList.end(), player);
	if(it != memberList.end()){
		return true;
	}

	return false;
}

bool Party::isPlayerInvited(const Player* player) const
{
	PlayerVector::const_iterator it = std::find(inviteList.begin(), inviteList.end(), player);
	if(it != inviteList.end()){
		return true;
	}

	return false;
}

void Party::updatePartyIcons(Player* player, PartyShields_t shield)
{
	if(!memberList.empty()){
		for(PlayerVector::iterator it = memberList.begin(); it != memberList.end(); ++it){
			(*it)->sendPlayerPartyIcons(player);
			if(shield != SHIELD_NONE){
				player->sendPlayerPartyIcons((*it)); //Members are blue
			}
			else{
				player->sendPlayerPartyIcons((*it));
			}
		}
	}

	getLeader()->sendPlayerPartyIcons(player);

	if(shield != SHIELD_NONE){
		player->sendPlayerPartyIcons(getLeader());
	}
	else{
		player->sendPlayerPartyIcons(getLeader());
	}

	player->sendPlayerPartyIcons(player);
}

void Party::updateInvitationIcons(Player* player, PartyShields_t shield)
{
	if(!inviteList.empty()){
		for(PlayerVector::iterator it = inviteList.begin(); it != inviteList.end(); ++it){
			(*it)->sendPlayerPartyIcons(player);
			if(shield != SHIELD_NONE){
				player->sendPlayerPartyIcons((*it));
			}
			else{
				player->sendPlayerPartyIcons((*it));
			}
		}
	}
}

void Party::broadcastPartyMessage(MessageClasses msgClass, const std::string& msg, bool sendToInvitations /*= false*/)
{
	PlayerVector::iterator it;
	if(!memberList.empty()){
		for(it = memberList.begin(); it != memberList.end(); ++it){
			(*it)->sendTextMessage(MSG_TARGET_CONSOLE | MSG_TARGET_TOP_CENTER_MAP,
				msgClass,
				MSG_COLOR_ORGANE,
				msg);
		}
	}

	getLeader()->sendTextMessage(MSG_TARGET_CONSOLE | MSG_TARGET_TOP_CENTER_MAP,
		msgClass,
		MSG_COLOR_ORGANE,
		msg);

	if(sendToInvitations && !inviteList.empty()){
		for(it = inviteList.begin(); it != inviteList.end(); ++it){
			(*it)->sendTextMessage(MSG_TARGET_CONSOLE | MSG_TARGET_TOP_CENTER_MAP,
				msgClass,
				MSG_COLOR_ORGANE,
				msg);
		}
	}
}
