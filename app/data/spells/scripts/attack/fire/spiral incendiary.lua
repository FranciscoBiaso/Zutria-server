local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_PHYSICALDAMAGE) -- effect of damage
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_FIREBALL) -- effect of spell
setCombatParam(combat, COMBAT_PARAM_MAGIC_INTERVALS, 1.5) -- magic with time
setCombatParam(combat, COMBAT_PARAM_COORDINATE_SYSTEM, 1) -- polar coordinate sytem
setCombatParam(combat, COMBAT_PARAM_COEFFICIENTA, 0.7) -- coefficient of function
setCombatParam(combat, COMBAT_PARAM_COEFFICIENTB, 0.35) -- coefficient of function
setCombatParam(combat, COMBAT_PARAM_THETA, 2080) -- theta angle
setCombatParam(combat, COMBAT_PARAM_DRAW_FUNCTION, DRAW_FUNCTION_FERMAT_SPIRAL) -- draw function
setCombatFormula(combat, COMBAT_FORMULA_LEVELMAGIC, -0.25, -0, -0.55, 0) -- formula damage
setCombatLuaState(combat) -- prerequisite for use of this system

function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end