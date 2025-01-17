function isSorcerer(cid)
	if (isPlayer(cid)) then
		local voc = getPlayerVocation(cid)
		if (voc == 1 or voc == 5) then
			return true
		end
	end
	
	return false
end

function isDruid(cid)
	if (isPlayer(cid)) then
		local voc = getPlayerVocation(cid)
		if (voc == 2 or voc == 6) then
			return true
		end
	end
	
	return false
end

function isPaladin(cid)
	if (isPlayer(cid)) then
		local voc = getPlayerVocation(cid)
		if (voc == 3 or voc == 7) then
			return true
		end
	end
	
	return false
end

function isKnight(cid)
	if (isPlayer(cid)) then
		local voc = getPlayerVocation(cid)
		if (voc == 4 or voc == 8) then
			return true
		end
	end
	
	return false
end

function isPromoted(cid)
	if (isPlayer(cid)) then
		local voc = getPlayerVocation(cid)
		if (voc >= 5 and voc <= 8) then
			return true
		end
	end
	
	return false
end

function getDirectionTo(pos1, pos2)
	local dir = NORTH
	if(pos1.x > pos2.x) then
		dir = WEST
		if(pos1.y > pos2.y) then
			dir = NORTHWEST
		elseif(pos1.y < pos2.y) then
			dir = SOUTHWEST
		end
	elseif(pos1.x < pos2.x) then
		dir = EAST
		if(pos1.y > pos2.y) then
			dir = NORTHEAST
		elseif(pos1.y < pos2.y) then
			dir = SOUTHEAST
		end
	else
		if(pos1.y > pos2.y) then
			dir = NORTH
		elseif(pos1.y < pos2.y) then
			dir = SOUTH
		end
	end
	return dir
end

function getDistanceBetween(pos1, pos2)
	local xDif = math.abs(pos1.x - pos2.x)
	local yDif = math.abs(pos1.y - pos2.y)

	local posDif = math.max(xDif, yDif)
	if (pos1.z ~= pos2.z) then
		posDif = (posDif + 7 + 12)
	end
	return posDif
end

function getPlayerLookPos(cid)
	return getPosByDir(getThingPos(cid), getPlayerLookDir(cid))
end

function getPosByDir(basePos, dir)
	local pos = basePos
	if(dir == NORTH) then
		pos.y = pos.y-1
	elseif(dir == SOUTH) then
		pos.y = pos.y + 1
	elseif(dir == WEST) then
		pos.x = pos.x-1
	elseif(dir == EAST) then
		pos.x = pos.x+1
	elseif(dir == NORTHWEST) then
		pos.y = pos.y-1
		pos.x = pos.x-1
	elseif(dir == NORTHEAST) then
		pos.y = pos.y-1
		pos.x = pos.x+1
	elseif(dir == SOUTHWEST) then
		pos.y = pos.y+1
		pos.x = pos.x-1
	elseif(dir == SOUTHEAST) then
		pos.y = pos.y+1
		pos.x = pos.x+1
	end
	return pos
end

-- Functions made by Jiddo
function doPlayerGiveItem(cid, itemid, count, charges)
	local hasCharges = (isItemRune(itemid) or isItemFluidContainer(itemid))
	if(hasCharges and charges == nil) then
		charges = 1
	end
	
	while count > 0 do
    	local tempcount = 1
    	
    	if(hasCharges) then
    		tempcount = charges
    	end
		
    	if(isItemStackable(itemid)) then
    		tempcount = math.min (100, count)
   		end
    	
       local ret = doPlayerAddItem(cid, itemid, tempcount)
       if(ret == false) then
        	ret = doCreateItem(itemid, tempcount, getPlayerPosition(cid))
		end
        
		if(ret ~= false) then
        	if(hasCharges) then
        		count = count-1
        	else
        		count = count-tempcount
        	end
		else
			return false
		end
	end
	return true
end

function doPlayerTakeItem(cid, itemid, count)
	if(getPlayerItemCount(cid,itemid) >= count) then
		
		while count > 0 do
			local tempcount = 0
    		if(isItemStackable(itemid)) then
    			tempcount = math.min (100, count)
    		else
    			tempcount = 1
    		end
        	local ret = doPlayerRemoveItem(cid, itemid, tempcount)
        	
            if(ret ~= false) then
            	count = count-tempcount
            else
            	return false
            end
		end
		
		if(count == 0) then
			return true
		end
	end
	return false
