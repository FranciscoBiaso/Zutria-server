local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_PHYSICALDAMAGE)
setCombatParam(combat, COMBAT_PARAM_EFFECT, 17)
setCombatParam(combat, COMBAT_PARAM_COORDINATE_SYSTEM, CARTESIAN_COORDINATE_SYSTEM)
setCombatParam(combat, COMBAT_PARAM_MAGIC_INTERVALS, 30)
setCombatParam(combat, COMBAT_PARAM_DISTANCEEFFECT, 1)
setCombatFormula(combat, COMBAT_FORMULA_LEVELMAGIC, -0.16, 0, -0.33, 0)

local area = createCombatArea({
	{0, 3, 3, 3, 0,},
	{3, 2, 2, 2, 3,},
	{3, 2, 1, 2, 3,},
	{3, 2, 2, 2, 3,},
	{0, 3, 3, 3, 0,},
})
setCombatArea(combat, area)

function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end

-- function onCastSpell(cid, var)

	-- local pos = variantToPosition(var)
  -- doTeleportThing(cid, pos)
	
	-- return true
-- end












