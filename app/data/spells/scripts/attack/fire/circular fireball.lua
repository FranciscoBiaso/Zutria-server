local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_PHYSICALDAMAGE)
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_FIREBALL)
setCombatFormula(combat, COMBAT_FORMULA_LEVELMAGIC, -0.25, -0, -0.55, 0)

local arr = {
{1, 1, 1},
{1, 3, 1},
{1, 1, 1},
{1, 1, 1}
}

local area = createCombatArea(arr)
setCombatArea(combat, area)

function onCastSpell(cid, var)
	return doCombatWithIntervals(cid, combat, var)
end