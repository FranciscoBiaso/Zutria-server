local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_PHYSICALDAMAGE) -- effect of damage
setCombatParam(combat, COMBAT_PARAM_EFFECT, 21) -- effect of spell
--setCombatParam(combat, COMBAT_PARAM_MAGIC_INTERVALS, 10.00) -- magic with time
setCombatParam(combat, COMBAT_PARAM_COORDINATE_SYSTEM, CARTESIAN_COORDINATE_SYSTEM)
--setCombatParam(combat, COMBAT_PARAM_COEFFICIENTA, 1) -- coefficient of function
--setCombatParam(combat, COMBAT_PARAM_COEFFICIENTB, 0.15) -- coefficient of function
--setCombatParam(combat, COMBAT_PARAM_THETA, (2) * 360) -- theta angle
setCombatParam(combat, COMBAT_PARAM_DRAW_FUNCTION, DRAW_FUNCTION_NONE) -- draw function (use matrix area)
setCombatFormula(combat, COMBAT_FORMULA_LEVELMAGIC, -10, -0.2, -10, 100) -- formula damage
--setCombatLuaState(combat) -- prerequisite for use of this system

local condition = createConditionObject(CONDITION_FIRE)
--setConditionParam(condition, CONDITION_PARAM_TICKS, 5000)
setConditionParam(condition, CONDITION_PARAM_DELAYED, 1)
addDamageCondition(condition, 10, 2000, -10)
-- mina minb maxa maxb
--setConditionFormula(condition, -0.2, 0.0, -0.4, 0.0)
setCombatCondition(combat, condition)

local area = createCombatArea(AREA_LINEWAVE1, AREADIAGONAL_WAVE1)
setCombatArea(combat, area)

function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end