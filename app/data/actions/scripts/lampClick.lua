local ITEM_LAMP_ON = 509
local ITEM_LAMP_OFF = 510



function onUse(cid, item, frompos, item2, topos)
	if (item.itemid == ITEM_LAMP_ON) then
		doTransformItemAndKeepDuration(item.uid, ITEM_LAMP_OFF)
	else
		doTransformItemAndKeepDuration(item.uid, ITEM_LAMP_ON)
	end
	
	doDecayItem(item.uid)
	return true
end 