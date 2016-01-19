function onUse(cid, item, frompos, item2, topos)
	newPos = {x = frompos.x + 2, y = frompos.y, z = frompos.z + 1, stackpos = 0}
	doTeleportThing(cid, newPos)
	return true
end