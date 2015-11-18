function onStepIn(cid, item, topos, frompos)
		newPos = {x = topos.x + 1, y = topos.y, z = topos.z + 1 , stackpos = 0}
		doTeleportThing(cid, newPos)
	return true
end
