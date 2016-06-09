local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_PHYSICALDAMAGE) -- effect of damage
setCombatParam(combat, COMBAT_PARAM_EFFECT, 4) -- effect of spell
setCombatParam(combat, COMBAT_PARAM_MAGIC_INTERVALS, 1.00) -- magic with time
setCombatParam(combat, COMBAT_PARAM_COORDINATE_SYSTEM, POLAR_COORDINATE_SYSTEM) -- polar coordinate sytem
setCombatParam(combat, COMBAT_PARAM_COEFFICIENTA, 1) -- coefficient of function
setCombatParam(combat, COMBAT_PARAM_COEFFICIENTB, 0.15) -- coefficient of function
setCombatParam(combat, COMBAT_PARAM_THETA, (3) * 360) -- theta angle
setCombatParam(combat, COMBAT_PARAM_DRAW_FUNCTION, DRAW_FUNCTION_ARQUIMEDES_SPIRAL) -- draw function
setCombatFormula(combat, COMBAT_FORMULA_LEVELMAGIC, -0.01, -0, -0.01, 0) -- formula damage
setCombatLuaState(combat) -- prerequisite for use of this system

function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end