local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_DISTANCEEFFECT, 0)
setCombatParam(combat, COMBAT_PARAM_CREATEITEM, 2875)
setCombatParam(combat, COMBAT_PARAM_AGGRESSIVE, FALSE)
setCombatParam(combat, COMBAT_PARAM_COORDINATE_SYSTEM, CARTESIAN_COORDINATE_SYSTEM)
setCombatParam(combat, COMBAT_PARAM_MAGIC_INTERVALS, 4)



local area = createCombatArea({{1}})
setCombatArea(combat, area)

function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end