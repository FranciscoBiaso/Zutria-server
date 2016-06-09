local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_PHYSICALDAMAGE) -- effect of damage
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_FIREBALL_EXPLOSION) -- effect of spell
setCombatParam(combat, COMBAT_PARAM_MAGIC_INTERVALS, 100.00) -- magic with time
setCombatParam(combat, COMBAT_PARAM_COORDINATE_SYSTEM, CARTESIAN_COORDINATE_SYSTEM)
setCombatParam(combat, COMBAT_PARAM_DRAW_FUNCTION, DRAW_FUNCTION_NONE) -- draw function (use matrix array)
--setCombatParam(combat, COMBAT_PARAM_COEFFICIENTA, 1) -- coefficient of function
--setCombatParam(combat, COMBAT_PARAM_COEFFICIENTB, 0.15) -- coefficient of function
--setCombatParam(combat, COMBAT_PARAM_THETA, (2) * 360) -- theta angle
setCombatFormula(combat, COMBAT_FORMULA_LEVELMAGIC, -1000, -1000, -1000, -1000) -- formula damage
setCombatLuaState(combat) -- prerequisite for use of this system


local area = createCombatArea(AREA_LINEWAVE2, AREADIAGONAL_WAVE2)
setCombatArea(combat, area)

function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end