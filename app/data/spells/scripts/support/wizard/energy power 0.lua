local combat = createCombatObject()

--setCombatParam(combat, COMBAT_PARAM_TYPE, 1) -- effect of damage
setCombatParam(combat, COMBAT_PARAM_EFFECT, 8) -- effect of spell
setCombatParam(combat, COMBAT_PARAM_AGGRESSIVE, FALSE)


local condition = createConditionObject(CONDITION_HASTE)
setConditionParam(condition, CONDITION_PARAM_TICKS, 4000)
setConditionFormula(condition, 0.3, 3, 0.3, 3)
setCombatCondition(combat, condition)

function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end