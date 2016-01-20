function onUse(cid, item, frompos, item2, topos)
	newPos = {x = frompos.x - 1, y = frompos.y + 1, z = frompos.z - 1, stackpos = 0}
	doTeleportThing(cid, newPos)
	return true
end