end

function doPlayerBuyItem(cid, itemid, count, cost, charges)
	if(doPlayerRemoveMoney(cid, cost)) then
		return doPlayerGiveItem(cid, itemid, count, charges)
	end
	
	return false
end

function doPlayerSellItem(cid, itemid, count, cost)
	if(doPlayerTakeItem(cid, itemid, count)) then
		if(doPlayerAddMoney(cid, cost) ~= true) then
			error('Could not add money to ' .. getPlayerName(cid) .. '(' .. cost .. 'gp)')
		end
		return true
	end
	return false
	
end
-- End of functions made by Jiddo

function getPlayerMoney(cid)
	return ((getPlayerItemCount(cid, ITEM_CRYSTAL_COIN) * 10000) + (getPlayerItemCount(cid, ITEM_PLATINUM_COIN) * 100) + getPlayerItemCount(cid, ITEM_GOLD_COIN))
end

function doPlayerWithdrawAllMoney(cid)
	return doPlayerWithdrawMoney(cid, getPlayerBalance(cid))
end

function doPlayerDepositAllMoney(cid)
	return doPlayerDepositMoney(cid, getPlayerMoney(cid))
end

function doPlayerTransferAllMoneyTo(cid, target)
	return doPlayerTransferMoneyTo(cid, target, getPlayerBalance(cid))
end

function playerExists(name)
	return (getPlayerGUIDByName(name) ~= 0)
end

function getConfigInfo(info)
	if (type(info) ~= 'string') then 
		return nil 
	end

	dofile('config.lua')
	return _G[info]
end

function getTibiaTime()
	local worldTime = getWorldTime()
	local hours = 0
	while (worldTime > 60) do
		hours = hours + 1
		worldTime = worldTime - 60
	end

	return tostring(hours .. ':' .. worldTime)
end

exhaustion = 
{
	check = function (cid, storage)
		local exhaust = getPlayerStorageValue(cid, storage)  
		if (os.time() >= exhaust) then
			return false
		else
			return true
		end
	end,

	get = function (cid, storage)
		local exhaust = getPlayerStorageValue(cid, storage) 
		local left = exhaust - os.time()
		if (left >= 0) then
			return left
		else
			return false
		end
	end,
	
	set = function (cid, storage, time)
		setPlayerStorageValue(cid, storage, os.time()+time)  
	end,

	make = function (cid, storage, time)
		local exhaust = exhaustion.get(cid, storage)
		if (exhaust > 0) then
			return false
		else
			exhaustion.set(cid, storage, time)
			return true
		end
	end
}


	table.find = function (table, value)
		for i,v in pairs(table) do
			if (v == value) then
				return i
			end
		end
		return nil
	end
	table.getPos = table.find

	table.isStrIn = function (txt, str)
		local result = false
		for i, v in pairs(str) do          
			result = (string.find(txt, v) and not string.find(txt, '(%w+)' .. v) and not string.find(txt, v .. '(%w+)'))
			if (result) then
				break
			end
		end
		return result
	end

	table.countElements = function (table, item)
		local count = 0
		for i, n in pairs(table) do
			if (item == n) then count = count + 1 end
		end
		return count
	end
	
	table.getCombinations = function (table, num)
		local a, number, select, newlist = {}, #table, num, {}
		for i = 1, select do
			a[#a + 1] = i
		end
		local newthing = {}
		while (1) do
			local newrow = {}
			for i = 1, select do
				newrow[#newrow + 1] = table[a[i]]
			end
			newlist[#newlist + 1] = newrow
			i = select
			while (a[i] == (number - select + i)) do
				i = i - 1
			end
			if (i < 1) then break end
				a[i] = a[i] + 1
				for j = i, select do
					a[j] = a[i] + j - i
				end
			end
		return newlist
	end


	string.split = function (str)
		local t = {}
		local function helper(word) table.insert(t, word) return "" end
		if (not str:gsub("%w+", helper):find"%S") then return t end
	end
	
	string.separate = function(separator, string)
		local a, b = {}, 0
		if (#string == 1) then return string end
	    while (true) do
			local nextSeparator = string.find(string, separator, b + 1, true)
			if (nextSeparator ~= nil) then
				table.insert(a, string.sub(string,b,nextSeparator-1)) 
				b = nextSeparator + 1 
			else
				table.insert(a, string.sub(string, b))
				break 
			end
	    end
		return a
	end

