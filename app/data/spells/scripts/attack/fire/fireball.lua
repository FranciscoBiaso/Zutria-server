local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_PHYSICALDAMAGE)
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_FIREBALL)
setCombatParam(combat, COMBAT_PARAM_DISTANCEEFFECT, CONST_ANI_FIREBALL)
setCombatParam(combat, COMBAT_PARAM_MAGIC_INTERVALS, 12)
setCombatFormula(combat, COMBAT_FORMULA_LEVELMAGIC, -0.25, -0, -0.55, 0)

function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end
