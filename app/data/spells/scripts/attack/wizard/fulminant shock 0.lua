local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_PHYSICALDAMAGE)
setCombatParam(combat, COMBAT_PARAM_EFFECT, 22)
setCombatParam(combat, COMBAT_PARAM_DISTANCEEFFECT, CONST_ANI_FIREBALL)
setCombatFormula(combat, COMBAT_FORMULA_LEVELMAGIC, -0.25, -0, -0.55, 0)

function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end
