function onUse(cid, item, frompos, item2, topos)
  local itemPosition = getItemPos(item.uid)
  local newPos = {x = itemPosition.x, y = itemPosition.y + 2, z = itemPosition.z + 1, stackpos = 0}
	
  doTeleportThing(cid, newPos)
	
  return true
end